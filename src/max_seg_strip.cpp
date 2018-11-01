#include <tbb/tbb.h>
#include <iostream>
#include <algorithm>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

using namespace std;
using namespace tbb;


struct msegs_res {
    int mtails;
    int msegs;
};


struct MaxSegStrip {
    int **A; // Input data
    long m; // Length of a row
// Insert additional parameters here
    int mbs;
    int ms;
    int ss;
    int mps;
    int sum;


    MaxSegStrip(int** _input, long rl) : A(_input), m(rl), mbs(0), ms(0), ss(0), mps(0), sum(0) {}

    MaxSegStrip(MaxSegStrip& s, split) { A = s.A; m = s.m; mbs = 0; ms = 0; ss = 0; mps = 0; sum = 0;}

    void operator()( const blocked_range<long>& r ) {
      for(int i = r.begin(); i < r.end(); i++)
        {
          ss = 0;
          for(int j = 0; j < m; j++)
            {
              ss += A[i][j];
            }
          sum = sum + ss;
          mps = max(mbs,ms);
          mbs = max(mbs + ss, 0);
          ms = max(mbs, ms);
        }

    }

    void join(MaxSegStrip& r) {
       ss = r.ss;
       int sumAux = sum;
       sum = r.sum + sum;
       mps = max(mps, sumAux + r.mps);
       int mbsAux = mbs;
       mbs = max(r.mbs, r.sum + mbs);
       ms = max(max(ms, r.ms), mbsAux + r.mbs);

    }

};


msegs_res seq_implem(int **A, long m, long n) {
    StopWatch t;
    t.start();

    // Insert sequential code here. It must be the same as the code in the operator()
  int max_bot_strip = 0;
  int max_strip = 0;
  int strip_sum = 0;
  for(int i = 0; i < n; i++)
	{
	  strip_sum = 0;
	  for(int j = 0; j < m; j++)
		{
		  strip_sum += A[i][j];
		}
	  max_bot_strip = max(max_bot_strip + strip_sum, 0);
	  max_strip = max(max_bot_strip, max_strip);
	}

    return {max_bot_strip, max_strip};
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

    MaxSegStrip msegs(input, m);
    parallel_reduce(blocked_range<long>(0, n-1), msegs);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), msegs);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}


int main(int argc, char** argv) {
    // Data size:
    if(argc < 3) {
        cout << "Usage:./ExpMaxSegStrip [NUM_ROWS] [NUM_COLS]" << endl;
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

        cout << "max-seg-strip" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}