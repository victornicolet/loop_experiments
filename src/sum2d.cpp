#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

using namespace std;
using namespace tbb;


struct Sum2D {
    int **A;
    int sum;
    long m;

    Sum2D(int** _input, long rl) : A(_input), m(rl), sum(0){}

    Sum2D(Sum2D& s, split) {sum = 0; A = s.A; m = s.m; }

    void operator()( const blocked_range<long>& r ) {
        int sum_loc = sum;
        for(long i = r.begin(); i != r.end(); ++i) {
            for(long j = 0; j < m-1; j++) {
                sum_loc += A[i][j];
            }
        }
        sum = sum_loc;
    }

    void join(Sum2D& rhs) { sum = sum + rhs.sum; }

};

int seq_implem(int **A, long m, long n) {
    int sum= 0;

    for(long i = 0; i < n - 1; ++i) {
        for(long j = 0; j < m-1; j++) {
            sum += A[i][j];
        }
    }

    return sum;
}

double do_seq(int **A, long m, long n) {

    seq_implem(A, m, n);

    StopWatch t;
    double elapsed = 0.0;
    for(int i = 0; i < NUM_REPEAT; i++) {
        t.start();
        seq_implem(A,m,n);
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

    Sum2D sum(input, m);
    parallel_reduce(blocked_range<long>(0, n-1), sum);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), sum);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}

int main(int argc, char** argv) {
    // Data size:
    if(argc < 3) {
        cout << "Usage:./Exp2DSum [NUM_ROWS] [NUM_COLS]" << endl;
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

        cout << "2d-sum" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}