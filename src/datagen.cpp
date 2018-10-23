//
// Created by victorn on 22/10/18.
//
#include <iostream>
#include "param.h"
#include "omp.h"
#include <random>
#include "datagen.h"

using namespace std;

int** create_rand_int_2D_matrix(long m, long n) {
    int** input;

    input = (int**) malloc(n * sizeof(int*));

    for(long i = 0; i < n; i++) {
        input[i] = (int*) malloc(m * sizeof(int));
    }


#pragma omp parallel for
    for(long i = 0; i < n; i++) {
        std::mt19937 rng;
        rng.seed(std::random_device()());
        std::uniform_int_distribution<std::mt19937::result_type> dist122(0, 255);
        for(long j =0; j < m; j++){
            input[i][j] =  (int)dist122(rng) - 122;
        }
    }
}

int*** create_rand_int_3D_matrix(long l, long m, long n) {
    int*** input;

    input = (int***) malloc(n * sizeof(int**));

    for(long i = 0; i < n; i++) {
        input[i] = (int**) malloc(m * sizeof(int*));

        for(long j = 0; j< m; j++) {
            input[i][j] = (int*) malloc(l * sizeof(int));
        }
    }

#pragma omp parallel for
    for(long i = 0; i < n; i++) {
        std::mt19937 rng;
        rng.seed(std::random_device()());
        std::uniform_int_distribution<std::mt19937::result_type> dist122(0, 255);
        for(long j =0; j < m; j++){
            for(long k =0; k < l; k++) {
                input[i][j][k] = (int) dist122(rng) - 122;
            }
        }
    }
}

