
/**************************************************************************
 * Desc: Useful pdf functions
 * Author: Andrew Howard
 * Date: 10 Dec 2002
 * CVS: $Id: pf_pdf.h 1658 2003-08-09 21:35:36Z inspectorg $
 *************************************************************************/

#ifndef PF_PDF_H
#define PF_PDF_H

#include "pf_vector.h"

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
 * Gaussian
 *************************************************************************/

// Gaussian PDF info
typedef struct
{
  // Mean, covariance and inverse covariance
  pf_vector_t x;
  pf_matrix_t cx;
  pf_matrix_t cxi;
  double cxdet;

  // Decomposed covariance matrix (rotation * diagonal)
  pf_matrix_t cr;
  pf_vector_t cd;

  // A random number generator
  gsl_rng *rng;

} pf_pdf_gaussian_t;


// Create a gaussian pdf
pf_pdf_gaussian_t *pf_pdf_gaussian_alloc(pf_vector_t x, pf_matrix_t cx);

// Destroy the pdf
void pf_pdf_gaussian_free(pf_pdf_gaussian_t *pdf);

// Compute the value of the pdf at some point [z].
double pf_pdf_gaussian_value(pf_pdf_gaussian_t *pdf, pf_vector_t z);

// Generate a sample from the the pdf.
pf_vector_t pf_pdf_gaussian_sample(pf_pdf_gaussian_t *pdf);


/**************************************************************************
 * Discrete
 *************************************************************************/

// Discrete PDF info
typedef struct
{
  // The list of discrete probs
  int prob_count;
  double *probs;

  // A random number generator
  gsl_rng *rng;

  // The discrete prob generator
  gsl_ran_discrete_t *ran;

} pf_pdf_discrete_t;


// Create a discrete pdf
pf_pdf_discrete_t *pf_pdf_discrete_alloc(int count, double *probs);

// Destroy the pdf
void pf_pdf_discrete_free(pf_pdf_discrete_t *pdf);

// Compute the value of the probability of some element [i]
double pf_pdf_discrete_value(pf_pdf_discrete_t *pdf, int i);

// Generate a sample from the the pdf.
int pf_pdf_discrete_sample(pf_pdf_discrete_t *pdf);

#ifdef __cplusplus
}
#endif

#endif
