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


struct MaxLeftStrip {
    int **A;
    long m;
    int lsum;
    int mls;
    int *cols;

    MaxLeftStrip(int **_input, long rl) : A(_input), m(rl), mls(INT_MIN), lsum(0) {
        cols = new int[rl]{0};
    }

    MaxLeftStrip(MaxLeftStrip &s, split) : mls(0), A(s.A), m(s.m), lsum(0) {
        cols = new int[s.m];
    }

    void operator()(const blocked_range<long> &r) {
        int _mls = INT_MIN;
        for (long i = r.begin(); i != r.end(); ++i) {
            int lsum = 0;
            _mls = INT_MIN;
            for (long j = 0; j < m; j++) {
                cols[j] += A[i][j];
                lsum += cols[j];
                _mls = max(_mls, lsum);
            }
        }
        mls = _mls;
    }

    void join(MaxLeftStrip &rhs) {
        mls = 0;
        lsum = 0;
        for (long j = 0; j < m; j++) {
            cols[j] += rhs.cols[j];
            lsum += cols[j];
            mls = max(lsum, mls);
        }
    }
};

int seq_implem(int **A, long m, long n) {
    int cols[m] = {0};
    int lsum = 0;
    int mls = INT_MIN;

    for(long i = 0; i < n; ++i) {
        lsum  = 0;
        mls = INT_MIN;
        for(long j = 0; j < m; j++) {
            cols[j] += A[i][j];
            lsum += cols[j];
            mls = max(mls, lsum);
        }
    }

    return mls;
}

double do_seq(int **A, long m, long n) {
    StopWatch t;
    seq_implem(A,m,n);
    double elapsed = 0.0;
    for(int i = 0; i < NUM_REPEAT; i++) {
        t.start();
        seq_implem(A, m, n);
        elapsed += t.stop();
    }
    return elapsed / NUM_REPEAT;
}


double do_par(int **input, long m, long n, int num_cores) {
    StopWatch t;
    double elapsed = 0.0;
    // TBB Initialization with num_cores cores
    static task_scheduler_init init(task_scheduler_init::deferred);
    init.initialize(num_cores, UT_THREAD_DEFAULT_STACK_SIZE);

    MaxLeftStrip maxLeftStrip(input, m);
    parallel_reduce(blocked_range<long>(0, n-1), maxLeftStrip);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), maxLeftStrip);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}


int main(int argc, char** argv) {
    // Data size:
    if(argc < 3) {
        cout << "Usage:./ExpMaxLeftStrip [NUM_ROWS] [NUM_COLS]" << endl;
        return  -1;
    }

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    // Data allocation and initialization
    int **input;
    input = create_rand_int_2D_matrix(m,n);


    for(int num_threads = 0; num_threads <= EXP_MAX_CORES; num_threads++) {
        double exp_time = 0.0;

        if (num_threads > 0) {
            // Do the parallel experiment.
            exp_time = do_par(input, m, n, num_threads);
        } else {
            // Do the sequential experiment.
            exp_time = do_seq(input, m, n);
        }

        cout << "max-left-strip" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}