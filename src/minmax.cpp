#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

using namespace std;
using namespace tbb;

struct minmax_res {
    int amin;
    int amax;
};

struct MinMax {
    int **A;
    int amin;
    int amax;
    long m;

    MinMax(int** _input, long rl) : A(_input), m(rl), amin(INT_MAX), amax(INT_MIN){}

    MinMax(MinMax& s, split) {
        amax = INT_MIN;
        amin = INT_MAX;
        A = s.A;
        m = s.m;
    }

    void operator()( const blocked_range<long>& r ) {
        int _amax = amax;
        int _amin = amin;
        for(long i = r.begin(); i != r.end(); ++i) {
            _amin = INT_MAX;
            for(long j = 0; j < m-1; j++) {
                _amin = min(_amin, A[i][j]);
            }
            _amax = max(_amax, _amin);
        }
        amax = _amax;
        amin = _amin;
    }

    void join(MinMax& rhs) {
        amax = max(amax, rhs.amax);
        amin = rhs.amin;
    }
};


minmax_res seq_implem(int **A, long m, long n) {
    StopWatch t;
    t.start();
    int amax = INT_MIN;
    int amin = INT_MAX;

    for(long i = 0; i < n - 1; ++i) {
        amin = INT_MAX;
        for(long j = 0; j < m-1; j++) {
            amin = min (amin, A[i][j]);
        }
        amax = max(amax, amin);
    }

    return {amin, amax};
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

    MinMax mm(input, m);
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

        cout << "minmax" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}