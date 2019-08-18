#include "stdafx.h"
#include "FEPointCongruency.h"
#include "FEModel.h"

using namespace Post;

//-----------------------------------------------------------------------------
FEPointCongruency::FEPointCongruency()
{
	m_nlevels = 1;
	m_nmax = 1;
	m_bext = 0;
}

//-----------------------------------------------------------------------------
FEPointCongruency::CONGRUENCY_DATA FEPointCongruency::Congruency(FEModel* pfem, int nid, int nstate)
{
	CONGRUENCY_DATA d;
	d.H1 = 0;
	d.G1 = 0;
	d.H2 = 0;
	d.G2 = 0;
	d.D = 0;
	d.a = 0;
	d.Ke = 0;
	d.Kemin = 0;
	d.Kemax = 0;
	d.nface = -1;

	// store a pointer to the mesh
	m_pfem = pfem;
	m_nstate = nstate;
	FEMeshBase* pm = m_pfem->GetFEMesh(0);

	// find the projection of the node onto the opposing surface
	vec3f q, sn;
	double rs[2];
	int nface;
	if (Project(nid, nface, q, rs, sn))
	{
		// find the local curvature measures at the node
		d.H1 = nodal_curvature(nid, sn, MEAN);
		d.G1 = nodal_curvature(nid, sn, DIFF);
		double w1 = nodal_curvature(nid, sn, ANGLE);

		// find the local curvature measure at the projection
		d.nface = nface;
		FEFace& face = pm->Face(nface);
		vec3f tsn = -sn;
		d.H2 = face_curvature(face, rs, tsn, MEAN);
		d.G2 = face_curvature(face, rs, tsn, DIFF);
		double w2 = face_curvature(face, rs, tsn, ANGLE);

		// find the angle between the principal axes of curvature
		d.a = w1 - w2 + PI;

		// calculate congruency
		double D2 = (d.G1*d.G1) + (d.G2*d.G2) + 2.0*d.G1*d.G2*cos(2.0*d.a);
		d.D = sqrt(D2);
		d.Kemin = d.H1 + d.H2 - 0.5*d.D;
		d.Kemax = d.H1 + d.H2 + 0.5*d.D;
		
		d.Ke = sqrt((d.Kemin*d.Kemin+d.Kemax*d.Kemax)*0.5);
	}
	return d;
}

//-----------------------------------------------------------------------------
float FEPointCongruency::face_curvature(FEFace& face, double rs[2], vec3f& sn, int m)
{
	int nf = face.Nodes();
	double K[4] = {0};
	for (int i=0; i<nf; ++i)
	{
		K[i] = nodal_curvature(face.node[i], sn, m);
	}

	double r = rs[0];
	double s = rs[1];
	double H[4];
	if (nf == 3)
	{
		H[0] = 1 - r - s;
		H[1] = r;
		H[2] = s;
	}
	else if (nf == 4)
	{
		H[0] = 0.25*(1 - r)*(1 - s);
		H[1] = 0.25*(1 + r)*(1 - s);
		H[2] = 0.25*(1 + r)*(1 + s);
		H[3] = 0.25*(1 - r)*(1 + s);
	}
	double Ke = 0;
	for (int i=0; i<nf; ++i) Ke += K[i]*H[i];
	
	return (float)Ke;
}

//-----------------------------------------------------------------------------
bool FEPointCongruency::Project(int nid, int& nface, vec3f& q, double rs[2], vec3f& sn)
{
	FEMeshBase* pm = m_pfem->GetFEMesh(0);

	// get the node position
	vec3f nr = m_pfem->NodePosition(nid, m_nstate);

	// find the normal at this node
	sn = vec3f(0.f, 0.f, 0.f);
	for (int i=0; i<pm->Faces(); ++i)
	{
		FEFace& face = pm->Face(i);
		int nf = face.Nodes();
		for (int j=0; j<nf; ++j)
		{
			if (face.node[j] == nid) sn += face.m_nn[j];
		}
	}
	sn.Normalize();

	// find the intersection of the ray with the surface makin sure not to intersect
	// faces that contain this node
	Ray ray = {nr, sn};
	return Intersect(ray, nface, nid, q, rs);
}

