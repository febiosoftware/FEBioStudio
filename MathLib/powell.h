#ifndef _POWELL_H_041820060_
#define _POWELL_H_041820060_

// routine for bracketing a minimum
void mnbrak(double* ax, double* bx, double* cx, double* fa, double* fb, double* fc, double (*fnc)(double));

// routine to find 1D minimum
double brent(double ax, double bx, double cx, double (*f)(double), double tol, double* xmin);

#endif // _POWELL_H_041820060_
