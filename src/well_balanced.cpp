//
// Created by victorn on 23/10/18.
//
#include <tbb/tbb.h>
#include <iostream>
#include <omp.h>
#include "StopWatch.h"
#include "param.h"
#include "datagen.h"

#define N_20SEC 100000
#define M_20SEC 30000

using namespace std;
using namespace tbb;

struct inner_loop {
    int line_offset;
    int min_offset;
    bool balance;
};

struct Bal {
    bool **A;
    long m;

    inner_loop* aux;

    Bal(bool** _input, inner_loop* aux, long rl) : A(_input), aux(aux), m(rl) {}


    void operator()(const blocked_range<long>& r ) const {
        for(long i = r.begin(); i != r.end(); ++i) {
            int line_offset = 0;
            int min_offset = 0;
            int balance = true;

            for(long j = 0; j < m; j++) {
                line_offset += A[i][j] ? 1 : -1;
                min_offset = min(min_offset, line_offset);
                balance = balance && line_offset > 0;
            }

            aux[i] = {line_offset, min_offset, balance};
        }

    }
};

double do_seq(bool** A, long m, long n) {
    StopWatch t;

    int count = 0;
    int offset = 0;
    int line_offset = 0;
    bool bal = true;

    t.start();

    for(long i = 0; i < n; ++i) {

        line_offset = 0;

        for(long j = 0; j < m; j++) {
            offset += A[i][j] ? 1 : -1;
            bal = bal && (offset + line_offset) >= 0;
        }
        offset += line_offset;

        if(bal && line_offset >= 0){
            count++;
        }
    }

    return t.stop();
}

double map_and_reduce_bal(Bal bp, long m, long n) {
    parallel_for(blocked_range<long>(0, n-1), bp);
    inner_loop* aux = bp.aux;

    int count = 0;
    int bal = true;
    int offset = 0;

    for(long i = 0; i < n; i++){
        bal = bal && (offset + aux[i].min_offset) > 0;
        offset += aux[i].line_offset;
        if(bal && aux[i].line_offset > 0) {
            count ++;
        }
    }
    return count;
}

double do_par(bool** input, long m, long n, int num_cores) {
    StopWatch t;
    double elapsed = 0.0;
    // TBB Initialization with num_cores cores
    static task_scheduler_init init(task_scheduler_init::deferred);
    init.initialize(num_cores, UT_THREAD_DEFAULT_STACK_SIZE);

    auto aux = new inner_loop[n];

    Bal balpar(input, aux, m);

    for(int i = 0; i < NUM_REPEAT ; i++){
        t.start();
        map_and_reduce_bal(balpar, m, n);
        elapsed += t.stop();
    }

    init.terminate();

    return elapsed / NUM_REPEAT;
}


int main(int argc, char** argv) {
    // Data size:
    if(argc < 3) {
        cout << "Usage:./ExpWellBalanced [NUM_ROWS] [NUM_COLS]" << endl;
        return  -1;
    }


    long n,m;

    n = atoi(argv[1]);
    m = atoi(argv[2]);

    // Data allocation and initialization
    bool** input;
    input = create_rand_bool_2D_matrix(m,n);

    double exp_time = 0.0;

    for(int num_threads = 0; num_threads <= EXP_MAX_CORES; num_threads++) {
        if (num_threads > 0) {
            // Do the parallel experiment.
            exp_time = do_par(input, m, n, num_threads);
        } else {
            // Do the sequential experiment.
            exp_time = do_seq(input, m, n);
        }

//        CSV LINE : Prog. name, N, M, L, Num threads used, Exp time, OpenMP exp time
        cout << "well-balanced" << "," <<  num_threads << ", " << exp_time << endl;
    }
    return 0;
}