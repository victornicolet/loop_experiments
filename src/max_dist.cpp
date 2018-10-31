#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

using namespace std;
using namespace tbb;


struct MaxDist {
    int *A;
    int *B;
    int md;
    long lb;

    MaxDist(int* _input1, int* _input2, long rl) : A(_input1), B(_input2), lb(rl), md(INT_MIN){}

    MaxDist(MaxDist& s, split) {md= INT_MIN; A = s.A;  B = s.B; lb = s.lb; }

    void operator()( const blocked_range<long>& r ) {
        int _md = md;

        for(long i = r.begin(); i != r.end(); ++i) {
            for(long j = 0; j < lb; j++) {
                if (A[i] - B[j] > 0) {
                    _md = max(_md, A[i] - B[j]);
                } else {
                    _md = max(_md, B[j] - A[i]);
                }

            }
        }

        md = _md;
    }

    void join(MaxDist& rhs) { md = md + rhs.md; }

};

int seq_implem(int *A, int *B, long la, long lb) {
    int md = INT_MIN;

    for(long i = 0; i < la; ++i) {
        for(long j = 0; j < lb; j++) {
            if (A[i] - B[j] > 0) {
                md = max(md, A[i] - B[j]);
            } else {
                md = max(md, B[j] - A[i]);
            }

        }
    }

    return md;
}

double do_seq(int *A, int* B, long la, long lb) {
    StopWatch t;
    seq_implem(A, B, la, lb);
    double elapsed = 0.0;
    for(int i = 0; i < NUM_REPEAT; i++) {
        t.start();
        seq_implem(A, B, la, lb);
        elapsed += t.stop();
    }
    return elapsed / NUM_REPEAT;
}

double do_par(int *a, int *b, long la, long lb, int num_cores) {
    StopWatch t;
    double elapsed = 0.0;
    // TBB Initialization with num_cores cores
    static task_scheduler_init init(task_scheduler_init::deferred);
    init.initialize(num_cores, UT_THREAD_DEFAULT_STACK_SIZE);

    MaxDist maxDist(a, b, lb);
    parallel_reduce(blocked_range<long>(0, la-1), maxDist);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, la-1), maxDist);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}


int main(int argc, char** argv) {
    // Data size:
    if(argc < 3) {
        cout << "Usage:./ExpMaxDist [LEN_INPUT1] [LEN_INPUT2]" << endl;
        return  -1;
    }

    int l1 = atoi(argv[1]);
    int l2 = atoi(argv[2]);

    // Data allocation and initialization
    int *input1, *input2;
    input1 = create_rand_int_1D_array(l1);
    input2 = create_rand_int_1D_array(l2);

    for(int num_threads = 0; num_threads <= EXP_MAX_CORES; num_threads++) {
        double exp_time = 0.0;

        if (num_threads > 0) {
            // Do the parallel experiment.
            exp_time = do_par(input1, input2, l1, l2, num_threads);
        } else {
            // Do the sequential experiment.
            exp_time = do_seq(input1, input2, l1, l2);
        }

        cout << "max-dist" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}

