#include <tbb/tbb.h>
#include <iostream>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

using namespace std;
using namespace tbb;


struct MTRR {
    int **A;
    long m;

    int *c;
    int *aux;
    int mtr;
    int mtrr;


    MTRR(int** _input, int* c, int *aux, long rl) :
            A(_input), c(c), aux(aux), m(rl), mtr(0), mtrr(0) {}

    MTRR(MTRR& s, split) : m(s.m), A(s.A), mtr(0), mtrr(0) {
        c = new int[s.m] {0};
        aux = new int[s.m] {0};
    }

    void operator()( const blocked_range<long>& r ) {

        for (long i = r.begin(); i != r.end(); i++) {
            long j2 = 0;
            int acc_aux = 0;
            mtr = 0;
            for (long j = 0; j < m; j++) {
                c[j] += A[i][j];
                mtr = max(mtr + c[j], 0);
                j2 = m - j - 1;
                acc_aux += c[j2] + A[i][j2];
                aux[j2] = max(acc_aux, aux[j2]);
            }
            mtrr = max(mtr, mtrr);
        }
    }

    void join(MTRR& r) {
        mtr = 0;
        long j2 = 0;
        int acc_aux = 0;
        for(long j = 0; j < m; j++){
            c[j] = c[j] + r.c[j];
            j2 = m - j - 1;
            acc_aux += c[j];
            aux[j2] = max(acc_aux + r.aux[j2], aux[j2]);
            mtr = max(aux[j2], mtr);
        }
        mtrr = max(mtr, mtrr);
    }

};

int seq_implem(int **A, long m, long n) {
    int sum= 0;
    int c[m] = {0};
    int mtrr = 0;

    for(long i = 0; i < n - 1; ++i) {
        for(long j = 0; j < m-1; j++) {
            sum += A[i][j];
        }
    }
    for (long i = 0; i < n; i++) {
        long j2 = 0;
        int mtr = 0;
        for (long j = 0; j < m; j++) {
            c[j] += A[i][j];
            mtr = max(mtr + c[j], 0);
            j2 = m - j - 1;
        }
        mtrr = max(mtr, mtrr);
    }

    return mtrr;
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


double do_par(int** const A, long m, long n, int num_cores) {
    StopWatch t;
    double elapsed = 0.0;
    int c[m] = {0};
    int aux[m] = {0};
    // TBB Initialization with num_cores cores
    static task_scheduler_init init(task_scheduler_init::deferred);
    init.initialize(num_cores, UT_THREAD_DEFAULT_STACK_SIZE);

    MTRR mtrr(A, c, aux, m);
    parallel_reduce(blocked_range<long>(0, n-1), mtrr);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        parallel_reduce(blocked_range<long>(0, n-1), mtrr);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}


int main(int argc, char** argv) {
    // Data size:
    if(argc < 3) {
        cout << "Usage:./ExpMTRR [NUM_ROWS] [NUM_COLS]" << endl;
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

        cout << "mtrr" << "," << num_threads << "," << exp_time << endl;
    }

    return 0;
}