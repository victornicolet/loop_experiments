#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"
#include "omp.h"

using namespace std;
using namespace tbb;


struct Mode {
    int *A;
    long n;

    int elt;
    int count;
    int max_cnt;
    int mode;

    Mode(int* _input, long rl) : A(_input), n(rl), elt(0), count(0), max_cnt(0), mode(0){}

    Mode(Mode& s, split) {mode = 0; count = 0; max_cnt = 0; elt = 0; A = s.A; n = s.n; }

    void operator()( const blocked_range<long>& r ) {

        for(long i = r.begin(); i != r.end(); ++i) {

            elt = A[i];
            count = 1;

            for(long j = 0; j < n-1; j++) {
                if(A[j] == elt) {
                    count++;
                }
            }

            if (count > max_cnt) {
                mode = elt;
            }
            max_cnt = max(max_cnt, count);

        }
    }

    void join(Mode& r) {
        count = r.count;
        elt = r.elt;
        max_cnt = max(r.max_cnt, max_cnt);
        mode =  (r.max_cnt > max_cnt) ? r.mode : mode;
    }

};

int seq_impl(int *A, long n) {
    int elt = 0;
    int mode = 0;
    int count = 0;
    int max_cnt = 0;

    for(long i = 0; i < n; ++i) {

        elt = A[i];
        count = 1;

        for(long j = 0; j < n-1; j++) {
            if(A[j] == elt) {
                count++;
            }
        }

        if (count > max_cnt) {
            mode = elt;
        }
        max_cnt = max(max_cnt, count);

    }

    return mode;
}

double do_seq(int *A, long n) {
    seq_impl(A,n);
    StopWatch t;
    t.start();
    seq_impl(A,n);
    return t.stop();
}


double do_par(int *input, long n, int num_cores) {
    StopWatch t;
    double elapsed = 0.0;
    // TBB Initialization with num_cores cores
    static task_scheduler_init init(task_scheduler_init::deferred);
    init.initialize(num_cores, UT_THREAD_DEFAULT_STACK_SIZE);

    Mode mode(input, n);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), mode);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}


int main(int argc, char** argv) {
    // Data size:
    if(argc < 2) {
        cout << "Usage:./ExpMode [NUM_ELTS]" << endl;
        return  -1;
    }

    int n = atoi(argv[1]);
    // Data allocation and initialization
    int *input;
    input = create_rand_int_1D_array(n);


    for(int num_threads = 0; num_threads <= EXP_MAX_CORES; num_threads++) {
        double exp_time = 0.0;

        if (num_threads > 0) {
            // Do the parallel experiment.
            exp_time = do_par(input, n, num_threads);
        } else {
            // Do the sequential experiment.
            exp_time = do_seq(input, n);
        }

        cout << "mode" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}