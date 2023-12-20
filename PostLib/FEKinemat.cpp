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
#include "FEKinemat.h"
#include <PostLib/FEMeshData_T.h>
#include <PostLib/FEPostModel.h>
using namespace Post;
using namespace std;

//-----------------------------------------------------------------------------
vec3d FEKinemat::KINE::apply(const vec3d& r)
{
	double q[3] = {r.x, r.y, r.z};
	double d[3];
	d[0] = m[0]*q[0] + m[1]*q[1] + m[ 2]*q[2] + m[ 3];
	d[1] = m[4]*q[0] + m[5]*q[1] + m[ 6]*q[2] + m[ 7];
	d[2] = m[8]*q[0] + m[9]*q[1] + m[10]*q[2] + m[11];
	return vec3d(d[0], d[1], d[2]);
}

vec3d FEKinemat::KINE::translate()
{
	return vec3d(m[3], m[7], m[11]);
}

mat3d FEKinemat::KINE::rotate()
{
	mat3d Q;
	Q[0][0] = m[0]; Q[0][1] = m[1]; Q[0][2] = m[2];
	Q[1][0] = m[4]; Q[1][1] = m[5]; Q[1][2] = m[6];
	Q[2][0] = m[8]; Q[2][1] = m[9]; Q[2][2] = m[10];
	return Q;
}

//-----------------------------------------------------------------------------
FEKinemat::FEKinemat()
{
	m_isKineValid = false;
}

bool FEKinemat::IsKineValid() const
{
	return m_isKineValid;
}

//-----------------------------------------------------------------------------
void FEKinemat::SetRange(int n0, int n1, int ni)
{
	if (n1 < n0) n1 = n0;
	if (ni < 0) ni = 1;
	m_n0 = n0 - 1;
	m_n1 = n1 - 1;
	m_ni = ni;
}

int FEKinemat::States() const
{
	return m_State.size();
}

//-----------------------------------------------------------------------------
bool FEKinemat::Apply(Post::FEPostModel* fem, const char* szkine)
{
	// read the kinematics file data
	m_isKineValid = false;
	if (ReadKine(szkine) == false) return false;
	m_isKineValid = true;

	// build the states
	if (BuildStates(fem) == false) return false;

	return true;
}

//-----------------------------------------------------------------------------
bool FEKinemat::ReadKine(const char* szfile)
{
	FILE* fp = fopen(szfile, "rt");
	if (fp == 0) return false;

	STATE s; KINE d;
	char szline[2048] = {0};
	while (fgets(szline, 2047, fp))
	{
		char* ch = szline;
		int n = 0;
		s.D.clear();
		do
		{
			d.m[n++] = (float) atof(ch);
			if (n == 16)
			{
				s.D.push_back(d);
				n = 0;
			}
			ch = strchr(ch, ',');
			if (ch) ch++;
		}
		while (ch);

		m_State.push_back(s);
	}

	fclose(fp);
	return true;
}

//-----------------------------------------------------------------------------
bool FEKinemat::BuildStates(Post::FEPostModel* pfem)
{
	if (pfem == nullptr) return false;
	Post::FEPostModel& fem = *pfem;

	// add the first state
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);
	int NMAT = fem.Materials();
	int NN = mesh.Nodes();
	int NE = mesh.Elements();

	// find the displacement field
	FEDataManager* pdm = fem.GetDataManager();
	int N = pdm->DataFields();
	FEDataFieldPtr pt = pdm->FirstDataField();
	int ND = -1;
	for (int i=0; i<N; ++i, ++pt)
	{
		 if ((*pt)->GetName() == "Displacement")
		 {
			 ND = i;
			 break;
		 }
	}
	if (ND == -1) return false;

	// get the initial coordinates
	vector<vec3d> r0(NN);
	for (int i=0; i<NN; ++i) r0[i] = to_vec3d(fem.NodePosition(i, 0));

	int NS = (int)m_State.size();
	if (m_n0 >= NS) return false;
	if (m_n1 - m_n0 +1 > NS) m_n1 = NS - 1;
	float t = 0.f;
	for (int ns = m_n0; ns <= m_n1; ns += m_ni, t += 1.f)
	{
		STATE& s = m_State[ns];

		// create a new state
		FEState* ps = 0;
		if (t == 0.f) ps = fem.GetState(0);
		else {
			try {
				ps = new FEState(t, &fem, fem.GetFEMesh(0));
				fem.AddState(ps);
			}
			catch (...)
			{
				return false;
			}
		}

		// get the displacement field
		Post::FENodeData<vec3f>& d = dynamic_cast<Post::FENodeData<vec3f>&>(ps->m_Data[ND]);

		for (int i=0; i<NN; ++i) d[i] = vec3f(0.f, 0.f, 0.f);

		int N = NMAT;
		if ((int)s.D.size() < NMAT) N = (int)s.D.size();
		for (int n=0; n<N; ++n)
		{
			KINE& kine = s.D[n];
			for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = 0;
			for (int i=0; i<NE; ++i)
			{
				FEElement_& e = mesh.ElementRef(i);
				if (e.m_MatID == n)
				{
					int ne = e.Nodes();
					for (int j=0; j<ne; ++j) mesh.Node(e.m_node[j]).m_ntag = 1;
				}
			}

			for (int i=0; i<NN; ++i)
			{
				FSNode& nd = mesh.Node(i);
				if (nd.m_ntag == 1)
				{
					const vec3d& r0_i = r0[i];
					d[i] = to_vec3f(kine.apply(r0_i) - r0_i);
				}
			}
		}
	}
	fem.UpdateBoundingBox();
	return true;
}
