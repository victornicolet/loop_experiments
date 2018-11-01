#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "omp.h"

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

double do_seq(int ***A, long l, long m, long n) {


    StopWatch t;
    t.start();
    int top_Box_sum = 0;
    int max_top_Box = 0;

    for(long i = 0; i < n; i++)
    {
      for(long j = 0; j < m; j++)
        {
            for(long k = 0; k < l; k++){
                top_Box_sum += A[i][j][k];
            }
        }
        max_top_Box = max(max_top_Box, top_Box_sum);
    }

    return t.stop();
}

struct mtb_result {
    int max_top_Box;
    int top_Box_sum;
};

mtb_result mtb_join(mtb_result a, mtb_result b) {
    return {
        max(a.max_top_Box, b.max_top_Box + a.top_Box_sum),
        a.top_Box_sum + b.top_Box_sum
    };
}

double parallel_omp(int ***A, long l, long m, long n, int num_cores) {
    StopWatch t;
    double elapsed = 0.0;
    t.start();

#pragma omp declare reduction \
  (join:mtb_result:omp_out=mtb_join(omp_out,omp_in)) \
  initializer(omp_priv={INT_MIN, 0})

        mtb_result mtb = {INT_MIN, 0};

#pragma omp parallel for reduction(join:mtb)
        for (long i = 0; i < n; i++) {
            for (long j = 0; j < m; j++) {
                for (long k = 0; k < l; k++) {
                    mtb.top_Box_sum += A[i][j][k];
                }
            }
            mtb.max_top_Box = max(mtb.max_top_Box, mtb.top_Box_sum);
        }
        elapsed = t.stop();

    return elapsed;
}

double do_par(int ***input, long l, long m, long n, int num_cores) {
    StopWatch t;
    double elapsed = 0.0;
    // TBB Initialization with num_cores cores
    static task_scheduler_init init(task_scheduler_init::deferred);
    init.initialize(num_cores, UT_THREAD_DEFAULT_STACK_SIZE);

    MaxTopBox mlr(input, l, m);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), mlr);
        elapsed += t.stop();
    }

    return elapsed / NUM_REPEAT;
}


double do_par_omp(int ***input, long l, long m, long n, int num_cores) {
    StopWatch t;
    double elapsed = 0.0;
    // TBB Initialization with num_cores cores
    static task_scheduler_init init(task_scheduler_init::deferred);
    init.initialize(num_cores, UT_THREAD_DEFAULT_STACK_SIZE);

    MaxTopBox mlr(input, m, l);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_omp(input, l, m, n, num_cores);
        elapsed += t.stop();
    }

    return elapsed / NUM_REPEAT;
}


int main(int argc, char** argv) {

    bool custom_sizes = false;

    if(argc <= 1) {
        cout << "Usage: xx [NUMBER OF CORES] [N] [L] [M]" << endl;
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
    // Data allocation and initialization

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


    int num_cores = atoi(argv[1]);
    double exp_time = 0.0;
    double exp_time_omp = 0.0;

    if (num_cores > 0) {
        // Do the parallel experiment.
        exp_time = do_par(input, l, m, n, num_cores);
        exp_time_omp = do_par_omp(input, l, m, n, num_cores);
    } else {
        // Do the sequential experiment.
        exp_time = do_seq(input, l, m, n);
    }

    if(custom_sizes) {
        cout << argv[0] << "," << n << "," << m << "," << l << "," << num_cores
             << ", " << exp_time <<"," << exp_time_omp << endl;
    } else {
        cout << argv[0] << "," << num_cores << "," << exp_time  <<","<< exp_time_omp<< endl;
    }
    return 0;
}
