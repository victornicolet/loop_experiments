#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"

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

double do_seq(int ***A, long l, long m, long n) {

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

    return t.stop();
}

double do_par(int*** input, long l, long m, long n, int num_cores) {
    StopWatch t;
    double elapsed = 0.0;
    // Any specific initalization of state variables must be done here.

    // TBB Initialization with num_cores cores
    static task_scheduler_init init(task_scheduler_init::deferred);
    init.initialize(num_cores, UT_THREAD_DEFAULT_STACK_SIZE);

    MaxBottomBox mlr(input , l, m);
//  Iterate one before starting to measure.
    parallel_reduce(blocked_range<long>(0, n), mlr);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n), mlr);
        elapsed += t.stop();
    }

    return elapsed / NUM_REPEAT;
}

int main(int argc, char** argv) {
    // Data size:
    long n = 2 << EXPERIMENTS_3D_N;
    long m = 2 << EXPERIMENTS_3D_M;
    long l = 2 << EXPERIMENTS_3D_L;
    // Data allocation and initialization
    int ***input;
    input = (int***) malloc(sizeof(int**) * n);
    for(long i = 0; i < n; i++) {
        input[i] = (int**) malloc(sizeof(int*) * m);
        for(long j =0; j < m; j++){
            input[i][j] = (int*) malloc(sizeof(int) * l);
            for(long k = 0; k < l; k++)
            {
                input[i][j][k] =  (rand() % 255) - 122;
            }
        }
    }

    if(argc <= 1) {
        cout << "Usage: Gradient1 [NUMBER OF CORES]" << endl;
        return  -1;
    }

    int num_cores = atoi(argv[1]);
    double exp_time = 0.0;

    if (num_cores > 0) {
        // Do the parallel experiment.
        exp_time = do_par(input, l, m, n, num_cores);
    } else {
        // Do the sequential experiment.
        exp_time = do_seq(input, l, m, n);
    }

    cout <<argv[0] << "," << num_cores << "," << exp_time << endl;

    return 0;
}
