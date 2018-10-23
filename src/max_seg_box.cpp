#include <tbb/tbb.h>
#include <iostream>
#include <algorithm>
#include "StopWatch.h"
#include "param.h"

using namespace std;
using namespace tbb;


struct MaxSegBox {
    int ***A; // Input data
    long m, l; // Length of a row
// Insert additional parameters here
    int mbs;
    int ms;
    int ss;
    int mps;
    int sum;


    MaxSegBox(int*** _input, long _l, long _m) : A(_input), m(_m), l(_l),  mbs(0), ms(0), ss(0), mps(0), sum(0) {}

    MaxSegBox(MaxSegBox& s, split) { A = s.A; m = s.m; l = s.l; mbs = 0; ms = 0; ss = 0; mps = 0; sum = 0;}

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
          sum = sum + ss;
          mps = max(mbs,ms);
          mbs = max(mbs + ss, 0);
          ms = max(mbs, ms);
        }

    }

    void join(MaxSegBox& r) {
       ss = r.ss;
       int sumAux = sum;
       sum = r.sum + sum;
       mps = max(mps, sumAux + r.mps);
       int mbsAux = mbs;
       mbs = max(r.mbs, r.sum + mbs);
       ms = max(max(ms, r.ms), mbsAux + r.mbs);

    }

};

double do_seq(int ***A, long l, long m, long n) {
    StopWatch t;
    t.start();

    // Insert sequential code here. It must be the same as the code in the operator()
  int max_bot_Box = 0;
  int max_Box = 0;
  int Box_sum = 0;
  for(long i = 0; i < n; i++)
	{
	  Box_sum = 0;
	  for(long j = 0; j < m; j++)
		{
		    for(long k = 0; k < l; k++)
		    {
                Box_sum += A[i][j][k];
		    }
		}
	  max_bot_Box = max(max_bot_Box + Box_sum, 0);
	  max_Box = max(max_bot_Box, max_Box);
	}

    return t.stop();
}

double do_par(int ***input, long l, long m, long n, int num_cores) {
    StopWatch t;
    double elapsed = 0.0;
    // Any specific initalization of state variables must be done here.

    // TBB Initialization with num_cores cores
    static task_scheduler_init init(task_scheduler_init::deferred);
    init.initialize(num_cores, UT_THREAD_DEFAULT_STACK_SIZE);

    MaxSegBox mtss(input,  l, m);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), mtss);
        elapsed += t.stop();
    }

    return elapsed / NUM_REPEAT;
}

int main(int argc, char** argv) {
    // Data size:
    bool custom_sizes = false;

    if(argc <= 1) {
        cout << "Usage:" << argv[0] << " [NUMBER OF CORES] [N] [L] [M]" << endl;
        return  -1;
    }

    long n,m,l;
    if (argc == 5) {
        n = atoi(argv[2]);
        m = atoi(argv[3]);
        l = atoi(argv[4]);
        custom_sizes = true;
    } else {
        // Data size:
        n = 2 << EXPERIMENTS_3D_N;
        m = 2 << EXPERIMENTS_3D_M;
        l = 2 << EXPERIMENTS_3D_L;
    }

    int ***input;
    input = (int***) malloc(sizeof(int**) * n);
    for(long i = 0; i < n; i++) {
        input[i] = (int**) malloc(sizeof(int*) * m);
        for(long j =0; j < m; j++){
            input[i][j] = (int*) malloc(sizeof(int) * l);
        }
    }

#pragma omp parallel for
    for(long i = 0; i < n; i++) {
#pragma omp parallel for
        for(long j =0; j < m; j++){
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

    if(custom_sizes) {
        cout << argv[0] << "," << n << "," << m << "," << l << "," << num_cores
             << "," << exp_time << endl;
    } else {
        cout << argv[0] << "," << num_cores << "," << exp_time << endl;
    }

    return 0;
}
