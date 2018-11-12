#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

using namespace std;
using namespace tbb;


struct MaxLeftRec {
    int **A;
    long m;
    int rs;
    int mlr;
    int *rects;
    int *mts_rects;

    MaxLeftRec(int** _input, long rl) : A(_input), m(rl), rs(0), mlr(0) {
        rects = new int[rl];
        mts_rects = new int[rl];
    }

    MaxLeftRec(MaxLeftRec& s, split) : mlr(0), rs(0), A(s.A), m(s.m) {
        rects = new int[s.m];
        mts_rects = new int[s.m];
    }

    void operator()( const blocked_range<long>& r ) {
        int lrs = 0;
        int lmlr = 0;

        for(long i = r.begin(); i != r.end(); ++i) {
            lrs = 0;
            lmlr = 0;
            for(long j = 0; j < m; j++) {
                lrs += A[i][j];
                rects[j] += lrs;
                mts_rects[j] = max(mts_rects[j] + lrs, 0);
                lmlr = max(lmlr, rects[j]);
            }
        }
    }

    void join(MaxLeftRec& rhs) {
        mlr = 0;
        rs = rhs.rs;
        for(long j = 0; j < m; j++) {
            rects [j] += rhs.rects[j];
            mts_rects [j] = max(rhs.mts_rects[j], mts_rects[j] + rhs.rects[j]);
            mlr = max(mlr, rects[j]);
        }
    }

};

int seq_implem(int **A, long m, long n) {
    int mts_rects[m] = {0};
    int rs = 0;
    int mlr = 0;

    for(long i = 0; i < n; ++i) {
        rs = 0;
        mlr = 0;
        for(long j = 0; j < m; j++) {
            rs += A[i][j];
            mts_rects[j] = max(mts_rects[j] + rs, 0);
            mlr = max(mlr, mts_rects[j]);
        }
    }

    return mlr;
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

    MaxLeftRec mm(input, m);
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
        cout << "Usage:./ExpMaxLeftRec [NUM_ROWS] [NUM_COLS]" << endl;
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

        cout << "max-left-rect" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}