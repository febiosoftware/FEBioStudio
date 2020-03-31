#include "stdafx.h"
#include "FEKinemat.h"
#include "stdafx.h"
#include <PostLib/FEMeshData_T.h>
#include <PostGL/GLModel.h>
using namespace Post;

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

//-----------------------------------------------------------------------------
FEKinemat::FEKinemat()
{

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

//-----------------------------------------------------------------------------
bool FEKinemat::Apply(Post::CGLModel* glm, const char* szkine)
{
	// read the kinematics file data
	if (ReadKine(szkine) == false) return false;

	// build the states
	if (BuildStates(glm) == false) return false;

	// update displacements on all states
	CGLModel& mdl = *glm;
	if (mdl.GetDisplacementMap() == nullptr)
	{
		mdl.AddDisplacementMap("Displacement");
	}

	int nstates = mdl.GetFEModel()->GetStates();
	for (int i=0; i<nstates; ++i) mdl.UpdateDisplacements(i, true);

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
bool FEKinemat::BuildStates(Post::CGLModel* glm)
{
	FEModel& fem = *glm->GetFEModel();
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
	for (int i=0; i<NN; ++i) r0[i] = fem.NodePosition(i, 0);

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
				FENode& nd = mesh.Node(i);
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
