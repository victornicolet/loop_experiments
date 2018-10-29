//
// Created by victorn on 22/10/18.
//

#ifndef NESTEDLOOPSEXPERIMENTS_DATAGEN_H
#define NESTEDLOOPSEXPERIMENTS_DATAGEN_H

int* create_rand_int_1D_array(long n);
int** create_rand_int_2D_matrix(long num_cols, long num_rows);
int*** create_rand_int_3D_matrix(long l, long m, long n);
bool** create_rand_bool_2D_matrix(long m, long n);
bool* create_rand_bool_1D_array(long n);

#endif //NESTEDLOOPSEXPERIMENTS_DATAGEN_H
