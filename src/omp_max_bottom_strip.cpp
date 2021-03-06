#include <tbb/tbb.h>
#include <iostream>
#include <omp.h>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

using namespace std;
using namespace tbb;

struct mbs_res {
    int mbs;
    int sum;
};

struct MaxBottomStrip {
    int **A;
    long m;
    int ss;
    int mbs;
    int sum;

    MaxBottomStrip(int** _input, long _m) : A(_input), m(_m), ss(0), mbs(0), sum(0) {}

    MaxBottomStrip(MaxBottomStrip& s, split) { A = s.A; m = s.m; ss = 0; mbs = 0; sum = 0; }

    void operator()( const blocked_range<long>& r ) {
      for(int i = r.begin(); i < r.end(); i++)
        {
          ss = 0;
          for(int j = 0; j < m; j++)
            {
              ss += A[i][j];
            }
          /*  Auxiliary : sum += strip_sum */
          sum += ss;
          mbs = max(mbs + ss, 0);
        }

    }

    void join(MaxBottomStrip& r) {
       ss = r.ss;
       sum = sum + r.sum;
       mbs = max(r.mbs, r.sum + mbs);

    }

};

mbs_res seq_implem(int **A, long m, long n) {

    int* rects = (int*) calloc(m, sizeof(int));

    StopWatch t;
    t.start();
    int ss = 0;
    int mbs = 0;

      for(int i = 0; i < n; i++)
        {
          ss = 0;
          for(int j = 0; j < m; j++)
            {
              ss += A[i][j];
            }
          /*  Auxiliary : sum += strip_sum */
          mbs = max(mbs + ss, 0);
        }

    return {ss, mbs};
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

mbs_res mbs_join(mbs_res a, mbs_res b) {
    return {
            max(a.mbs + b.sum, b.mbs),
            a.sum + b.sum
    };
}


mbs_res parallel_omp(int **A, long m, long n, int num_cores) {
    StopWatch t;

#pragma omp declare reduction \
  (join:mbs_res:omp_out=mbs_join(omp_out,omp_in)) \
  initializer(omp_priv={0, 0})

    mbs_res mbs_res = {0, 0};

    omp_set_dynamic(0);
    omp_set_num_threads(num_cores);
#pragma omp parallel for reduction(join:mbs_res)
    for (long i = 0; i < n; i++) {
        int lsum = 0;
        for (long j = 0; j < m; j++) {
            lsum += A[i][j];
        }
        mbs_res.mbs = max(mbs_res.mbs + lsum, 0);
        mbs_res.sum += lsum;
    }

    return mbs_res;
}



double do_par(int **input, long m, long n, int num_cores) {
    StopWatch t;
    double elapsed = 0.0;
    // TBB Initialization with num_cores cores
    static task_scheduler_init init(task_scheduler_init::deferred);
    init.initialize(num_cores, UT_THREAD_DEFAULT_STACK_SIZE);

    MaxBottomStrip mm(input, m);
    parallel_reduce(blocked_range<long>(0, n-1), mm);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), mm);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}

double do_par_omp(int **input, long m, long n, int num_cores) {
    StopWatch t;

    double elapsed = 0.0;

    parallel_omp(input, m, n, num_cores);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_omp(input, m, n, num_cores);
        elapsed += t.stop();
    }

    return elapsed / NUM_REPEAT;
}

int main(int argc, char** argv) {
    if(argc <= 1) {
        cout << "Usage: ./OMPExpMaxBotBox [N] [M]" << endl;
        return  -1;
    }

    long n,m,l;
    if (argc == 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
    } else {
        // Data size:
        n = 2 << EXPERIMENTS_3D_N;
        m = 2 << EXPERIMENTS_3D_M;
    }
    // Data allocation and initialization

    int **input = create_rand_int_2D_matrix(m, n);

    double exp_time;
    double exp_time_omp = 0.0;

    for(int num_threads = 0; num_threads <= 16; num_threads++) {
        if (num_threads > 0) {
            // Do the parallel experiment.
            exp_time = do_par(input,  m, n, num_threads);
            exp_time_omp = do_par_omp(input, m, n, num_threads);
        } else {
            // Do the sequential experiment.
            exp_time = do_seq(input, m, n);
        }

//        CSV LINE : Prog. name, N, M, L, Num threads used, Exp time, OpenMP exp time
        cout << argv[0] << "," << n << "," << m << "," << num_threads
             << ", " << exp_time << "," << exp_time_omp << endl;
    }
    return 0;
}
