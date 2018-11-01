#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

using namespace std;
using namespace tbb;


struct SaddlePoint {
    int **A;
    long m;
    int* colm;
    int mcols = 0;
    int xrows = 0;
    int rowx = 0;


    SaddlePoint(int** _input, long rl) :
            A(_input), m(rl), mcols(INT_MIN), xrows(INT_MIN), rowx(INT_MIN) {
        colm = new int[m] {INT_MAX};
    }

    SaddlePoint(SaddlePoint& s, split) : A(s.A), m(s.m), mcols(INT_MIN), xrows(INT_MIN), rowx(INT_MIN) {
        colm = new int[s.m] {INT_MAX};
    }

    void operator()( const blocked_range<long>& r ) {

        for(long i = r.begin(); i != r.end(); i++)
        {
            rowx = INT_MIN;
            for(long j = 0; j < m; j++)
            {
                rowx = max(rowx, A[i][j]);
                colm[j] = min(colm[j], A[i][j]);
                mcols = max(colm[j], mcols);
            }
            xrows = min(rowx, xrows);
        }
    }

    void join(SaddlePoint& r) {
        for(long j = 0; j < m; j++) {
            colm[j] = min(colm[j], r.colm[j]);
            mcols = max(colm[j], mcols);
        }
        rowx = r.rowx;
        xrows = min(xrows, r.xrows);
    }
};

int seq_implem(int **A, long m, long n) {
    int rowx = INT_MIN;
    int mcols = INT_MIN;
    int xrows = INT_MIN;
    int* colm;
    colm = new int[m] {INT_MAX};
    for(long j = 0; j < m; j ++) {
        colm[j] = INT_MAX;
    }

    for(long i = 0; i < n; ++i) {
        rowx = INT_MIN;
        for(long j = 0; j < m; j++) {
            rowx = max(rowx, A[i][j]);
            colm[j] = min(colm[j], A[i][j]);
            mcols = max(colm[j], mcols);
        }
        xrows = min(rowx, xrows);
    }

    return xrows;
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

    SaddlePoint mm(input, m);
    parallel_reduce(blocked_range<long>(0, n-1), mm);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), mm);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}


int main(int argc, char** argv) {
    // Data size:
    if(argc < 3) {
        cout << "Usage:./ExpSaddlePoint [NUM_ROWS] [NUM_COLS]" << endl;
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

        cout << "saddle-point" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}