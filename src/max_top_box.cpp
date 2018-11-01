#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "omp.h"
#include "datagen.h"

using namespace std;
using namespace tbb;


struct MaxTopBox {
    int ***A;
    long m, l;
    int tss;
    int mts;

    MaxTopBox(int*** _input, long _l, long _m) : A(_input), m(_m), l(_l), tss(0), mts(0) {}

    MaxTopBox(MaxTopBox& s, split) {mts = 0; tss = 0; A = s.A; m = s.m; l = s.l; }

    void operator()( const blocked_range<long>& r ) {
      for(long i = r.begin(); i < r.end(); i++)
        {
          for(long j = 0; j < m; j++)
            {
                for(long k = 0; k < l; k++)
                {
                    tss += A[i][j][k];
                }
            }
          mts = max(mts, tss);
        }
    }
    
    // To do
    void join(MaxTopBox& rhs) {
        int aux = tss;
        tss = rhs.tss+tss;
        mts = max(mts,rhs.mts+aux);
    }

};

struct mtb_result {
    int tbsum;
    int mtbox;
};


mtb_result seq_implem(int ***A, long l, long m, long n) {


    StopWatch t;
    t.start();
    int tbsum = 0;
    int mtbox = 0;

    for(long i = 0; i < n; i++)
    {
      for(long j = 0; j < m; j++)
        {
            for(long k = 0; k < l; k++){
                tbsum += A[i][j][k];
            }
        }
        mtbox = max(mtbox, tbsum);
    }

    return {tbsum, mtbox};
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

    MaxTopBox mtbx(A, m, l);
    parallel_reduce(blocked_range<long>(0, n-1), mtbx);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), mtbx);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}


int main(int argc, char** argv) {
    // Data size:
    if(argc < 4) {
        cout << "Usage:./ExpMaxTopBox [NUM_ROWS] [NUM_COLS] [DEPTH]" << endl;
        return  -1;
    }

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int l = atoi(argv[3]);
    // Data allocation and initialization
    int ***A;
    A = create_rand_int_3D_matrix(l,m,n);


    for(int num_threads = 0; num_threads <= EXP_MAX_CORES; num_threads++) {
        double exp_time = 0.0;

        if (num_threads > 0) {
            // Do the parallel experiment.
            exp_time = do_par(A, l, m, n, num_threads);
        } else {
            // Do the sequential experiment.
            exp_time = do_seq(A, l, m, n);
        }

        cout << "max-top-box" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}