//-----------------------------------------------------------------------------
bool FEPointCongruency::Intersect(const Ray& ray, int& nface, int nid, vec3f& q, double rs[2])
{
	FEMeshBase* pm = m_pfem->GetFEMesh(0);
	nface = -1;
	double Dmin = 0;
	double rsi[2];
	for (int i=0; i<pm->Faces(); ++i)
	{
		FEFace& face = pm->Face(i);
		// make sure this face does not contain nid
		if (face.HasNode(nid) == false)
		{
			bool b = false;
			switch (face.m_ntype)
			{
			case FACE_TRI3 : b = IntersectTri3 (ray, face, q, rsi); break;
			case FACE_QUAD4: b = IntersectQuad4(ray, face, q, rsi); break;
			}

			if (b)
			{
				double D = (q - ray.origin).Length();
				if ((nface == -1) || (D < Dmin))
				{
					nface = i;
					Dmin = D;
					rs[0] = rsi[0];
					rs[1] = rsi[1];
				}
			}
		}
	}
	return (nface != -1);
}

//-----------------------------------------------------------------------------
bool FEPointCongruency::IntersectTri3(const Ray& ray, FEFace& face, vec3f& q, double rs[2])
{
	const double tol = 0.01;

	FEMeshBase* pm = m_pfem->GetFEMesh(0);

	vec3f n1 = m_pfem->NodePosition(face.node[0], m_nstate);
	vec3f n2 = m_pfem->NodePosition(face.node[1], m_nstate);
	vec3f n3 = m_pfem->NodePosition(face.node[2], m_nstate);

	Triangle tri = {n1, n2, n3};

	Intersection intersect;
	bool b = ::IntersectTriangle(ray, tri, intersect);

	q = intersect.point;
	rs[0] = intersect.r[0];
	rs[1] = intersect.r[1];

	return b;
}

//-----------------------------------------------------------------------------
bool FEPointCongruency::IntersectQuad4(const Ray& ray, FEFace& face, vec3f& q, double rs[2])
{
	const double tol = 0.01;
	FEFace tri1,tri2;
	tri1.node[0] = face.node[0]; tri1.node[1] = face.node[1]; tri1.node[2] = face.node[2];
	tri1.m_nn[0] = face.m_nn[0]; tri1.m_nn[1] = face.m_nn[1]; tri1.m_nn[2] = face.m_nn[2];

	tri2.node[0] = face.node[2]; tri2.node[1] = face.node[3]; tri2.node[2] = face.node[0];
	tri2.m_nn[0] = face.m_nn[2]; tri2.m_nn[1] = face.m_nn[3]; tri2.m_nn[2] = face.m_nn[0];

	if (IntersectTri3(ray, tri1, q, rs) ||
		IntersectTri3(ray, tri2, q, rs))
	{
		FEMeshBase* pm = m_pfem->GetFEMesh(0);
		vec3f y[4];
		y[0] = m_pfem->NodePosition(face.node[0], m_nstate);
		y[1] = m_pfem->NodePosition(face.node[1], m_nstate);
		y[2] = m_pfem->NodePosition(face.node[2], m_nstate);
		y[3] = m_pfem->NodePosition(face.node[3], m_nstate);

		Quad quad = {y[0], y[1], y[2], y[3]};
		Intersection intersect;
		bool b = ::IntersectQuad(ray, quad, intersect);

		q = intersect.point;
		rs[0] = intersect.r[0];
		rs[1] = intersect.r[1];
		return b;
	}
	else return false;
}

