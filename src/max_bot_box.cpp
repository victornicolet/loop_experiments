#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

using namespace std;
using namespace tbb;


struct MaxBottomBox {
    int ***A;
    long m;
    long l;
    int ss;
    int mbs;
    int sum;

    MaxBottomBox(int*** _input, long _l, long _m) : A(_input), l(_l), m(_m), ss(0), mbs(0), sum(0) {}

    MaxBottomBox(MaxBottomBox& s, split) { A = s.A; l = s.l; m = s.m; ss = 0; mbs = 0; sum = 0; }

    void operator()( const blocked_range<long>& r ) {
      for(long i = r.begin(); i < r.end(); i++)
        {
          ss = 0;
          for(long j = 0; j < m; j++)
            {
                for(long k = 0; k < l; k++) {
                    ss += A[i][j][k];
                }
            }
          /*  Auxiliary : sum += strip_sum */
          sum += ss;
          mbs = max(mbs + ss, 0);
        }

    }

    void join(MaxBottomBox& r) {
       ss = r.ss;
       sum = sum + r.sum;
       mbs = max(r.mbs, r.sum + mbs);
    }

};

struct mbbox_res {
    int botsum;
    int mbsum;
};

mbbox_res seq_implem(int ***A, long l, long m, long n) {

    StopWatch t;
    t.start();
    int ss = 0;
    int mbs = 0;

      for(long i = 0; i < n; i++)
        {
          ss = 0;
          for(long j = 0; j < m; j++)
            {
                for(long k = 0; k < l; k++) {
                    ss += A[i][j][k];
                }
            }
          /*  Auxiliary : sum += strip_sum */
          mbs = max(mbs + ss, 0);
        }

    return {ss, mbs};
}


double do_seq(int ***A, long l, long m, long n) {
    StopWatch t;
    seq_implem(A, l, m, n);
    double elapsed = 0.0;
    for(int i = 0; i < NUM_REPEAT; i++) {
        t.start();
        seq_implem(A, l, m, n);
        elapsed += t.stop();
    }
    return elapsed / NUM_REPEAT;
}


double do_par(int ***A, long l, long m, long n, int num_cores) {
    StopWatch t;
    double elapsed = 0.0;
    // TBB Initialization with num_cores cores
    static task_scheduler_init init(task_scheduler_init::deferred);
    init.initialize(num_cores, UT_THREAD_DEFAULT_STACK_SIZE);

    MaxBottomBox maxBottomBox(A, m, l);
    parallel_reduce(blocked_range<long>(0, n-1), maxBottomBox);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), maxBottomBox);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}


int main(int argc, char** argv) {
    // Data size:
    if(argc < 4) {
        cout << "Usage:./ExpMaxBottomBox [NUM_ROWS] [NUM_COLS] [DEPTH]" << endl;
        return  -1;
    }

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int l = atoi(argv[3]);
    // Data allocation and initialization
    int ***A;
    A = create_rand_int_3D_matrix(l,m,n);


    for(int num_threads = 0; num_threads <= EXP_MAX_CORES; num_threads++) {
        double exp_time;

        if (num_threads > 0) {
            // Do the parallel experiment.
            exp_time = do_par(A, l, m, n, num_threads);
        } else {
            // Do the sequential experiment.
            exp_time = do_seq(A, l, m, n);
        }

        cout << "max-bottom-box" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}
