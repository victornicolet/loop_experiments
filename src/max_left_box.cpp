//
// Created by victorn on 07/11/18.
//

#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

using namespace std;
using namespace tbb;


struct MaxLeftBox {
    int ***A;
    long m, l;
    int lsum;
    int mls;
    int *c;

    MaxLeftBox(int ***_input, long _l, long _m) : A(_input), l(_l), m(_m), mls(INT_MIN), lsum(0) {
        c = new int[_m]{0};
    }

    MaxLeftBox(MaxLeftBox &s, split) : mls(0), A(s.A), l(s.l), m(s.m), lsum(0) {
        c = new int[s.m];
    }

    void operator()(const blocked_range<long> &r) {
        int _mls = INT_MIN;
        for (long i = r.begin(); i != r.end(); ++i) {

            int lsum = 0;
            _mls = INT_MIN;
            int psum = 0;

            for (long j = 0; j < m; j++) {
                for(long k=0; k<l; k++) {
                    psum += A[i][j][k];
                }

                c[j] += psum;
                lsum += c[j];
                _mls = max(_mls, lsum);
            }
        }
        mls = _mls;
    }

    void join(MaxLeftBox &rhs) {
        mls = 0;
        lsum = 0;
        for (long j = 0; j < m; j++) {
            c[j] += rhs.c[j];
            lsum += c[j];
            mls = max(lsum, mls);
        }
    }
};

int seq_implem(int ***A, long l, long m, long n) {
    int cols[m] = {0};
    int mls = INT_MIN;

    for(long i = 0; i < n; ++i) {
        int lsum  = 0;
        mls = INT_MIN;
        for(long j = 0; j < m; j++) {
            int psum = 0;
            for(long k = 0; k < l; k++) {
                psum += A[i][j][k];
            }
            cols[j] += psum;
            lsum += cols[j];
            mls = max(mls, lsum);
        }
    }

    return mls;
}

double do_seq(int ***A, long l, long m, long n) {
    StopWatch t;
    seq_implem(A, l, m,n);
    double elapsed = 0.0;
    for(int i = 0; i < NUM_REPEAT; i++) {
        t.start();
        seq_implem(A, l, m, n);
        elapsed += t.stop();
    }
    return elapsed / NUM_REPEAT;
}


double do_par(int ***input, long l, long m, long n, int num_cores) {
    StopWatch t;
    double elapsed = 0.0;
    // TBB Initialization with num_cores cores
    static task_scheduler_init init(task_scheduler_init::deferred);
    init.initialize(num_cores, UT_THREAD_DEFAULT_STACK_SIZE);

    MaxLeftBox maxLeftBox(input, l, m);
    parallel_reduce(blocked_range<long>(0, n-1), maxLeftBox);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), maxLeftBox);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}


int main(int argc, char** argv) {
    // Data size:
    if(argc < 4) {
        cout << "Usage:./ExpMaxLeftBox [NUM_ROWS] [NUM_COLS] [NUM_3D]" << endl;
        return  -1;
    }

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int l = atoi(argv[3]);
    // Data allocation and initialization
    int ***input;
    input = create_rand_int_3D_matrix(l, m,n);


    for(int num_threads = 0; num_threads <= EXP_MAX_CORES; num_threads++) {
        double exp_time;

        if (num_threads > 0) {
            // Do the parallel experiment.
            exp_time = do_par(input, l, m, n, num_threads);
        } else {
            // Do the sequential experiment.
            exp_time = do_seq(input, l, m, n);
        }

        cout << "max-left-box" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}