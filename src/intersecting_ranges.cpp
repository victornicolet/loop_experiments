//
// Created by victorn on 12/11/18.
//

#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"
#include "omp.h"

using namespace std;
using namespace tbb;


struct IntersectingRanges {
    int *H;
    int *L;
    long n;

    bool b;
    bool b0;

    IntersectingRanges(int* hbounds, int* lbounds, long len) :
            H(hbounds), L(lbounds), n(len), b(false), b0(false) {}

    IntersectingRanges(IntersectingRanges& s, split): H(s.H), L(s.L), n(s.n), b(false), b0(false) {}

    void operator()( const blocked_range<long>& r ) {

        for(long i = r.begin(); i != r.end(); ++i) {

            bool b0 = false;

            for(long j = 0; j < n; j++) {
                b0 = b0 ||
                     (((H[i] <= H[j]) && (L[j] <= H[i])) ||
                      ((H[j] <= H[i]) && (L[i] <= H[j])));
            }

            b = b || b0;

        }
    }

    void join(IntersectingRanges& r) {
        b = b || r.b;
    }

};

bool seq_impl(int *H, int *L, long n) {
    bool b = false;

    for(long i = 0; i < n; i++) {

        bool b0 = false;

        for(long j = 0; j < n; j++) {
            b0 = b0 ||
                 (((H[i] <= H[j]) && (L[j] <= H[i])) ||
                  ((H[j] <= H[i]) && (L[i] <= H[j])));
        }

        b = b || b0;

    }

    return b;
}

double do_seq(int *H, int* L, long n) {
    seq_impl(H, L, n);
    StopWatch t;
    t.start();
    seq_impl(H, L, n);
    return t.stop();
}


double do_par(int *H, int *L, long n, int num_cores) {
    StopWatch t;
    double elapsed = 0.0;
    // TBB Initialization with num_cores cores
    static task_scheduler_init init(task_scheduler_init::deferred);
    init.initialize(num_cores, UT_THREAD_DEFAULT_STACK_SIZE);

    IntersectingRanges intersectingRanges(H, L, n);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), intersectingRanges);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}


int main(int argc, char** argv) {
    // Data size:
    if(argc < 2) {
        cout << "Usage:./ExpIntersectingRanges [NUM_ELTS]" << endl;
        return  -1;
    }

    int n = atoi(argv[1]);
    // Data allocation and initialization
    int* H = create_rand_int_1D_array(n);
    int * L = create_rand_int_1D_array(n);

    for(int num_threads = 0; num_threads <= EXP_MAX_CORES; num_threads++) {
        double exp_time;

        if (num_threads > 0) {
            // Do the parallel experiment.
            exp_time = do_par(H, L, n, num_threads);
        } else {
            // Do the sequential experiment.
            exp_time = do_seq(H, L, n);
        }

        cout << "intersecting-ranges" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}