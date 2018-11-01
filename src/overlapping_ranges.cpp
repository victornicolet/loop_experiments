#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

using namespace std;
using namespace tbb;


struct Overlaps {
    int **A;
    long m;

    int high, h, low, l;
    int aux_incl44, aux_incl45;
    bool incl;

    Overlaps(int** _input, long rl) : A(_input), m(rl),
                                      high(INT_MAX), h(INT_MAX), low(INT_MIN), l(INT_MIN),
                                      incl(true){}

    Overlaps(Overlaps& s, split) {
        high = INT_MAX; h = INT_MAX;
        low = INT_MIN; l = INT_MIN;
        incl = true;
        A = s.A;
        m = s.m;
    }

    void operator()( const blocked_range<long>& r ) {

        long e = r.begin();

        for(long i = e; i != r.end(); ++i) {
            low = 0;
            high = 0;

            for(long j = 0; j < m; j++) {
                high = max(high, A[i][j]);
                low = min(low, A[i][j]);
            }

            if (i == e) {
                aux_incl44 = high;
                aux_incl45 = low;
            }

            h = min(h, high);
            l = min(l, low);

            incl = incl && h > l;
        }
    }

    void join(Overlaps& r) {
        low = r.low;
        high = r.high;
        l = r.l;
        h = r.h;
        incl = (r.aux_incl44 >= h) ? false : ((aux_incl45 <= r.l) ? (r.incl && incl) : false);
    }

};

bool seq_implem(int **A, long m, long n) {
    int sum= 0;
    int l = 0;
    int h = 0;
    bool incl = true;


    for(long i = 0; i < n; ++i) {
        int low = 0;
        int high = 0;
        for(long j = 0; j < m; j++) {
            high = max(high, A[i][j]);
            low = min(low, A[i][j]);
        }
        h = min(h, high);
        l = min(l, low);
        incl = incl && h > l;
    }

    return incl;
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

    Overlaps overlaps(input, m);
    parallel_reduce(blocked_range<long>(0, n-1), overlaps);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), overlaps);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}


int main(int argc, char** argv) {
    // Data size:
    if(argc < 3) {
        cout << "Usage:./ExpOverlap [NUM_ROWS] [NUM_COLS]" << endl;
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

        cout << "overlapping-ranges" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}