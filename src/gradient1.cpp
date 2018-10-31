#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

using namespace std;
using namespace tbb;


struct Gradient1 {
    int **A;
    bool b;
    long m;

    Gradient1(int** _input, long rl) : A(_input), m(rl), b(true){}

    Gradient1(Gradient1& s, split) {b = true; A = s.A; m = s.m; }

    void operator()( const blocked_range<long>& r ) {
        bool bl = b;
        for(long i = r.begin(); i != r.end(); ++i) {
            for(long j = 0; j < m-1; j++) {
                bl = bl &&
                    (A[i+1][j] > A[i][j]) &&
                    (A[i][j+1] > A[i][j]);
            }
        }
        b = bl;
    }

    void join(Gradient1& rhs) { b = b && rhs.b; }

};


bool seq_implem(int **A, long m, long n) {
    bool b = true;

    for(long i = 0; i < n - 1; ++i) {
        for(long j = 0; j < m-1; j++) {
            b = b &&
                (A[i+1][j] > A[i][j]) &&
                (A[i][j+1] > A[i][j]);
        }
    }

    return b;
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

    Gradient1 gr(input, m);
    parallel_reduce(blocked_range<long>(0, n-1), gr);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), gr);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}

int main(int argc, char** argv) {
    // Data size:
    if(argc < 3) {
        cout << "Usage:./ExpGradient1 [NUM_ROWS] [NUM_COLS]" << endl;
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

        cout << "gradient1" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}