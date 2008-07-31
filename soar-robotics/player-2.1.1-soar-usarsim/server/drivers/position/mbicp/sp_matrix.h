/*
  J. Neira
  J. A. Castellanos
  Robotics and Real Time Group
  University of Zaragoza
  
  sp_matrix.h
  Implements basic MATRIX operations
*/

#ifndef _SP_MATRIX_H
#define _SP_MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ROWS (7)
#define MAX_COLS (7)

typedef struct {
  int   rows;
  int   cols;
  float data[MAX_ROWS][MAX_COLS];
} MATRIX;

typedef struct {
  int   elements;
  float data[MAX_ROWS];
} VECTOR;

#define DOF (3)

typedef struct {
  int mat[DOF];
  int range;
} BMAT;

#define MROWS(m)             ((m).rows)
#define MCOLS(m)             ((m).cols)
#define MDATA(m,i,j)         ((m).data[i][j])

#define VELEMENTS(v)         ((v).elements)
#define VDATA(v,i)           ((v).data[i])

#define M_SQUARE(m)          ((m).rows == (m).cols)
#define M_COMPAT_DIM(m, n)   ((m).cols == (n).rows)
#define M_EQUAL_DIM(m, n)    (((m).rows == (n).rows)  && ((m).cols == (n).cols))
#define V_EQUAL_DIM(v, w)    (((v).elements == (w).elements))
#define MV_COMPAT_DIM(m, v)  ((m).cols == (v).elements)

#define FIRST(b)             ((b).mat[0])
#define SECOND(b)            ((b).mat[1])
#define THIRD(b)             ((b).mat[2])
#define RANGE(b)             ((b).range)

#define SQUARE(x)            ((x)*(x))

MATRIX create_matrix (int rows, int cols);
void initialize_matrix (MATRIX *m, int rows, int cols);
void diagonal_matrix (MATRIX *m, int dim, float el1, float el2, float el3);
void print_matrix (char *message, MATRIX const *m);
VECTOR create_vector (int elements);
void initialize_vector (VECTOR *v, int elements);
void print_vector (char *message, VECTOR const *v);
float cross_product (MATRIX const *m, int f1, int c1, int f2, int c2);
int determinant (MATRIX const *m, float *result);
int inverse_matrix (MATRIX const *m, MATRIX *n);
int multiply_matrix_vector (MATRIX const *m, VECTOR const *v, VECTOR *r);

#ifdef __cplusplus
}
#endif

#endif

