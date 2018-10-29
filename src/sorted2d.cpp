#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

using namespace std;
using namespace tbb;

static int __min_int = static_cast<int>(INT64_MIN);
static int __max_int = static_cast<int>(INT64_MAX);

struct Sorted2D {
    int **A;
    bool sorted;
    int prev;
    int first;
    long m;

    Sorted2D(int** _input, long rl) :
            A(_input), m(rl), sorted(true), prev(__max_int), first(__min_int){}

    Sorted2D(Sorted2D& s, split) {
        sorted = true;
        prev = __max_int;
        first = __min_int;
        A = s.A;
        m = s.m;
    }

    void operator()( const blocked_range<long>& r ) {
        bool bl = sorted;
        int loc_prev = prev;
        for(long i = r.begin(); i != r.end(); ++i) {
            for(long j = 0; j < m-1; j++) {
                bl = bl && loc_prev > A[i][j];
                loc_prev = A[i][j];
            }
        }
        first =
        prev = loc_prev;
        sorted = bl;
    }

    void join(Sorted2D& rhs) {
        sorted = sorted && rhs.sorted && rhs.prev > first;
        first = rhs.first;
    }

};


bool implem_seq(int **A, long m, long n) {
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
    implem_seq(A, m, n);

    StopWatch t;
    double elapsed = 0.0;
    for(int i = 0; i < NUM_REPEAT; i++) {
        t.start();
        implem_seq(A, m, n);
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

    Sorted2D sr(input, m);
    parallel_reduce(blocked_range<long>(0, n-1), sr);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), sr);
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

        cout << "2d-sorted" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}