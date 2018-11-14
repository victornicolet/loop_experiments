#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"
#include "omp.h"

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

struct mbbs_result {
    int mbbs;
    int sum;
};

mbbs_result seq_implem(int ***A, long l, long m, long n) {

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

    return {mbs, ss};
}



mbbs_result mbbs_join(mbbs_result a, mbbs_result b) {
    return {
            max(a.mbbs + b.sum, b.mbbs),
            a.sum + b.sum
    };
}

mbbs_result parallel_omp(int ***A, long l, long m, long n, int num_cores) {
    StopWatch t;

#pragma omp declare reduction \
  (join:mbbs_result:omp_out=mbbs_join(omp_out,omp_in)) \
  initializer(omp_priv={0, 0})

    mbbs_result mbbsResult = {0, 0};

    omp_set_dynamic(0);
    omp_set_num_threads(num_cores);
#pragma omp parallel for reduction(join:mbbsResult)
    for (long i = 0; i < n; i++) {
        int psum = 0;
        for (long j = 0; j < m; j++) {
            for (long k = 0; k < l; k++) {
                psum += A[i][j][k];
            }
        }
        mbbsResult.mbbs = max(mbbsResult.mbbs + psum, 0);
        mbbsResult.sum += psum;
    }

    return mbbsResult;
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

    MaxBottomBox maxBottomBox(A, l, m);
    parallel_reduce(blocked_range<long>(0, n-1), maxBottomBox);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), maxBottomBox);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}

double do_par_omp(int ***input, long l, long m, long n, int num_cores) {
    StopWatch t;
    double elapsed = 0.0;

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_omp(input, l, m, n, num_cores);
        elapsed += t.stop();
    }

    return elapsed / NUM_REPEAT;
}

int main(int argc, char** argv) {
    if(argc <= 1) {
        cout << "Usage: ./OMPExpMaxBotBox [N] [L] [M]" << endl;
        return  -1;
    }

    long n,m,l;
    if (argc == 4) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        l = atoi(argv[3]);
    } else {
        // Data size:
        n = 2 << EXPERIMENTS_3D_N;
        m = 2 << EXPERIMENTS_3D_M;
        l = 2 << EXPERIMENTS_3D_L;
    }
    // Data allocation and initialization

    int ***input = create_rand_int_3D_matrix(l, m, n);

    double exp_time;
    double exp_time_omp = 0.0;

    for(int num_threads = 0; num_threads <= 16; num_threads++) {
        if (num_threads > 0) {
            // Do the parallel experiment.
            exp_time = do_par(input, l, m, n, num_threads);
            exp_time_omp = do_par_omp(input, l, m, n, num_threads);
        } else {
            // Do the sequential experiment.
            exp_time = do_seq(input, l, m, n);
        }

//        CSV LINE : Prog. name, N, M, L, Num threads used, Exp time, OpenMP exp time
        cout << argv[0] << "," << n << "," << m << "," << l << "," << num_threads
             << ", " << exp_time << "," << exp_time_omp << endl;
    }
    return 0;
}