//-----------------------------------------------------------------------------
float FEPointCongruency::nodal_curvature(int nid, vec3f& sn, int m)
{
	// get the model's surface
	FEMeshBase* pmesh = m_pfem->GetFEMesh(0);

	// get the reference nodal position
	vec3f r0 = m_pfem->NodePosition(nid, m_nstate);

	// get the node-face list
	vector<NodeFaceRef>& nfl = pmesh->NodeFaceList(nid);
	int NF = nfl.size();

	// array of nodal points
	vector<vec3f> x; x.reserve(128);
	vector<vec3f> y; y.reserve(128);

	// number of levels
	int nlevels = m_nlevels - 1;
	if (nlevels <  0) nlevels = 0;
	if (nlevels > 10) nlevels = 10;

	// get the neighbors
	set<int> nl1;
	level(nid, nlevels, nl1);

	// get the node coordinates
	set<int>::iterator it;
	for (it=nl1.begin(); it != nl1.end(); ++it)
	{
		if (*it != nid) x.push_back(m_pfem->NodePosition(*it, m_nstate));
	}
	y.resize(x.size());
	int nn = x.size();

	// for less than three neighbors, we assume K = 0
	double K, k1, k2;
	if (nn < 3) K = 0;
	else if ((nn < 5) || (m_bext == 0) || (m==6))
	{
		// for less than 5 points, or if the extended quadric flag is zero,
		// we use the quadric method

		// construct local coordinate system
		vec3f e3 = sn;

		vec3f qx(1.f-sn.x*sn.x, -sn.y*sn.x, -sn.z*sn.x);
		if (qx.Length() < 1e-5) qx = vec3f(-sn.x*sn.y, 1.f-sn.y*sn.y, -sn.z*sn.y);
		qx.Normalize();
		vec3f e1 = qx;

		vec3f e2 = e3 ^ e1;

		Mat3d Q;
		Q[0][0] = e1.x; Q[1][0] = e2.x; Q[2][0] = e3.x;
		Q[0][1] = e1.y; Q[1][1] = e2.y; Q[2][1] = e3.y;
		Q[0][2] = e1.z; Q[1][2] = e2.z; Q[2][2] = e3.z;
		Mat3d Qt = Q.transpose();

		// map coordinates
		for (int i=0; i<nn; ++i)
		{
			vec3f tmp = x[i] - r0;
			y[i] = Q*tmp;
		}

		// setup the linear system
		Matrix R(nn, 3);
		vector<double> r(nn);
		for (int i=0; i<nn; ++i)
		{
			vec3f& p = y[i];
			R[i][0] = p.x*p.x;
			R[i][1] = p.x*p.y;
			R[i][2] = p.y*p.y;
			r[i] = p.z;
		}

		// solve for quadric coefficients
		vector<double> q(3);
		R.lsq_solve(q, r);
		double a = q[0], b = q[1], c = q[2];

		// calculate curvature
		switch (m)
		{
		case 0: // Gaussian
			K = 4*a*c - b*b;
			break;
		case 1:	// Mean
			K = a + c;
			break;
		case 2:	// 1-principal
			K = a + c + sqrt((a-c)*(a-c) + b*b);
			break;
		case 3:	// 2-principal
			K = a + c - sqrt((a-c)*(a-c) + b*b);
			break;
		case 4:	// rms
			{
				double k1 = a + c + sqrt((a-c)*(a-c) + b*b);
				double k2 = a + c - sqrt((a-c)*(a-c) + b*b);
				K = sqrt(0.5*(k1*k1 + k2*k2));
			}
			break;
		case 5: // diff
			{
				k1 = a + c + sqrt((a-c)*(a-c) + b*b);
				k2 = a + c - sqrt((a-c)*(a-c) + b*b);
				K = k2 - k1;
			}
			break;
		case 6: // alpha
			{
				K = 0.5*atan2(b, a - c);
			}
			break;
		default:
			assert(false);
			K = 0;
		}
	}
	else
	{
		// loop until converged
		const int NMAX = 100;
		if (m_nmax > NMAX) m_nmax = NMAX;
		if (m_nmax < 1) m_nmax = 1;
		int niter = 0;
		while (niter < m_nmax)
		{
			// construct local coordinate system
			vec3f e3 = sn;

			vec3f qx(1.f-sn.x*sn.x, -sn.y*sn.x, -sn.z*sn.x);
			if (qx.Length() < 1e-5) qx = vec3f(-sn.x*sn.y, 1.f-sn.y*sn.y, -sn.z*sn.y);
			qx.Normalize();
			vec3f e1 = qx;

			vec3f e2 = e3 ^ e1;

			Mat3d Q;
			Q[0][0] = e1.x; Q[1][0] = e2.x; Q[2][0] = e3.x;
			Q[0][1] = e1.y; Q[1][1] = e2.y; Q[2][1] = e3.y;
			Q[0][2] = e1.z; Q[1][2] = e2.z; Q[2][2] = e3.z;
			Mat3d Qt = Q.transpose();

			// map coordinates
			for (int i=0; i<nn; ++i)
			{
				vec3f tmp = x[i] - r0;
				y[i] = Q*tmp;
			}

			// setup the linear system
			Matrix R(nn, 5);
			vector<double> r(nn);
			for (int i=0; i<nn; ++i)
			{
				vec3f& p = y[i];
				R[i][0] = p.x*p.x;
				R[i][1] = p.x*p.y;
				R[i][2] = p.y*p.y;
				R[i][3] = p.x;
				R[i][4] = p.y;
				r[i] = p.z;
			}

			// solve for quadric coefficients
			vector<double> q(5);
			R.lsq_solve(q, r);
			double a = q[0], b = q[1], c = q[2], d = q[3], e = q[4];

			// calculate curvature
			double D = 1.0 + d*d + e*e;
			switch (m)
			{
			case 0: // Gaussian
				K = (4*a*c - b*b) / (D*D);
				break;
			case 1: // Mean
				K = (a + c + a*e*e + c*d*d - b*d*e)/pow(D, 1.5);
				break;
			case 2: // 1-principal
				{
					double H = (a + c + a*e*e + c*d*d - b*d*e)/pow(D, 1.5);
					double G = (4*a*c - b*b) / (D*D);
					K = H + sqrt(H*H - G);
				}
				break;
			case 3: // 2-principal
				{
					double H = (a + c + a*e*e + c*d*d - b*d*e)/pow(D, 1.5);
					double G = (4*a*c - b*b) / (D*D);
					K = H - sqrt(H*H - G);
				}
				break;
			case 4: // RMS
				{
					double H = (a + c + a*e*e + c*d*d - b*d*e)/pow(D, 1.5);
					double G = (4*a*c - b*b) / (D*D);
					double k1 = H + sqrt(H*H - G);
					double k2 = H - sqrt(H*H - G);
					K = sqrt((k1*k1+k2*k2)*0.5);
				}
				break;
			case 5:
				{
					double H = (a + c + a*e*e + c*d*d - b*d*e)/pow(D, 1.5);
					double G = (4*a*c - b*b) / (D*D);
					double k1 = H + sqrt(H*H - G);
					double k2 = H - sqrt(H*H - G);
					K = k1 - k2;
				}
				break;
			default:
				assert(false);
				K = 0;
			}

			// setup the new normal
			sn = vec3f(- (float) d, - (float) e, 1.f);
			sn.Normalize();
			sn = Qt*sn;

			// increase counter
			niter++;
		}
	}

	// return result
	return (float) K;
}

