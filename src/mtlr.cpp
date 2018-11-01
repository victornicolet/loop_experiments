#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

using namespace std;
using namespace tbb;


struct MTLR {
    int** A;
    long m;

    int *c;
    int *max_recs;
    int mtr;
    int mtlr;
    int sum;


    MTLR(int**  _input, int*c, int* aux, long rl) : A(_input), c(c), max_recs(aux), m(rl), sum(0), mtr(0), mtlr(0){
    }

    MTLR(MTLR& s, split) : A(s.A), sum(0), mtlr(0), mtr(0), m(s.m){
        c = new int[m] {0};
        max_recs = new int[m] {0};
        m = s.m;

    }

    void operator()(const blocked_range<long>& r ) {
        for (long i = r.begin(); i != r.end(); i++) {
            sum = 0;
            mtr = 0;
            for (long j = 0; j < m; j++) {
                sum += A[i][j];
                c[j] += sum;
                mtr = max(c[j], mtr);
                max_recs[j] = max(max_recs[j], c[j]);
            }
            mtlr = max(mtr, mtlr);
        }
    }

    void join(MTLR& r) {
        mtr = 0;
        for(long j = 0; j < m; j++){
            c[j] = c[j] + r.c[j];
            max_recs[j] = max(max_recs[j], c[j] + r.max_recs[j]);
            mtr = max(mtr, max_recs[j]);
        }
        mtlr = max(mtr, mtlr);
    }

};


struct mtlr_res {int* c; int mtr; int mtlr;};


mtlr_res seq_implem(int **A, long m, long n) {
    int sum= 0;
    int mtr =0;
    int mtlr = 0;
    int c[m] = {0};

    for(long i = 0; i < n; i++) {
        sum = 0;
        mtr = 0;
        for(long j = 0; j < m; j++) {
            sum += A[i][j];
            c[j] += sum;
            mtr = max(c[j], mtr);
        }
        mtlr = max(mtr, mtlr);
    }

    return {c, mtr, mtlr};
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


double do_par(int** A, long m, long n, int num_cores) {
    StopWatch t;
    double elapsed = 0.0;
    int c[m] = {0};
    int aux[m] = {0};
    // TBB Initialization with num_cores cores
    static task_scheduler_init init(task_scheduler_init::deferred);
    init.initialize(num_cores, UT_THREAD_DEFAULT_STACK_SIZE);

    MTLR mtlr(A, c, aux, m);
    parallel_reduce(blocked_range<long>(0, n-1), mtlr);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), mtlr);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}


int main(int argc, char** argv) {
    // Data size:
    if(argc < 3) {
        cout << "Usage:./ExpMTLR [NUM_ROWS] [NUM_COLS]" << endl;
        return  -1;
    }

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    // Data allocation and initialization
    int **input;
    input = create_rand_int_2D_matrix(m,n);

    for(int num_threads = 0; num_threads <= EXP_MAX_CORES; num_threads++) {
        double exp_time;

        if (num_threads > 0) {
            // Do the parallel experiment.
            exp_time = do_par(input, m, n, num_threads);
        } else {
            // Do the sequential experiment.
            exp_time = do_seq(input, m, n);
        }

        cout << "mtlr" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}