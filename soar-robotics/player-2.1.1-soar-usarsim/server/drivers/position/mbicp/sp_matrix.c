#include <stdio.h>
#include "sp_matrix.h"

/*****************************************************************************/
 MATRIX create_matrix (int rows, int cols)

/*****************************************************************************
 Creates a MATRIX of dimensions (rows, cols) and initializaes it to zeros.
******************************************************************************/
{
  MATRIX m; 

  MROWS (m) = rows;
  MCOLS (m) = cols;

  {
    int i, j;

    for (i = 0; i < MROWS (m); i++)
        for (j = 0; j < MCOLS (m); j++)
            MDATA (m, i, j) = 0;
  }

  return m;
}

/*****************************************************************************/
 void initialize_matrix (MATRIX *m, int rows, int cols)

/*****************************************************************************
 Initializes a MATRIX to dimensions (rows, cols) and content zeros.
******************************************************************************/
{
  MROWS (*m) = rows;
  MCOLS (*m) = cols;

  {
    int i, j;

    for (i = 0; i < MROWS (*m); i++)
        for (j = 0; j < MCOLS (*m); j++)
            MDATA (*m, i, j) = 0;
  }

}


/*****************************************************************************/
 void print_matrix (char *message, MATRIX const *m)

/*****************************************************************************
 Print to stdout the contents of MATRIX m.
******************************************************************************/
{
  int i, j;

  printf ("%s\n",message);
  printf("%d %d \n",MROWS (*m),MCOLS (*m));
  if ((MROWS (*m) <= MAX_ROWS) && (MCOLS (*m) <= MAX_COLS))
    for (i = 0; i < MROWS (*m); i++)
    {
        for (j = 0; j < MCOLS (*m); j++)
            printf ("%10.5f ", MDATA (*m, i, j));
        printf ("\n");
    }
  else printf ("Dimension incorrecta!");
  printf ("\n");
}

/*****************************************************************************/
 VECTOR create_vector (int elements)

/*****************************************************************************
 Initializes a VECTOR to dimension (elements) and its contents to zeros.
******************************************************************************/
{
  VECTOR v;

  VELEMENTS (v) = elements;

  {
    int i;

    for (i = 0; i < VELEMENTS (v); i++)
        VDATA (v, i) = 0;
  }

  return v;
}

/*****************************************************************************/
 void initialize_vector (VECTOR *v, int elements)

/*****************************************************************************
 Initializes a VECTOR to dimension (elements) and its contents to zeros.
******************************************************************************/
{
  VELEMENTS (*v) = elements;

  {
    int i;

    for (i = 0; i < VELEMENTS (*v); i++)
        VDATA (*v, i) = 0;
  }
}

/*****************************************************************************/
 void print_vector (char *message, VECTOR const *v)
 
/*****************************************************************************
 Print to stdout the contents of VECTOR m.
******************************************************************************/
{
  int i;

  printf ("%s\n",message);
  if (VELEMENTS (*v) <= MAX_ROWS)
    for (i = 0; i < VELEMENTS (*v); i++)
    {
        printf ("%f ", VDATA (*v, i));
        printf ("\n");
    }
  else printf ("Dimension incorrecta!");
  printf ("\n");
}

/*****************************************************************************/
 float cross_product (MATRIX const *m, int f1, int c1, int f2, int c2)

/*****************************************************************************
******************************************************************************/
{
  return MDATA (*m, f1, c1) * MDATA (*m, f2, c2) - MDATA (*m, f1, c2) * MDATA (*m, f2, c1);
}

/*****************************************************************************/
int determinant (MATRIX const *m, float *result)
/*****************************************************************************
******************************************************************************/
{
  if (!M_SQUARE (*m))
  {
     printf ("ERROR (determinant): MATRIX must be square!\n");
     print_matrix ("MATRIX:", m);	
	 return -1;     
  }
  else
  {    

    if (MROWS (*m) == 1)
       *result = MDATA (*m, 0, 0);
    else if (MROWS (*m) == 2)
       *result = cross_product (m, 0, 0, 1, 1);
    else
       *result = MDATA (*m, 0, 0) * cross_product (m, 1, 1, 2, 2)
              - MDATA (*m, 0, 1) * cross_product (m, 1, 0, 2, 2)
              + MDATA (*m, 0, 2) * cross_product (m, 1, 0, 2, 1);


    return 1;
  }  
}

/*****************************************************************************/
 int inverse_matrix (MATRIX const *m, MATRIX *n)

/*****************************************************************************
******************************************************************************/
{
  if (!M_SQUARE (*m))
  {	 
     printf ("ERROR (inverse_matrix): MATRIX must be square!\n");
     print_matrix ("MATRIX:", m);
	 n->cols=0; n->rows=0;
     return -1;
  }
  else
  {
    float det;
	int res;

    res = determinant (m,&det);

    if (res == -1)
    {
       printf ("ERROR (inverse_matrix): singular MATRIX!\n");
       print_matrix ("MATRIX:", m);
       return -1;
    }
    else
    {
      initialize_matrix (n, MROWS (*m), MCOLS (*m));
      if (MROWS (*m) == 1)
      {
        MDATA (*n, 0, 0) = 1 / det ;
      }
      else if (MROWS (*m) == 2)
      {
        MDATA (*n, 0, 0) = MDATA (*m, 1, 1) / det ;
        MDATA (*n, 0, 1) = -MDATA (*m, 0, 1) / det ;
        MDATA (*n, 1, 0) = -MDATA (*m, 1, 0) / det ;
        MDATA (*n, 1, 1) = MDATA (*m, 0, 0) / det ;
      }
      else
      {
        MDATA (*n, 0, 0) = cross_product (m, 1, 1, 2, 2) / det ;
        MDATA (*n, 0, 1) = -cross_product (m, 0, 1, 2, 2) / det ;
        MDATA (*n, 0, 2) = cross_product (m, 0, 1, 1, 2) / det ;
        MDATA (*n, 1, 0) = -cross_product (m, 1, 0, 2, 2) / det ;
        MDATA (*n, 1, 1) = cross_product (m, 0, 0, 2, 2) / det ;
        MDATA (*n, 1, 2) = -cross_product (m, 0, 0, 1, 2) / det ;
        MDATA (*n, 2, 0) = cross_product (m, 1, 0, 2, 1) / det ;
        MDATA (*n, 2, 1) = -cross_product (m, 0, 0, 2, 1) / det ;
        MDATA (*n, 2, 2) = cross_product (m, 0, 0, 1, 1) / det ;
      }	  
	}
  }
  return 1;
}

/*****************************************************************************/
 int multiply_matrix_vector (MATRIX const *m, VECTOR const *v, VECTOR *r)

/*****************************************************************************
 Returns the VECTOR-MATRIX product of m and v in r.
******************************************************************************/
{
  if (! (MV_COMPAT_DIM (*m, *v)))
  {
     printf ("ERROR (multiply_matrix_vector): MATRIX  and VECTOR dimensions incompatible!\n");
     print_matrix ("MATRIX:", m);
     print_vector ("VECTOR:", v);
     return -1; /*added 1996-07*/
  }
  else
  {
    int i, j;
    float datum;

    VELEMENTS (*r) = MROWS (*m);

    for (i = 0; i < MROWS (*m); i++)
    {
        datum = 0;
        for (j = 0; j < VELEMENTS (*v); j++)
            datum = datum + MDATA (*m, i, j) * VDATA (*v, j);
        VDATA (*r, i) = datum;
    }
  }
  return 1;
}

