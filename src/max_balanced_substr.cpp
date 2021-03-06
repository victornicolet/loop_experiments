#include <tbb/tbb.h>
#include <iostream>
#include <omp.h>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

using namespace std;
using namespace tbb;

struct MWSS {
    bool *A;
    long n;

    int max_length;
    int offset;
    bool balance;

    MWSS(bool* _input, long _n) : A(_input), n(_n), max_length(0), offset(0), balance(true) {}

    MWSS(MWSS& s, split) {
        max_length = 0;
        offset = 0;
        balance = true;
        A = s.A;
        n = s.n;
    }

    void operator()( const blocked_range<long>& r ) {

        for(long i = r.begin(); i != r.end(); ++i) {
            offset = 0;
            balance = true;

            for(long j = i; j < n; j++) {

                offset += A[j] ? 1 : -1;

                if (offset == 0 && balance) {
                    max_length = max(max_length, static_cast<int>(j - i + 1));
                }

                if (offset <= 0 ) {
                    balance = false;
                }
            }
        }

    }

    void join(MWSS& r) {
        offset = r.offset;
        balance = r.balance;
        max_length = max(r.max_length, max_length);

    }

};

int seq_impl(const bool *A, long n) {

    int offset;
    int max_length = INT_MIN;
    bool balance;

    for(long i = 0; i < n; ++i) {
        offset = 0;
        balance = true;

        for(long j = i; j < n; j++) {

            offset += A[j] ? 1 : -1;

            if (offset == 0 && balance) {
                max_length = max(max_length, static_cast<int>(j - i + 1));
            }

            if (offset <= 0 ) {
                balance = false;
            }
        }
    }

    return max_length;
}

double do_seq(const bool* A, long n) {
    // Run once before measuring for stability
    seq_impl(A,n);
    StopWatch t;
    t.start();
    seq_impl(A,n);
    return t.stop();
}




double do_par(bool *input, long n, int num_cores) {
    StopWatch t;
    double elapsed = 0.0;
    // TBB Initialization with num_cores cores
    static task_scheduler_init init(task_scheduler_init::deferred);
    init.initialize(num_cores, UT_THREAD_DEFAULT_STACK_SIZE);

    MWSS mwss(input, n);
    parallel_reduce(blocked_range<long>(0, n-1), mwss);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), mwss);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}

int main(int argc, char** argv) {
    if(argc <= 1) {
        cout << "Usage: ./ExpMaxBalSub [N]" << endl;
        return  -1;
    }

    long n;
    n = atoi(argv[1]);
    // Data allocation and initialization
    bool* input;
    input = create_rand_bool_1D_array(n);

    double exp_time;

    for(int num_threads = 0; num_threads <= EXP_MAX_CORES; num_threads++) {
        if (num_threads > 0) {
            // Do the parallel experiment.
            exp_time = do_par(input, n, num_threads);
        } else {
            // Do the sequential experiment.
            exp_time = do_seq(input, n);
        }

        cout << "max-balanced-substr" << "," << num_threads << ", " << exp_time << endl;
    }
    return 0;
}