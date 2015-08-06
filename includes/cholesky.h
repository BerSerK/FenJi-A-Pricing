/**
 * @file   cholesky.h
 * @author YeShiwei <yeshiwei.math@gmail.com>
 * @date   Wed Jul 15 13:48:45 2015
 * 
 * @brief  cholesky.h from Numerical Recipes
 * 
 * 
 */

#ifndef __CHOLESKY_H__
#define __CHOLESKY_H__

/**
 *  Object for Cholesky decomposition of a matrix A, and related functions. 
 * 
 */
struct Cholesky{
  Int n; /**< Dimension of the matrix A. */
  MatDoub el; /**< Stores the decomposition. */
  
  /** 
   * Constructor. Given a positive-definite symmetric matrix a[0..n-1][0..n-1], construct and store its Cholesky decomposition, A D L 􏰩 LT .
   * 
   * @param a The matrix A.
   * 
   * @return 
   */
  Cholesky(MatDoub_I &a) : n(a.nrows()), el(a) {
    Int i,j,k;
    VecDoub tmp;
    Doub sum;
    if (el.ncols() != n) throw("need square matrix"); 
    for (i=0;i<n;i++) {
      for (j=i;j<n;j++) {
	for (sum=el[i][j],k=i-1;k>=0;k--) sum -= el[i][k]*el[j][k];  
	if (i == j) {
	  if (sum <= 0.0) //A, with rounding errors, is not positive-definite. 
	    throw("Cholesky failed");
	  el[i][i]=sqrt(sum);
	} else el[j][i]=sum/el[i][i];
      } 
    }
    for (i=0;i<n;i++) for (j=0;j<i;j++) el[j][i] = 0.; 
  }
  
  /** 
   * Solve the set of n linear equations A x = b, where a is a positive-definite symmetric matrix whose Cholesky decomposition has been stored. b[0..n-1] is input as the right-hand side vector. The solution vector is returned in x[0..n-1].
   * 
   * @param b righthand side.
   * @param x the unknow.
   */
  void solve(VecDoub_I &b, VecDoub_O &x) {
    Int i,k;
    Doub sum;
    if (b.size() != n || x.size() != n) throw("bad lengths in Cholesky");
    for (i=0;i<n;i++) { //Solve L 􏰩 y D b, storing y in x. 
      for (sum=b[i],k=i-1;k>=0;k--) sum -= el[i][k]*x[k]; 
      x[i]=sum/el[i][i];
    }
    for (i=n-1;i>=0;i--) { //Solve LT 􏰩 x D y.
      for (sum=x[i],k=i+1;k<n;k++) sum -= el[k][i]*x[k]; 
      x[i]=sum/el[i][i];
    } 
  }
  
  /** 
   * Multiply L y = b, where L is the lower triangular matrix in the stored Cholesky decom- position. y[0..n-1] is input. The result is returned in b[0..n-1].
   * 
   * @param y input
   * @param b result
   */
  void elmult(VecDoub_I &y, VecDoub_O &b) {
    
    Int i,j;
    if (b.size() != n || y.size() != n) throw("bad lengths"); 
    for (i=0;i<n;i++) {
      b[i] = 0.;
      for (j=0;j<=i;j++) b[i] += el[i][j]*y[j]; 
    }
  }

  /** 
   * Solve L y = b, where L is the lower triangular matrix in the stored Cholesky decomposi- tion. b[0..n-1] is input as the right-hand side vector. The solution vector is returned in y[0..n-1].
   * 
   * @param b input as the right-hand side vector.
   * @param y the solution.
   */
  void elsolve(VecDoub_I &b, VecDoub_O &y) {

    Int i,j;
    Doub sum;
    if (b.size() != n || y.size() != n) throw("bad lengths"); 
    for (i=0;i<n;i++) {
      for (sum=b[i],j=0; j<i; j++) sum -= el[i][j]*y[j];
      y[i] = sum/el[i][i];
    }
}
  
  /** 
   * Set ainv[0..n-1][0..n-1] to the matrix inverse of A, the matrix whose Cholesky decom- position has been stored.
   * 
   * @param ainv inverse of A.
   */
  void inverse(MatDoub_O &ainv) {
  
  Int i,j,k;
  Doub sum;
  ainv.resize(n,n);
  for (i=0;i<n;i++) 
    for (j=0;j<=i;j++){
      sum = (i==j? 1. : 0.);
      for (k=i-1;k>=j;k--) sum -= el[i][k]*ainv[j][k]; 
      ainv[j][i]= sum/el[i][i];
    }
  for (i=n-1;i>=0;i--) for (j=0;j<=i;j++){
      sum = (i<j? 0. : ainv[j][i]);
      for (k=i+1;k<n;k++) sum -= el[k][i]*ainv[j][k]; 
      ainv[i][j] = ainv[j][i] = sum/el[i][i];
    } 
  }

  /** 
   * Return the logarithm of the determinant of A, the matrix whose Cholesky decomposition has been stored.
   * 
   * 
   * @return logarithm of the determinant of A.
   */
  Doub logdet() {
    Doub sum = 0.;
    for (Int i=0; i<n; i++) sum += log(el[i][i]); return 2.*sum;
  } 
};

#endif
