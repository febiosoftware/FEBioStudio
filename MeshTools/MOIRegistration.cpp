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

#include "stdafx.h"
#include "MOIRegistration.h"
#include <GeomLib/GObject.h>
#include <MeshLib/FSMesh.h>
#include <FECore/matrix.h>
#include <FEBioStudio/MeasureTools.h>
#include <MeshTools/FENNQuery.h>
#include <numeric>
using namespace std;

GMOIRegistration::GMOIRegistration()
{
	m_err = 0.0;
    m_useArea = true;
}

Transform GMOIRegistration::Register(GObject* ptrg, GObject* psrc)
{
	FSMesh& trgMesh = *ptrg->GetFEMesh();
	FSMesh& srcMesh = *psrc->GetFEMesh();

	int NX = trgMesh.Nodes();
	int NP = srcMesh.Nodes();

    mat3d trgMOI = (m_useArea ? CalculateAreaMOI(trgMesh) : CalculateMOI(trgMesh));
    mat3d srcMOI = (m_useArea ? CalculateAreaMOI(srcMesh) : CalculateMOI(srcMesh));

    double eval[3];
    vec3d evec[3];
    const double eps = 1e-12;

    // target
    mat3ds mois = trgMOI.sym();
    mois.eigen2(eval, evec);
    // sort eigenvalues in ascending order
    vector<int> indices(3);
    iota(indices.begin(), indices.end(), 0);
    vector<double> ev(eval, eval + 3);
    sort(indices.begin(), indices.end(), [&](int A, int B)->bool {return ev[A] < ev[B]; });
    // check handedness of eigenvectors and swap if needed
    if (((evec[indices[0]] ^ evec[indices[1]]) * evec[indices[2]]) < 0)
        evec[indices[2]] = -evec[indices[2]];
    mat3d fevec = mat3d(evec[indices[0]].x, evec[indices[0]].y, evec[indices[0]].z,
                        evec[indices[1]].x, evec[indices[1]].y, evec[indices[1]].z,
                        evec[indices[2]].x, evec[indices[2]].y, evec[indices[2]].z);
    vec3d feval = vec3d(eval[indices[0]], eval[indices[1]], eval[indices[2]]);
    // roundoff small numbers
    fevec = CleanUp(fevec, eps);
    
    // create a quaternion from the matrix of eigenvectors
    quatd trgq(fevec);
    trgq.MakeUnit();
    
    // source
    mois = srcMOI.sym();
    mois.eigen2(eval, evec);
    // sort eigenvalues in ascending order
    indices.assign(3,0);
    iota(indices.begin(), indices.end(), 0);
    ev = vector<double>(eval, eval + 3);
    sort(indices.begin(), indices.end(), [&](int A, int B)->bool {return ev[A] < ev[B]; });
    // check handedness of eigenvectors and swap if needed
    if (((evec[indices[0]] ^ evec[indices[1]]) * evec[indices[2]]) < 0)
        evec[indices[2]] = -evec[indices[2]];
    fevec = mat3d(evec[indices[0]].x, evec[indices[0]].y, evec[indices[0]].z,
                        evec[indices[1]].x, evec[indices[1]].y, evec[indices[1]].z,
                        evec[indices[2]].x, evec[indices[2]].y, evec[indices[2]].z);
    feval = vec3d(eval[indices[0]], eval[indices[1]], eval[indices[2]]);
    // roundoff small numbers
    fevec = CleanUp(fevec, eps);
    
    // create a quaternion from the matrix of eigenvectors
    quatd srcq(fevec);
    srcq.MakeUnit();
    
	Transform Q;
    
    quatd q = trgq.Conjugate()*srcq;

    vec3d trgCOM = (m_useArea ? CalculateAreaCOM(trgMesh) : CalculateCOM(trgMesh));
    vec3d srcCOM = (m_useArea ? CalculateAreaCOM(srcMesh) : CalculateCOM(srcMesh));
    vec3d COM = trgCOM - srcCOM;
    
    Q.Rotate(q, srcCOM);

    Q.Translate(COM);
    
    

	return Q;
}

void GMOIRegistration::ApplyTransform(const vector<vec3d>& P0, const Transform& Q, vector<vec3d>& P)
{
	const vec3d& t = Q.GetPosition();
	const quatd& q = Q.GetRotation();

	int N = (int) P0.size();
	P.resize(N);
	for (int i=0; i<N; ++i)
	{
		vec3d p = P0[i];
		P[i] = q*p + t;
	}
}
