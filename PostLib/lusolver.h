// lusolver.h
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LUSOLVER_H__4E540300_07D8_4732_BB8D_6570BB162180__INCLUDED_)
#define AFX_LUSOLVER_H__4E540300_07D8_4732_BB8D_6570BB162180__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

void ludcmp(double**a, int n, int* indx);
void lubksb(double**a, int n, int *indx, double b[]);


#endif // if !defined(AFX_LUSOLVER_H__4E540300_07D8_4732_BB8D_6570BB162180__INCLUDED_)
