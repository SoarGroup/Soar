/* binomial.c
 *
 *    Author: Jon Hamkins (copied from Numerical Recipes, p. 215)
 *       Date  : 3-9-98
 *         
 *    Routines to compute binomial coefficients in an efficient way that
 *    avoids overflow errors.
 *               */

#include <math.h>

/* returns ln(Gamma(xx)) for xx > 0 */
double gammln(double xx)
{
  double x,y,tmp,ser;
  static double cof[6]={76.18009172947146,-86.50532032941677,
    24.01409824083091,-1.231739572450155,0.1208650973866179e-2,
    -0.5395239384953e-5};
  int j;

  y=x=xx;
  tmp=x+5.5;
  tmp -= (x+0.5)*log(tmp);
  ser = 1.000000000190015;
  for (j=0;j<=5;j++) ser += cof[j]/++y;
  return -tmp+log(2.5066282746310005*ser/x);
}

/* returns ln(n!) */
double factln(int n)
{
  static double a[256];

  if (n<0) {
    return -1.0;
  }
  if (n<=1) return 0.0;
  /* check if in range of table */
  if (n<=255) return a[n] ? a[n] : (a[n] = gammln(n+1.0));
  /* out of range of table */
  else return gammln(n+1.0);
}

/* returns n choose k, as a double */
double binomial(int n, int k)
{
  return floor(0.5 + exp(factln(n)-factln(k)-factln(n-k)));
}

/* returns ln(n choose k) -- use this when n choose k overflows
 *  * Note: exp(binomialln(x)) does not give an integer exactly */
double binomialln(int n, int k)
{
  return(factln(n)-factln(k)-factln(n-k));
}

/* Returns the value n! as a floating-point number. */
double factrl(int n)
{
  double gammln(double xx);
  static int ntop=4;
  /* Fill in table only as required. */
  static double a[33]={1.0,1.0,2.0,6.0,24.0}; 
  int j;

  if (n < 0) { return -1.0; }
  if (n > 32) return exp(gammln(n+1.0));
  /* Larger value than size of table is required. Actually, this big a value
   *      is going to over flow on many computers, but no harm in trying. */
  while (ntop<n) { /* Fill in table up to desired value. */
    j=ntop++;
    a[ntop]=a[j]*ntop;
  }
  return a[n];
}

