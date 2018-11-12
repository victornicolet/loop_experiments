#include <tbb/tbb.h>
#include <iostream>
#include <algorithm>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

using namespace std;
using namespace tbb;


struct MaxSegBox {
    int ***A; // Input data
    long m, l; // Length of a row
// Insert additional parameters here
    int mbs;
    int ms;
    int mps;
    int sum;


    MaxSegBox(int*** _input, long _l, long _m) : A(_input), m(_m), l(_l),  mbs(0), ms(0), mps(0), sum(0) {}

    MaxSegBox(MaxSegBox& s, split) { A = s.A; m = s.m; l = s.l; mbs = 0; ms = 0; mps = 0; sum = 0;}

    void operator()( const blocked_range<long>& r ) {
      for(long i = r.begin(); i < r.end(); i++)
        {
          int ss = 0;
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
       int sumAux = sum;
       sum = r.sum + sum;
       mps = max(mps, sumAux + r.mps);
       int mbsAux = mbs;
       mbs = max(r.mbs, r.sum + mbs);
       ms = max(max(ms, r.ms), mbsAux + r.mbs);

    }

};

struct msbox_res { int mbb; int msb; };


msbox_res seq_implem(int ***A, long l, long m, long n) {

  int mbb = 0;
  int msb = 0;

  for(long i = 0; i < n; i++)
	{
	  int bsum = 0;
	  for(long j = 0; j < m; j++)
		{
		    for(long k = 0; k < l; k++)
		    {
                bsum += A[i][j][k];
		    }
		}
	  mbb = max(mbb + bsum, 0);
	  msb = max(mbb, msb);
	}

    return {mbb, msb};
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

    MaxSegBox maxSegBox(A, m, l);
    parallel_reduce(blocked_range<long>(0, n-1), maxSegBox);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), maxSegBox);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}


int main(int argc, char** argv) {
    // Data size:
    if(argc < 4) {
        cout << "Usage:./ExpMaxSegBox [NUM_ROWS] [NUM_COLS] [DEPTH]" << endl;
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

        cout << "max-seg-box" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}

