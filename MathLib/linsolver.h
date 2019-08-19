#pragma once

void colsol_factor(int N, double* values, int* pointers);
void colsol_solve(int N, double* values, int* pointers, double* R);

void ludcmp(double**a, int n, int* indx);
void lubksb(double**a, int n, int *indx, double b[]);
