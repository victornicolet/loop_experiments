#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"

using namespace std;
using namespace tbb;


struct BoxSum {
    int **A;
    long m, l;
    int sum;

    BoxSum(int ** _input, long rl, long rl2) : A(_input), m(rl), l(rl2), sum(0) {}

    BoxSum(BoxSum& s, split) { A = s.A; m = s.m; l = s.l; }

    void operator () (const blocked_range<long>& r) {
        int loc_sum = sum;

        for(long i = r.begin(); i < r.end (); i++) {
            for(long j = 0; j < l; j++) {
                loc_sum += A[i][j];
            }
        }

        sum = loc_sum;
    }

    void join(BoxSum& rhs) {
        sum += rhs.sum;
    }
};

struct MaxTopBox {
    int ***A;
    long m, l;
    int tss;
    int mts;

    MaxTopBox(int*** _input, long rl, long rl2) : A(_input), m(rl), l(rl2), tss(0), mts(0) {}

    MaxTopBox(MaxTopBox& s, split) {mts = 0; tss = 0; A = s.A; m = s.m; l = s.l; }

    void operator()( const blocked_range<long>& r ) {

      for(long i = r.begin(); i < r.end(); i++)
        {
            BoxSum planesum(A[i], m, l);
            parallel_reduce(blocked_range<long>(0, m), planesum);
            mts = max(mts, planesum.sum);
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

double do_par(int ***input, long l, long m, long n, int num_cores) {
    StopWatch t;
    double elapsed = 0.0;
    // TBB Initialization with num_cores cores
    static task_scheduler_init init(task_scheduler_init::deferred);
    init.initialize(num_cores, UT_THREAD_DEFAULT_STACK_SIZE);

    MaxTopBox mlr(input, m, l);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), mlr);
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
#pragma omp parallel for
    for(long i = 0; i < n; i++) {
        input[i] = (int**) malloc(sizeof(int*) * m);
#pragma omp parallel for
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