//-----------------------------------------------------------------------------
void FEPointCongruency::level(int n, int l, set<int>& nl1)
{
	// get the model's surface
	FEMeshBase* pmesh = m_pfem->GetFEMesh(0);

	// add the first node
	nl1.insert(n);

	// loop over all levels
	vector<int> nl2; nl2.reserve(64);
	for (int k=0; k<=l; ++k)
	{
		// reset face marks
		set<int>::iterator it;
		for (it = nl1.begin(); it != nl1.end(); ++it)
		{
			// get the node-face list
			vector<NodeFaceRef>& nfl = pmesh->NodeFaceList(*it);
			int NF = nfl.size();

			// add the other nodes
			for (int i=0; i<NF; ++i)
			{
				FEFace& f = pmesh->Face(nfl[i].first); 
				f.m_ntag = 0;
			}
		}

		// loop over all nodes
		nl2.clear();
		for (it = nl1.begin(); it != nl1.end(); ++it)
		{
			// get the node-face list
			vector<NodeFaceRef>& nfl = pmesh->NodeFaceList(*it);
			int NF = nfl.size();

			// add the other nodes
			for (int i=0; i<NF; ++i)
			{
				FEFace& f = pmesh->Face(nfl[i].first);
				if (f.m_ntag == 0)
				{
					int ne = f.Nodes();
					for (int j=0; j<ne; ++j) if (f.node[j] != *it) nl2.push_back(f.node[j]);
					f.m_ntag = 1;
				}
			}
		}

		// merge sets
		if (!nl2.empty()) nl1.insert(nl2.begin(), nl2.end());
	}
}
