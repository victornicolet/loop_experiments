#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

using namespace std;
using namespace tbb;

struct mts_res { int sum; int mts; };

struct MaxTopStrip {
    int **A;
    long m;
    int tss;
    int mts;

    MaxTopStrip(int** _input, long rl) : A(_input), m(rl), tss(0), mts(0) {}

    MaxTopStrip(MaxTopStrip& s, split) {mts = 0; tss = 0; A = s.A; m = s.m; }

    void operator()( const blocked_range<long>& r ) {
      for(long i = r.begin(); i < r.end(); i++)
        {
          for(long j = 0; j < m; j++)
            {
              tss += A[i][j];
            }
          mts = max(mts, tss);
        }

    }
    
    // To do
    void join(MaxTopStrip& rhs) {
        int aux = tss;
        tss = rhs.tss+tss;
        mts = max(mts,rhs.mts+aux);
    }

};

mts_res seq_implem(int **A, long m, long n) {

    int top_strip_sum = 0;
    int max_top_strip = 0;
    int strip_sum = 0;
    for(int i = 0; i < n; i++)
    {
      for(int j = 0; j < m; j++)
        {
          top_strip_sum += A[i][j];
        }
      max_top_strip = max(max_top_strip, top_strip_sum);
    }

    return {top_strip_sum, max_top_strip};
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

    MaxTopStrip mtp(input, m);
    parallel_reduce(blocked_range<long>(0, n-1), mtp);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), mtp);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}


int main(int argc, char** argv) {
    // Data size:
    if(argc < 3) {
        cout << "Usage:./ExpMinMax [NUM_ROWS] [NUM_COLS]" << endl;
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

        cout << "max-top-strip" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}