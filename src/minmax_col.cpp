#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

using namespace std;
using namespace tbb;

struct minmaxcol_res {
    int *amin;
    int amaxmin;
};

struct MinMaxCol {
    int **A;
    int *amin;
    int amaxmin;
    long m;

    MinMaxCol(int** _input, long rl) : A(_input), m(rl), amaxmin(INT_MIN){
        amin = new int[rl];
    }

    MinMaxCol(MinMaxCol& s, split) {
        amaxmin = INT_MIN;
        A = s.A;
        m = s.m;
        amin = new int[s.m];
    }

    void operator()( const blocked_range<long>& r ) {
        int _amaxmin = amaxmin;
        int* _amin = amin;

        for(long i = r.begin(); i != r.end(); ++i) {
            for(long j = 0; j < m; j++) {
                _amin[j] = min(_amin[j], A[i][j]);
                _amaxmin = max(_amaxmin, _amin[j]);
            }
        }

        amaxmin = _amaxmin;
        amin = _amin;

    }

    void join(MinMaxCol& rhs) {
        for(long j = 0; j < m; j++) {
            amin[j] = min(amin[j], rhs.amin[j]);
            amaxmin = max(amin[j], amaxmin);
        }
    }

};

minmaxcol_res seq_implem(int **A, long m, long n) {
    int _amaxmin = INT_MIN;
    int* _amin = new int[m];

    for(long i = 0; i < n; ++i) {
        for(long j = 0; j < m; j++) {
            _amin[j] = min(_amin[j], A[i][j]);
            _amaxmin = max(_amaxmin, _amin[j]);
        }
    }

    return {_amin, _amaxmin};
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

    MinMaxCol mm(input, m);
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

        cout << "minmax-col" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}
