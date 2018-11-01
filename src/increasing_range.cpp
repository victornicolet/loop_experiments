#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

using namespace std;
using namespace tbb;

struct Increases {
    int **A;
    long m;

    int high;
    int phigh;
    int aux;
    bool incr;

    Increases(int** _input, long rl) : A(_input), m(rl),
                                       high(0), phigh(0), aux(INT_MIN), incr(true) {}

    Increases(Increases& s, split) {
        high = 0; phigh = 0; aux = INT_MIN;
        A = s.A;
        m = s.m;
        incr = true;
    }

    void operator()( const blocked_range<long>& r ) {

        long e = r.begin();

        for(long i = e; i != r.end(); ++i)
        {
            high = INT_MIN;
            for(int j = 0; j < m; j++) {

                if (A[i][j] <= phigh) {
                    incr = 0;
                }

                aux = min(aux, A[i][j]);
                high = max(high, A[i][j]);
            }
            phigh = high;
        }
    }

    void join(Increases& r) {
        aux = min(aux, r.aux);
        high = r.high;
        phigh = r.phigh;
        incr = incr && (r.aux <= phigh) ? false : r.incr;
    }

};

bool seq_implem(int **A, long m, long n) {
    int low, high;
    int l = 0;
    int h = 0;
    bool incr = true;


    for(long i = 0; i < n; ++i) {
        low = 0;
        high = 0;

        for(long j = 0; j < m; j++) {
            high = max(high, A[i][j]);
            low = min(low, A[i][j]);
        }

        h = min(h, high);
        l = min(l, low);

        incr = incr && h > l;

    }

    return incr;
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

    Increases increases(input, m);
    parallel_reduce(blocked_range<long>(0, n-1), increases);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), increases);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}


int main(int argc, char** argv) {
    // Data size:
    if(argc < 3) {
        cout << "Usage:./ExpIncreasingRanges [NUM_ROWS] [NUM_COLS]" << endl;
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

        cout << "increasing-ranges" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}