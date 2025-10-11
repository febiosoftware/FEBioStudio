/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "FSCoreMesh.h"
#include "hex.h"
#include "tet.h"
#include "penta.h"
#include "pyra.h"
#include "tri3.h"
#include "quad4.h"
using namespace std;

//-----------------------------------------------------------------------------
//! constructor
FSCoreMesh::FSCoreMesh()
{
}

//-----------------------------------------------------------------------------
//! destructor
FSCoreMesh::~FSCoreMesh()
{
}

//-----------------------------------------------------------------------------
//! This function checks if all elements are of the type specified in the argument
bool FSCoreMesh::IsType(int ntype) const
{
	int NE = Elements();
	for (int i=0; i<NE; ++i)
	{
		const FSElement_& el = ElementRef(i);
		if (el.Type() != ntype) return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
//! get the mesh type (returns -1 for mixed meshes)
int FSCoreMesh::GetMeshType() const
{
	int NE = Elements();
	if (NE <= 0) return -1;
	int ntype = ElementRef(0).Type();
	for (int i = 1; i < NE; ++i)
	{
		const FSElement_& el = ElementRef(i);
		if (el.Type() != ntype) return -1;
	}
	return ntype;
}

//-----------------------------------------------------------------------------
vec3d FSCoreMesh::ElementCenter(const FSElement_& el) const
{
	vec3d r;
	if (el.Type() == FE_BEAM3)
	{
		r = m_Node[el.m_node[2]].r;
	}
	else
	{
		int N = el.Nodes();
		for (int i = 0; i < N; i++) r += m_Node[el.m_node[i]].r;
		r /= (float)N;
	}
	return r;
}

//-----------------------------------------------------------------------------
// Count nr of beam elements
int FSCoreMesh::BeamElements()
{
	int n = 0;
	for (int i = 0; i<Elements(); ++i)
	{
		if (ElementRef(i).IsBeam()) n++;
	}

	return n;
}

//-----------------------------------------------------------------------------
// Count nr of shell elements
int FSCoreMesh::ShellElements()
{
	int n = 0;
	for (int i = 0; i<Elements(); ++i)
	{
		if (ElementRef(i).IsShell()) n++;
	}

	return n;
}

//-----------------------------------------------------------------------------
// Count nr of solid elements
int FSCoreMesh::SolidElements()
{
	int n = 0;
	for (int i = 0; i<Elements(); ++i)
	{
		if (ElementRef(i).IsSolid()) n++;
	}

	return n;
}

//-----------------------------------------------------------------------------
//! Is an element exterior or not
bool FSCoreMesh::IsExterior(FSElement_* pe) const
{
	// make sure the element is visible
	if (pe->IsVisible() == false) return false;

	// get number of faces
	int NF = pe->Faces();

	// a shell has 0 faces and is always exterior
	if (NF == 0) return true;

	// solid elements
	for (int i = 0; i<NF; ++i)
	{
		const FSElement_* ei = ElementPtr(pe->m_nbr[i]);
		if ((ei == 0) || (ei->IsVisible() == false)) return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Calculate the volume of an element
double FSCoreMesh::ElementVolume(int iel)
{
	FSElement_& el = ElementRef(iel);
	return ElementVolume(el);
}

double FSCoreMesh::ElementVolume(const FSElement_& el)
{
    if (el.IsSolid()) {
        switch (el.Type())
        {
            case FE_HEX8   : return HexVolume(el); break;
            case FE_HEX20  : return HexVolume(el); break;
            case FE_HEX27  : return HexVolume(el); break;
            case FE_TET4   : return TetVolume(el); break;
            case FE_TET10  : return TetVolume(el); break;
            case FE_TET15  : return TetVolume(el); break;
            case FE_TET20  : return TetVolume(el); break;
            case FE_PENTA6 : return PentaVolume(el); break;
            case FE_PENTA15: return PentaVolume(el); break;
            case FE_PYRA5  : return PyramidVolume(el); break;
            case FE_PYRA13 : return PyramidVolume(el); break;
            case FE_QUAD4  : return QuadVolume(el); break;
        }
    }
    else if (el.IsShell()) {
        FSFace f;
        el.GetShellFace(f);
        double havg = 0;
        for (int i=0; i<f.Nodes(); ++i)
            havg += el.m_h[i];
        havg /= f.Nodes();
        if (havg == 0) havg = 1;
        return FaceArea(f)*havg;
    }

	return 0.0;
}

//-----------------------------------------------------------------------------
double hex8_volume(vec3d* r, bool bJ)
{
	const int NELN = 8;
	const int NINT = 8;
    
    static double gr[NINT] = { 0 };
    static double gs[NINT] = { 0 };
    static double gt[NINT] = { 0 };
    static double gw[NINT] = { 0 };

	static double H[NINT][NELN] = { 0 };
	static double Gr[NINT][NELN] = { 0 };
	static double Gs[NINT][NELN] = { 0 };
	static double Gt[NINT][NELN] = { 0 };
	static bool bfirst = true;

	if (bfirst)
	{
        HEX8::gauss_data(gr, gs, gt, gw);
		for (int n = 0; n<NINT; ++n)
		{
			// calculate shape function values at gauss points
			HEX8::shape(H[n], gr[n], gs[n], gt[n]);

			// calculate local derivatives of shape functions at gauss points
			HEX8::shape_deriv(Gr[n], Gs[n], Gt[n], gr[n], gs[n], gt[n]);
		}

		bfirst = false;
	}

	double J[3][3];
	double vol = 0;
	for (int n = 0; n<NINT; ++n)
	{
		double* Grn = Gr[n];
		double* Gsn = Gs[n];
		double* Gtn = Gt[n];

		J[0][0] = J[0][1] = J[0][2] = 0.0;
		J[1][0] = J[1][1] = J[1][2] = 0.0;
		J[2][0] = J[2][1] = J[2][2] = 0.0;
		for (int i = 0; i<NELN; ++i)
		{
			double Gri = Grn[i];
			double Gsi = Gsn[i];
			double Gti = Gtn[i];

			double x = r[i].x;
			double y = r[i].y;
			double z = r[i].z;

			J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
			J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
			J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
		}

		// calculate the determinant
		double detJ = J[0][0] * (J[1][1] * J[2][2] - J[1][2] * J[2][1])
			+ J[0][1] * (J[1][2] * J[2][0] - J[2][2] * J[1][0])
			+ J[0][2] * (J[1][0] * J[2][1] - J[1][1] * J[2][0]);

        // evaluate volume or (if bJ = true) minimum Jacobian
        if (bJ) {
            if ((detJ < vol) || (n == 0)) vol = detJ;
        }
        else vol += detJ*gw[n];
	}

	return vol;
}

//-----------------------------------------------------------------------------
double hex20_volume(vec3d* r, bool bJ)
{
    const int NELN = 20;
    const int NINT = 8;

    static double gr[NINT] = { 0 };
    static double gs[NINT] = { 0 };
    static double gt[NINT] = { 0 };
    static double gw[NINT] = { 0 };
    
    static double H[NINT][NELN] = { 0 };
    static double Gr[NINT][NELN] = { 0 };
    static double Gs[NINT][NELN] = { 0 };
    static double Gt[NINT][NELN] = { 0 };
    static bool bfirst = true;
    
    if (bfirst)
    {
        HEX20::gauss_data(gr, gs, gt, gw);
        for (int n = 0; n<NINT; ++n)
        {
            // calculate shape function values at gauss points
            HEX20::shape(H[n], gr[n], gs[n], gt[n]);
            
            // calculate local derivatives of shape functions at gauss points
            HEX20::shape_deriv(Gr[n], Gs[n], Gt[n], gr[n], gs[n], gt[n]);
        }
        
        bfirst = false;
    }
    
    double J[3][3];
    double vol = 0;
    for (int n = 0; n<NINT; ++n)
    {
        double* Grn = Gr[n];
        double* Gsn = Gs[n];
        double* Gtn = Gt[n];
        
        J[0][0] = J[0][1] = J[0][2] = 0.0;
        J[1][0] = J[1][1] = J[1][2] = 0.0;
        J[2][0] = J[2][1] = J[2][2] = 0.0;
        for (int i = 0; i<NELN; ++i)
        {
            double Gri = Grn[i];
            double Gsi = Gsn[i];
            double Gti = Gtn[i];
            
            double x = r[i].x;
            double y = r[i].y;
            double z = r[i].z;
            
            J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
            J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
            J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
        }
        
        // calculate the determinant
        double detJ = J[0][0] * (J[1][1] * J[2][2] - J[1][2] * J[2][1])
        + J[0][1] * (J[1][2] * J[2][0] - J[2][2] * J[1][0])
        + J[0][2] * (J[1][0] * J[2][1] - J[1][1] * J[2][0]);
        
        // evaluate volume or (if bJ = true) minimum Jacobian
        if (bJ) {
            if ((detJ < vol) || (n == 0)) vol = detJ;
        }
        else vol += detJ*gw[n];
    }
    
    return vol;
}

//-----------------------------------------------------------------------------
double hex27_volume(vec3d* r, bool bJ)
{
    // gauss-point data
    const int NELN = 27;
    const int NINT = 27;
    
    static double gr[NINT] = { 0 };
    static double gs[NINT] = { 0 };
    static double gt[NINT] = { 0 };
    static double gw[NINT] = { 0 };

    static double H[NINT][NELN] = { 0 };
    static double Gr[NINT][NELN] = { 0 };
    static double Gs[NINT][NELN] = { 0 };
    static double Gt[NINT][NELN] = { 0 };
    static bool bfirst = true;
    
    if (bfirst)
    {
        HEX27::gauss_data(gr, gs, gt, gw);
        for (int n = 0; n<NINT; ++n)
        {
            // calculate shape function values at gauss points
            HEX27::shape(H[n], gr[n], gs[n], gt[n]);
            
            // calculate local derivatives of shape functions at gauss points
            HEX27::shape_deriv(Gr[n], Gs[n], Gt[n], gr[n], gs[n], gt[n]);
        }
        
        bfirst = false;
    }
    
    double J[3][3];
    double vol = 0;
    for (int n = 0; n<NINT; ++n)
    {
        double* Grn = Gr[n];
        double* Gsn = Gs[n];
        double* Gtn = Gt[n];
        
        J[0][0] = J[0][1] = J[0][2] = 0.0;
        J[1][0] = J[1][1] = J[1][2] = 0.0;
        J[2][0] = J[2][1] = J[2][2] = 0.0;
        for (int i = 0; i<NELN; ++i)
        {
            double Gri = Grn[i];
            double Gsi = Gsn[i];
            double Gti = Gtn[i];
            
            double x = r[i].x;
            double y = r[i].y;
            double z = r[i].z;
            
            J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
            J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
            J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
        }
        
        // calculate the determinant
        double detJ = J[0][0] * (J[1][1] * J[2][2] - J[1][2] * J[2][1])
        + J[0][1] * (J[1][2] * J[2][0] - J[2][2] * J[1][0])
        + J[0][2] * (J[1][0] * J[2][1] - J[1][1] * J[2][0]);
        
        // evaluate volume or (if bJ = true) minimum Jacobian
        if (bJ) {
            if ((detJ < vol) || (n == 0)) vol = detJ;
        }
        else vol += detJ*gw[n];
    }
    
    return vol;
}

//-----------------------------------------------------------------------------
// Calculate the volume of a hex element
double FSCoreMesh::HexVolume(const FSElement_& el)
{
	assert((el.Type() == FE_HEX8) || (el.Type() == FE_HEX20) || (el.Type() == FE_HEX27));

    
    vec3d rt[FSElement::MAX_NODES];
    for (int i = 0; i<el.Nodes(); ++i) rt[i] = NodePosition(el.m_node[i]);
    
    switch (el.Type())
    {
        case FE_HEX8:
            return hex8_volume(rt);
            break;
        case FE_HEX20:
            return hex20_volume(rt);
            break;
        case FE_HEX27:
            return hex27_volume(rt);
            break;
    }
    return 0.0;
}


//-----------------------------------------------------------------------------
double penta6_volume(vec3d* r, bool bJ)
{
    const int NELN = 6;
    const int NINT = 6;
    
    static double gr[NINT] = { 0 };
    static double gs[NINT] = { 0 };
    static double gt[NINT] = { 0 };
    static double gw[NINT] = { 0 };

    static double H[NINT][NELN] = { 0 };
    static double Gr[NINT][NELN] = { 0 };
    static double Gs[NINT][NELN] = { 0 };
    static double Gt[NINT][NELN] = { 0 };
    static bool bfirst = true;
    
    if (bfirst)
    {
        PENTA6::gauss_data(gr, gs, gt, gw);
        for (int n = 0; n<NINT; ++n)
        {
            // calculate shape function values at gauss points
            PENTA6::shape(H[n], gr[n], gs[n], gt[n]);
            
            // calculate local derivatives of shape functions at gauss points
            PENTA6::shape_deriv(Gr[n], Gs[n], Gt[n], gr[n], gs[n], gt[n]);
        }
        
        bfirst = false;
    }
    
    double J[3][3];
    double vol = 0;
    for (int n = 0; n<NINT; ++n)
    {
        double* Grn = Gr[n];
        double* Gsn = Gs[n];
        double* Gtn = Gt[n];
        
        J[0][0] = J[0][1] = J[0][2] = 0.0;
        J[1][0] = J[1][1] = J[1][2] = 0.0;
        J[2][0] = J[2][1] = J[2][2] = 0.0;
        for (int i = 0; i<NELN; ++i)
        {
            double Gri = Grn[i];
            double Gsi = Gsn[i];
            double Gti = Gtn[i];
            
            double x = r[i].x;
            double y = r[i].y;
            double z = r[i].z;
            
            J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
            J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
            J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
        }
        
        // calculate the determinant
        double detJ = J[0][0] * (J[1][1] * J[2][2] - J[1][2] * J[2][1])
        + J[0][1] * (J[1][2] * J[2][0] - J[2][2] * J[1][0])
        + J[0][2] * (J[1][0] * J[2][1] - J[1][1] * J[2][0]);
        
        // evaluate volume or (if bJ = true) minimum Jacobian
        if (bJ) {
            if ((detJ < vol) || (n == 0)) vol = detJ;
        }
        else vol += detJ*gw[n];
    }
    
    return vol;
}

//-----------------------------------------------------------------------------
double penta15_volume(vec3d* r, bool bJ)
{
    const int NELN = 15;
    const int NINT = 8;
    
    static double gr[NINT] = { 0 };
    static double gs[NINT] = { 0 };
    static double gt[NINT] = { 0 };
    static double gw[NINT] = { 0 };

    static double H[NINT][NELN] = { 0 };
    static double Gr[NINT][NELN] = { 0 };
    static double Gs[NINT][NELN] = { 0 };
    static double Gt[NINT][NELN] = { 0 };
    static bool bfirst = true;
    
    if (bfirst)
    {
        PENTA15::gauss_data(gr, gs, gt, gw);
        for (int n = 0; n<NINT; ++n)
        {
            // calculate shape function values at gauss points
            PENTA15::shape(H[n], gr[n], gs[n], gt[n]);
            
            // calculate local derivatives of shape functions at gauss points
            PENTA15::shape_deriv(Gr[n], Gs[n], Gt[n], gr[n], gs[n], gt[n]);
        }
        
        bfirst = false;
    }
    
    double J[3][3];
    double vol = 0;
    for (int n = 0; n<NINT; ++n)
    {
        double* Grn = Gr[n];
        double* Gsn = Gs[n];
        double* Gtn = Gt[n];
        
        J[0][0] = J[0][1] = J[0][2] = 0.0;
        J[1][0] = J[1][1] = J[1][2] = 0.0;
        J[2][0] = J[2][1] = J[2][2] = 0.0;
        for (int i = 0; i<NELN; ++i)
        {
            double Gri = Grn[i];
            double Gsi = Gsn[i];
            double Gti = Gtn[i];
            
            double x = r[i].x;
            double y = r[i].y;
            double z = r[i].z;
            
            J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
            J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
            J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
        }
        
        // calculate the determinant
        double detJ = J[0][0] * (J[1][1] * J[2][2] - J[1][2] * J[2][1])
        + J[0][1] * (J[1][2] * J[2][0] - J[2][2] * J[1][0])
        + J[0][2] * (J[1][0] * J[2][1] - J[1][1] * J[2][0]);
        
        // evaluate volume or (if bJ = true) minimum Jacobian
        if (bJ) {
            if ((detJ < vol) || (n == 0)) vol = detJ;
        }
        else vol += detJ*gw[n];
    }
    
    return vol;
}

//-----------------------------------------------------------------------------
// Calculate the volume of a pentahedral element
double FSCoreMesh::PentaVolume(const FSElement_& el)
{
	assert((el.Type() == FE_PENTA6) || (el.Type() == FE_PENTA15));

    vec3d rt[FSElement::MAX_NODES];
    for (int i = 0; i<el.Nodes(); ++i) rt[i] = NodePosition(el.m_node[i]);
    
    switch (el.Type())
    {
        case FE_PENTA6:
            return penta6_volume(rt);
            break;
        case FE_PENTA15:
            return penta15_volume(rt);
            break;
    }
    return 0.0;
}
//-----------------------------------------------------------------------------
double pyra5_volume(vec3d* r, bool bJ)
{
    const int NELN = 5;
    const int NINT = 8;
    
    static double gr[NINT] = {0};
    static double gs[NINT] = {0};
    static double gt[NINT] = {0};
    static double gw[NINT] = {0};
    
    static double H[NINT][NELN] = { 0 };
    static double Gr[NINT][NELN] = { 0 };
    static double Gs[NINT][NELN] = { 0 };
    static double Gt[NINT][NELN] = { 0 };
    static bool bfirst = true;
    
    if (bfirst)
    {
        PYRA5::gauss_data(gr, gs, gt, gw);
        for (int n = 0; n<NINT; ++n)
        {
            // calculate shape function values at gauss points
            PYRA5::shape(H[n], gr[n], gs[n], gt[n]);
            
            // calculate local derivatives of shape functions at gauss points
            PYRA5::shape_deriv(Gr[n], Gs[n], Gt[n], gr[n], gs[n], gt[n]);
        }
        
        bfirst = false;
    }
    
    double J[3][3];
    double vol = 0;
    for (int n = 0; n<NINT; ++n)
    {
        double* Grn = Gr[n];
        double* Gsn = Gs[n];
        double* Gtn = Gt[n];
        
        J[0][0] = J[0][1] = J[0][2] = 0.0;
        J[1][0] = J[1][1] = J[1][2] = 0.0;
        J[2][0] = J[2][1] = J[2][2] = 0.0;
        for (int i = 0; i<NELN; ++i)
        {
            double Gri = Grn[i];
            double Gsi = Gsn[i];
            double Gti = Gtn[i];
            
            double x = r[i].x;
            double y = r[i].y;
            double z = r[i].z;
            
            J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
            J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
            J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
        }
        
        // calculate the determinant
        double detJ = J[0][0] * (J[1][1] * J[2][2] - J[1][2] * J[2][1])
        + J[0][1] * (J[1][2] * J[2][0] - J[2][2] * J[1][0])
        + J[0][2] * (J[1][0] * J[2][1] - J[1][1] * J[2][0]);
        
        // evaluate volume or (if bJ = true) minimum Jacobian
        if (bJ) {
            if ((detJ < vol) || (n == 0)) vol = detJ;
        }
        else vol += detJ*gw[n];
    }
    
    return vol;
}

//-----------------------------------------------------------------------------
double pyra13_volume(vec3d* r, bool bJ)
{
    const int NELN = 13;
    const int NINT = 8;
    
    static double gr[NINT] = {0};
    static double gs[NINT] = {0};
    static double gt[NINT] = {0};
    static double gw[NINT] = {0};

    static double H[NINT][NELN] = { 0 };
    static double Gr[NINT][NELN] = { 0 };
    static double Gs[NINT][NELN] = { 0 };
    static double Gt[NINT][NELN] = { 0 };
    static bool bfirst = true;
    
    if (bfirst)
    {
        PYRA13::gauss_data(gr, gs, gt, gw);
        for (int n = 0; n<NINT; ++n)
        {
            // calculate shape function values at gauss points
            PYRA13::shape(H[n], gr[n], gs[n], gt[n]);
            
            // calculate local derivatives of shape functions at gauss points
            PYRA13::shape_deriv(Gr[n], Gs[n], Gt[n], gr[n], gs[n], gt[n]);
        }
        
        bfirst = false;
    }
    
    double J[3][3];
    double vol = 0;
    for (int n = 0; n<NINT; ++n)
    {
        double* Grn = Gr[n];
        double* Gsn = Gs[n];
        double* Gtn = Gt[n];
        
        J[0][0] = J[0][1] = J[0][2] = 0.0;
        J[1][0] = J[1][1] = J[1][2] = 0.0;
        J[2][0] = J[2][1] = J[2][2] = 0.0;
        for (int i = 0; i<NELN; ++i)
        {
            double Gri = Grn[i];
            double Gsi = Gsn[i];
            double Gti = Gtn[i];
            
            double x = r[i].x;
            double y = r[i].y;
            double z = r[i].z;
            
            J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
            J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
            J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
        }
        
        // calculate the determinant
        double detJ = J[0][0] * (J[1][1] * J[2][2] - J[1][2] * J[2][1])
        + J[0][1] * (J[1][2] * J[2][0] - J[2][2] * J[1][0])
        + J[0][2] * (J[1][0] * J[2][1] - J[1][1] * J[2][0]);
        
        // evaluate volume or (if bJ = true) minimum Jacobian
        if (bJ) {
            if ((detJ < vol) || (n == 0)) vol = detJ;
        }
        else vol += detJ*gw[n];
    }
    
    return vol;
}

//-----------------------------------------------------------------------------
// Calculate the volume of a pyramid element
double FSCoreMesh::PyramidVolume(const FSElement_& el)
{
	assert((el.Type() == FE_PYRA5) || (el.Type() == FE_PYRA13));
    
    vec3d rt[FSElement::MAX_NODES];
    for (int i = 0; i<el.Nodes(); ++i) rt[i] = NodePosition(el.m_node[i]);
    
    switch (el.Type())
    {
        case FE_PYRA5:
            return pyra5_volume(rt);
            break;
        case FE_PYRA13:
            return pyra13_volume(rt);
            break;
    }
    return 0.0;
}

//-----------------------------------------------------------------------------
double tri3_volume(vec3d* r, vec3d* D, bool bJ)
{
    const int NELN = 3;
    const int NINT = 6;

    static double gr[NINT] = { 0 };
    static double gs[NINT] = { 0 };
    static double gt[NINT] = { 0 };
    static double gw[NINT] = { 0 };

    static double H[NINT][NELN] = { 0 };
    static double Gr[NINT][NELN] = { 0 };
    static double Gs[NINT][NELN] = { 0 };
    static bool bfirst = true;

    if (bfirst)
    {
        TRI3::gauss_data(gr, gs, gt, gw);
        for (int n = 0; n < NINT; ++n)
        {
            // calculate shape function values at gauss points
            TRI3::shape(H[n], gr[n], gs[n]);

            // calculate local derivatives of shape functions at gauss points
            TRI3::shape_deriv(Gr[n], Gs[n], gr[n], gs[n]);
        }

        bfirst = false;
    }

    double V = 0;
    vec3d g[3];
    for (int n = 0; n < NINT; ++n)
    {
        // jacobian matrix
        double eta = gt[n];

        double* Mr = Gr[n];
        double* Ms = Gs[n];
        double* M = H[n];

        // evaluate covariant basis vectors
        g[0] = g[1] = g[2] = vec3d(0, 0, 0);
        for (int i = 0; i < NELN; ++i)
        {
            g[0] += (r[i] + D[i] * eta / 2) * Mr[i];
            g[1] += (r[i] + D[i] * eta / 2) * Ms[i];
            g[2] += D[i] * (M[i] / 2);
        }

        mat3d J = mat3d(g[0].x, g[1].x, g[2].x,
            g[0].y, g[1].y, g[2].y,
            g[0].z, g[1].z, g[2].z);

        // calculate the determinant
        double detJ = J.det();

        // evaluate volume or (if bJ = true) minimum Jacobian
        if (bJ) {
            if ((detJ < V) || (n == 0)) V = detJ;
        }
        else V += detJ * gw[n];
    }

    return V;
}

//-----------------------------------------------------------------------------
double quad4_volume(vec3d* r, vec3d* D, bool bJ)
{
    const int NELN = 4;
    const int NINT = 8;

    static double gr[NINT] = { 0 };
    static double gs[NINT] = { 0 };
    static double gt[NINT] = { 0 };
    static double gw[NINT] = { 0 };

    static double H[NINT][NELN] = { 0 };
    static double Gr[NINT][NELN] = { 0 };
    static double Gs[NINT][NELN] = { 0 };
    static bool bfirst = true;

    if (bfirst)
    {
        QUAD4::gauss_data(gr, gs, gt, gw);
        for (int n = 0; n < NINT; ++n)
        {
            // calculate shape function values at gauss points
            QUAD4::shape(H[n], gr[n], gs[n]);

            // calculate local derivatives of shape functions at gauss points
            QUAD4::shape_deriv(Gr[n], Gs[n], gr[n], gs[n]);
        }

        bfirst = false;
    }

    double V = 0;
    vec3d g[3];
    for (int n = 0; n < NINT; ++n)
    {
        // jacobian matrix
        double eta = gt[n];

        double* Mr = Gr[n];
        double* Ms = Gs[n];
        double* M = H[n];

        // evaluate covariant basis vectors
        g[0] = g[1] = g[2] = vec3d(0, 0, 0);
        for (int i = 0; i < NELN; ++i)
        {
            g[0] += (r[i] + D[i] * eta / 2) * Mr[i];
            g[1] += (r[i] + D[i] * eta / 2) * Ms[i];
            g[2] += D[i] * (M[i] / 2);
        }

        mat3d J = mat3d(g[0].x, g[1].x, g[2].x,
            g[0].y, g[1].y, g[2].y,
            g[0].z, g[1].z, g[2].z);

        // calculate the determinant
        double detJ = J.det();

        // evaluate volume or (if bJ = true) minimum Jacobian
        if (bJ) {
            if ((detJ < V) || (n == 0)) V = detJ;
        }
        else V += detJ * gw[n];
    }

    return V;
}

//-----------------------------------------------------------------------------
// Calculate the volume of a pyramid element
double FSCoreMesh::QuadVolume(const FSElement_& el)
{
    assert(el.Type() == FE_QUAD4);

    FSFace& face = Face(el.m_face[0]);

    vec3d rt[FSElement::MAX_NODES];
    vec3d Dt[FSElement::MAX_NODES];
    for (int i = 0; i < el.Nodes(); ++i)
    {
        rt[i] = m_Node[el.m_node[i]].r;
        Dt[i] = to_vec3d(face.m_nn[i]*el.m_h[i]);
    }

    switch (el.Type())
    {
    case FE_TRI3 : return tri3_volume(rt, Dt); break;
    case FE_QUAD4: return quad4_volume(rt, Dt); break;
    }
    return 0.0;
}

//-----------------------------------------------------------------------------
double tet4_volume(vec3d* r, bool bJ)
{
	const int NELN = 4;
	const int NINT = 4;

    static double gr[NINT] = {0};
    static double gs[NINT] = {0};
    static double gt[NINT] = {0};
    static double gw[NINT] = {0};

	static double H[NINT][NELN] = { 0 };
	static double Gr[NINT][NELN] = { 0 };
	static double Gs[NINT][NELN] = { 0 };
	static double Gt[NINT][NELN] = { 0 };
	static bool bfirst = true;

	if (bfirst)
	{
        TET4::gauss_data(gr, gs, gt, gw);
		for (int n = 0; n<NINT; ++n)
		{
			// calculate shape function values at gauss points
			TET4::shape(H[n], gr[n], gs[n], gt[n]);

			// calculate local derivatives of shape functions at gauss points
			TET4::shape_deriv(Gr[n], Gs[n], Gt[n], gr[n], gs[n], gt[n]);
		}

		bfirst = false;
	}

	double J[3][3];
	double vol = 0;
	for (int n = 0; n<NINT; ++n)
	{
		double* Grn = Gr[n];
		double* Gsn = Gs[n];
		double* Gtn = Gt[n];

		J[0][0] = J[0][1] = J[0][2] = 0.0;
		J[1][0] = J[1][1] = J[1][2] = 0.0;
		J[2][0] = J[2][1] = J[2][2] = 0.0;
		for (int i = 0; i<NELN; ++i)
		{
			double Gri = Grn[i];
			double Gsi = Gsn[i];
			double Gti = Gtn[i];

			double x = r[i].x;
			double y = r[i].y;
			double z = r[i].z;

			J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
			J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
			J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
		}

		// calculate the determinant
		double detJ = J[0][0] * (J[1][1] * J[2][2] - J[1][2] * J[2][1])
			+ J[0][1] * (J[1][2] * J[2][0] - J[2][2] * J[1][0])
			+ J[0][2] * (J[1][0] * J[2][1] - J[1][1] * J[2][0]);

        // evaluate volume or (if bJ = true) minimum Jacobian
        if (bJ) {
            if ((detJ < vol) || (n == 0)) vol = detJ;
        }
        else vol += detJ*gw[n];
	}

	return vol;
}

//-----------------------------------------------------------------------------
double tet5_volume(vec3d* r, bool bJ)
{
    const int NELN = 5;
    const int NINT = 4;
    
    static double gr[NINT] = {0};
    static double gs[NINT] = {0};
    static double gt[NINT] = {0};
    static double gw[NINT] = {0};
    
    static double H[NINT][NELN] = { 0 };
    static double Gr[NINT][NELN] = { 0 };
    static double Gs[NINT][NELN] = { 0 };
    static double Gt[NINT][NELN] = { 0 };
    static bool bfirst = true;
    
    if (bfirst)
    {
        TET5::gauss_data(gr, gs, gt, gw);
        for (int n = 0; n<NINT; ++n)
        {
            // calculate shape function values at gauss points
            TET5::shape(H[n], gr[n], gs[n], gt[n]);
            
            // calculate local derivatives of shape functions at gauss points
            TET5::shape_deriv(Gr[n], Gs[n], Gt[n], gr[n], gs[n], gt[n]);
        }
        
        bfirst = false;
    }
    
    double J[3][3];
    double vol = 0;
    for (int n = 0; n<NINT; ++n)
    {
        double* Grn = Gr[n];
        double* Gsn = Gs[n];
        double* Gtn = Gt[n];
        
        J[0][0] = J[0][1] = J[0][2] = 0.0;
        J[1][0] = J[1][1] = J[1][2] = 0.0;
        J[2][0] = J[2][1] = J[2][2] = 0.0;
        for (int i = 0; i<NELN; ++i)
        {
            double Gri = Grn[i];
            double Gsi = Gsn[i];
            double Gti = Gtn[i];
            
            double x = r[i].x;
            double y = r[i].y;
            double z = r[i].z;
            
            J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
            J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
            J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
        }
        
        // calculate the determinant
        double detJ = J[0][0] * (J[1][1] * J[2][2] - J[1][2] * J[2][1])
        + J[0][1] * (J[1][2] * J[2][0] - J[2][2] * J[1][0])
        + J[0][2] * (J[1][0] * J[2][1] - J[1][1] * J[2][0]);
        
        // evaluate volume or (if bJ = true) minimum Jacobian
        if (bJ) {
            if ((detJ < vol) || (n == 0)) vol = detJ;
        }
        else vol += detJ*gw[n];
    }
    
    return vol;
}

//-----------------------------------------------------------------------------
double tet10_volume(vec3d* r, bool bJ)
{
	const int NELN = 10;
	const int NINT = 8;

	static double gr[NINT] = { 0 };
	static double gs[NINT] = { 0 };
	static double gt[NINT] = { 0 };
	static double gw[NINT] = { 0 };

	static double H[NINT][NELN] = { 0 };
	static double Gr[NINT][NELN] = { 0 };
	static double Gs[NINT][NELN] = { 0 };
	static double Gt[NINT][NELN] = { 0 };
	static bool bfirst = true;

	if (bfirst)
	{
        TET10::gauss_data(gr, gs, gt, gw);
		for (int n = 0; n<NINT; ++n)
		{
			// calculate shape function values at gauss points
			TET10::shape(H[n], gr[n], gs[n], gt[n]);

			// calculate local derivatives of shape functions at gauss points
			TET10::shape_deriv(Gr[n], Gs[n], Gt[n], gr[n], gs[n], gt[n]);
		}

		bfirst = false;
	}

	double J[3][3];
	double vol = 0;
	for (int n = 0; n<NINT; ++n)
	{
		double* Grn = Gr[n];
		double* Gsn = Gs[n];
		double* Gtn = Gt[n];

		J[0][0] = J[0][1] = J[0][2] = 0.0;
		J[1][0] = J[1][1] = J[1][2] = 0.0;
		J[2][0] = J[2][1] = J[2][2] = 0.0;
		for (int i = 0; i<NELN; ++i)
		{
			double Gri = Grn[i];
			double Gsi = Gsn[i];
			double Gti = Gtn[i];

			double x = r[i].x;
			double y = r[i].y;
			double z = r[i].z;

			J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
			J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
			J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
		}

		// calculate the determinant
		double detJ = J[0][0] * (J[1][1] * J[2][2] - J[1][2] * J[2][1])
			+ J[0][1] * (J[1][2] * J[2][0] - J[2][2] * J[1][0])
			+ J[0][2] * (J[1][0] * J[2][1] - J[1][1] * J[2][0]);

        // evaluate volume or (if bJ = true) minimum Jacobian
        if (bJ) {
            if ((detJ < vol) || (n == 0)) vol = detJ;
        }
        else vol += detJ*gw[n];
	}

	return vol;
}

//-----------------------------------------------------------------------------
double tet15_volume(vec3d* r, bool bJ)
{
    const int NELN = 15;
    const int NINT = 8;
    
    static double gr[NINT] = { 0 };
    static double gs[NINT] = { 0 };
    static double gt[NINT] = { 0 };
    static double gw[NINT] = { 0 };
    
    static double H[NINT][NELN] = { 0 };
    static double Gr[NINT][NELN] = { 0 };
    static double Gs[NINT][NELN] = { 0 };
    static double Gt[NINT][NELN] = { 0 };
    static bool bfirst = true;
    
    if (bfirst)
    {
        TET15::gauss_data(gr, gs, gt, gw);
        for (int n = 0; n<NINT; ++n)
        {
            // calculate shape function values at gauss points
            TET15::shape(H[n], gr[n], gs[n], gt[n]);
            
            // calculate local derivatives of shape functions at gauss points
            TET15::shape_deriv(Gr[n], Gs[n], Gt[n], gr[n], gs[n], gt[n]);
        }
        
        bfirst = false;
    }
    
    double J[3][3];
    double vol = 0;
    for (int n = 0; n<NINT; ++n)
    {
        double* Grn = Gr[n];
        double* Gsn = Gs[n];
        double* Gtn = Gt[n];
        
        J[0][0] = J[0][1] = J[0][2] = 0.0;
        J[1][0] = J[1][1] = J[1][2] = 0.0;
        J[2][0] = J[2][1] = J[2][2] = 0.0;
        for (int i = 0; i<NELN; ++i)
        {
            double Gri = Grn[i];
            double Gsi = Gsn[i];
            double Gti = Gtn[i];
            
            double x = r[i].x;
            double y = r[i].y;
            double z = r[i].z;
            
            J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
            J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
            J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
        }
        
        // calculate the determinant
        double detJ = J[0][0] * (J[1][1] * J[2][2] - J[1][2] * J[2][1])
        + J[0][1] * (J[1][2] * J[2][0] - J[2][2] * J[1][0])
        + J[0][2] * (J[1][0] * J[2][1] - J[1][1] * J[2][0]);
        
        // evaluate volume or (if bJ = true) minimum Jacobian
        if (bJ) {
            if ((detJ < vol) || (n == 0)) vol = detJ;
        }
        else vol += detJ*gw[n];
    }
    
    return vol;
}

//-----------------------------------------------------------------------------
double tet20_volume(vec3d* r, bool bJ)
{
    const int NELN = 20;
    const int NINT = 15;
    
    static double gr[NINT] = { 0 };
    static double gs[NINT] = { 0 };
    static double gt[NINT] = { 0 };
    static double gw[NINT] = { 0 };
    
    static double H[NINT][NELN] = { 0 };
    static double Gr[NINT][NELN] = { 0 };
    static double Gs[NINT][NELN] = { 0 };
    static double Gt[NINT][NELN] = { 0 };
    static bool bfirst = true;
    
    if (bfirst)
    {
        TET20::gauss_data(gr, gs, gt, gw);
        for (int n = 0; n<NINT; ++n)
        {
            // calculate shape function values at gauss points
            TET20::shape(H[n], gr[n], gs[n], gt[n]);
            
            // calculate local derivatives of shape functions at gauss points
            TET20::shape_deriv(Gr[n], Gs[n], Gt[n], gr[n], gs[n], gt[n]);
        }
        
        bfirst = false;
    }
    
    double J[3][3];
    double vol = 0;
    for (int n = 0; n<NINT; ++n)
    {
        double* Grn = Gr[n];
        double* Gsn = Gs[n];
        double* Gtn = Gt[n];
        
        J[0][0] = J[0][1] = J[0][2] = 0.0;
        J[1][0] = J[1][1] = J[1][2] = 0.0;
        J[2][0] = J[2][1] = J[2][2] = 0.0;
        for (int i = 0; i<NELN; ++i)
        {
            double Gri = Grn[i];
            double Gsi = Gsn[i];
            double Gti = Gtn[i];
            
            double x = r[i].x;
            double y = r[i].y;
            double z = r[i].z;
            
            J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
            J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
            J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
        }
        
        // calculate the determinant
        double detJ = J[0][0] * (J[1][1] * J[2][2] - J[1][2] * J[2][1])
        + J[0][1] * (J[1][2] * J[2][0] - J[2][2] * J[1][0])
        + J[0][2] * (J[1][0] * J[2][1] - J[1][1] * J[2][0]);
        
        // evaluate volume or (if bJ = true) minimum Jacobian
        if (bJ) {
            if ((detJ < vol) || (n == 0)) vol = detJ;
        }
        else vol += detJ*gw[n];
    }
    
    return vol;
}


//-----------------------------------------------------------------------------
// Calculate the volume of a tetrahedral element
double FSCoreMesh::TetVolume(const FSElement_& el)
{
	assert((el.Type() == FE_TET4) || (el.Type() == FE_TET10)
		|| (el.Type() == FE_TET15) || (el.Type() == FE_TET20));

	vec3d rt[FSElement::MAX_NODES];
	for (int i = 0; i<el.Nodes(); ++i) rt[i] = NodePosition(el.m_node[i]);

	switch (el.Type())
	{
	case FE_TET4:
		return tet4_volume(rt);
		break;
    case FE_TET5:
        return tet5_volume(rt);
        break;
	case FE_TET10:
        return tet10_volume(rt);
        break;
	case FE_TET15:
		return tet15_volume(rt);
		break;
    case FE_TET20:
        return tet20_volume(rt);
        break;
	}
	return 0.0;
}

//-----------------------------------------------------------------------------
// Tag all elements
void FSCoreMesh::TagAllElements(int ntag)
{
	const int NE = Elements();
	for (int i = 0; i<NE; ++i) ElementRef(i).m_ntag = ntag;
}

//-----------------------------------------------------------------------------
// See if this is a shell mesh.
bool FSCoreMesh::IsShell() const
{
	int NE = Elements();
	for (int i = 0; i<NE; ++i)
	{
		const FSElement_& el = ElementRef(i);
		if (el.IsShell() == false) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// See if this is a solid mesh.
bool FSCoreMesh::IsSolid() const
{
	int NE = Elements();
	for (int i = 0; i<NE; ++i)
	{
		const FSElement_& el = ElementRef(i);
		if (el.IsSolid() == false) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Find a face of an element.
int FSCoreMesh::FindFace(FSElement_ *pe, FSFace &f, FSFace& fe)
{
	if (pe->IsSolid())
	{
		int n = pe->Faces();
		for (int i = 0; i<n; ++i)
		{
			fe = pe->GetFace(i);
			if (fe == f) return i;
		}
	}
	if (pe->IsShell())
	{
		pe->GetShellFace(fe);
		if (fe == f) return 0;
	}

	return -1;
}

//-------------------------------------------------------------------------------------------------
void FSCoreMesh::SelectElements(const vector<int>& elem)
{
	for (int i : elem) ElementRef(i).Select();
}

//-----------------------------------------------------------------------------
// This function finds the interior and exterior nodes.
void FSCoreMesh::MarkExteriorNodes()
{
	// assume all nodes interior
	int nodes = Nodes();
	for (int i = 0; i<nodes; ++i) Node(i).SetExterior(false);

	// mark (exterior) face nodes as exterior
	int faces = Faces();
	for (int i = 0; i<faces; ++i)
	{
		FSFace& face = Face(i);
		if (face.IsExterior())
		{
			for (int j = 0; j < face.Nodes(); ++j)
				m_Node[face.n[j]].SetExterior(true);
		}
	}

	// mark all nodes attached to beams as exterior
	int elems = Elements();
	for (int i = 0; i<elems; ++i)
	{
		FSElement_& el = ElementRef(i);
		if (el.IsType(FE_BEAM2) || el.IsType(FE_BEAM3))
		{
			for (int j=0; j<el.Nodes(); ++j) Node(el.m_node[j]).SetExterior(true);
		}
	}
}

//-----------------------------------------------------------------------------
void FSCoreMesh::FindNodesFromPart(int gid, vector<int>& node)
{
	for (int i = 0; i<Nodes(); ++i) Node(i).m_ntag = 0;
	for (int i = 0; i<Elements(); ++i)
	{
		FSElement_& e = ElementRef(i);
		if (e.m_gid == gid)
		{
			int ne = e.Nodes();
			for (int j = 0; j<ne; ++j) Node(e.m_node[j]).m_ntag = 1;
		}
	}

	int nodes = 0;
	for (int i = 0; i<Nodes(); ++i) if (Node(i).m_ntag == 1) nodes++;

	node.resize(nodes);
	nodes = 0;
	for (int i = 0; i<Nodes(); ++i)
		if (Node(i).m_ntag == 1) node[nodes++] = i;
}

//-------------------------------------------------------------------------------------------------
FSNode* FSCoreMesh::FindNodeFromID(int gid)
{
	for (int i = 0; i < Nodes(); ++i)
	{
		FSNode& node = Node(i);
		if (node.m_gid == gid) return &node;
	}
	return nullptr;
}

//-------------------------------------------------------------------------------------------------
void FSCoreMesh::ShowAllElements()
{
	for (int i = 0; i<Nodes(); ++i) Node(i).Show();
	for (int i = 0; i<Edges(); ++i) Edge(i).Show();
	for (int i = 0; i<Faces(); ++i) Face(i).Show();
	for (int i = 0; i<Elements(); ++i) ElementRef(i).Show();
}

//-------------------------------------------------------------------------------------------------
void FSCoreMesh::ShowElements(vector<int>& elem, bool show)
{
	// show or hide all the elements
	if (show)
		for (int i : elem) ElementRef(i).Unhide();
	else
		for (int i : elem) ElementRef(i).Hide();

	// update visibility of all other items
	UpdateItemVisibility();
}

//-------------------------------------------------------------------------------------------------
void FSCoreMesh::UpdateItemVisibility()
{
	// tag all visible nodes
	TagAllNodes(0);
	for (int i = 0; i<Elements(); ++i)
	{
		FSElement_& el = ElementRef(i);
		if (el.IsVisible())
		{
			int ne = el.Nodes();
			for (int j = 0; j<ne; ++j) Node(el.m_node[j]).m_ntag = 1;
		}
	}

	// update face visibility
	for (int i = 0; i<Faces(); ++i)
	{
		FSFace& face = Face(i);

		FSElement_* e0 = ElementPtr(face.m_elem[0].eid); assert(e0);
		FSElement_* e1 = ElementPtr(face.m_elem[1].eid);

		if (!e0->IsVisible() && ((e1 == 0) || !e1->IsVisible())) face.Hide(); else { face.Show(); face.Unhide(); }
	}

	// update visibility of all other items
	for (int i = 0; i<Nodes(); ++i)
	{
		FSNode& node = Node(i);
		if (node.m_ntag == 1) { node.Show(); node.Unhide(); } else node.Hide();
	}

	for (int i = 0; i<Edges(); ++i)
	{
		FSEdge& edge = Edge(i);
		if ((Node(edge.n[0]).m_ntag == 0) || (Node(edge.n[1]).m_ntag == 0)) edge.Hide();
		else { edge.Show(); edge.Unhide(); }
	}
}

//-----------------------------------------------------------------------------
// count node partitions
int FSCoreMesh::CountNodePartitions() const
{
	int max_gid = -1;
	for (int i = 0; i<Nodes(); ++i)
	{
		const FSNode& node = Node(i);
		if (node.m_gid > max_gid) max_gid = node.m_gid;
	}
	return max_gid + 1;
}

//-----------------------------------------------------------------------------
int FSCoreMesh::CountEdgePartitions() const
{
	int max_gid = -1;
	for (int i = 0; i<Edges(); ++i)
	{
		const FSEdge& edge = Edge(i);
		if (edge.m_gid > max_gid) max_gid = edge.m_gid;
	}
	return max_gid + 1;
}

//-----------------------------------------------------------------------------
int FSCoreMesh::CountFacePartitions() const
{
	int max_gid = -1;
	for (int i = 0; i<Faces(); ++i)
	{
		const FSFace& face = Face(i);
		if (face.m_gid > max_gid) max_gid = face.m_gid;
	}
	return max_gid + 1;
}


//-----------------------------------------------------------------------------
int FSCoreMesh::CountElementPartitions() const
{
	int max_gid = -1;
	for (int i = 0; i<Elements(); ++i)
	{
		const FSElement_& elem = ElementRef(i);
		if (elem.m_gid > max_gid) max_gid = elem.m_gid;
	}
	return max_gid + 1;
}

//-----------------------------------------------------------------------------
int FSCoreMesh::CountSmoothingGroups() const
{
	int max_sg = -1;
	for (int i = 0; i<Faces(); ++i)
	{
		const FSFace& face = Face(i);
		if (face.m_sid > max_sg) max_sg = face.m_sid;
	}
	return max_sg + 1;
}

//-----------------------------------------------------------------------------
void ForAllElements(FSCoreMesh& mesh, std::function<void(FSElement_& el)> f)
{
	for (int i = 0; i < mesh.Elements(); ++i)
	{
		FSElement_& el = mesh.ElementRef(i);
		f(el);
	}
}

void ForAllSelectedElements(FSCoreMesh& mesh, std::function<void(FSElement_& el)> f)
{
	for (int i = 0; i < mesh.Elements(); ++i)
	{
		FSElement_& el = mesh.ElementRef(i);
		if (el.IsSelected()) f(el);
	}
}

void ForAllSelectedNodes(FSCoreMesh& mesh, std::function<void(FSNode& node)> f)
{
	int NN = mesh.Nodes();
	for (int i = 0; i < NN; ++i)
	{
		FSNode& node = mesh.Node(i);
		if (node.IsSelected()) f(node);
	}
}

void ForAllTaggedNodes(FSCoreMesh& mesh, int tag, std::function<void(FSNode& node)> f)
{
	int NN = mesh.Nodes();
	for (int i = 0; i < NN; ++i)
	{
		FSNode& node = mesh.Node(i);
		if (node.m_ntag == tag) f(node);
	}
}
