// GLMesh.cpp: implementation of the CGLMesh class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GLMesh.h"
#include <stack>
#include <cstring>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGLMesh::CGLMesh()
{
	m_pNode = 0;
	m_nNodes = 0;

	m_pFace = 0;
	m_nFaces = 0;

	m_pLine = 0;
	m_nLines = 0;

	m_bAutoSmooth = false;

	m_bUseTex = false;

	m_bclip = true;

	// set default material parameters
	m_shininess = 0;
	m_transparency = 1;
	m_diffuse [0] = m_diffuse [1] = m_diffuse [2] = 1.0f; m_diffuse [3] = 1.f;
	m_ambient [0] = m_ambient [1] = m_ambient [2] = 0.1f; m_ambient [3] = 1.f;
	m_specular[0] = m_specular[1] = m_specular[2] = 0.f; m_specular[3] = 1.f;
	m_emission[0] = m_emission[1] = m_emission[2] = 0.f; m_emission[3] = 1.f;
	m_bshadow = true;
}

CGLMesh::CGLMesh(CGLMesh& mesh)
{
	int i;

	m_nNodes = mesh.m_nNodes;
	m_nFaces = mesh.m_nFaces;
	m_nLines = mesh.m_nLines;

	m_pNode = new CGLNode[m_nNodes];
	for (i=0; i<m_nNodes; i++) m_pNode[i] = mesh.m_pNode[i];

	m_pFace = new CGLFace[m_nFaces];
	for (i=0; i<m_nFaces; i++) m_pFace[i] =  mesh.m_pFace[i];

	m_pLine = new CGLLine[m_nLines];
	for (i=0; i<m_nLines; i++) m_pLine[i] = mesh.m_pLine[i];

	m_ambient[0]	= mesh.m_ambient[0];
	m_ambient[1]	= mesh.m_ambient[1];
	m_ambient[2]	= mesh.m_ambient[2];
	m_ambient[3]	= mesh.m_ambient[3];
	m_diffuse[0]	= mesh.m_diffuse[0];
	m_diffuse[1]	= mesh.m_diffuse[1];
	m_diffuse[2]	= mesh.m_diffuse[2];
	m_diffuse[3]	= mesh.m_diffuse[3];
	m_emission[0]	= mesh.m_emission[0];
	m_emission[1]	= mesh.m_emission[1];
	m_emission[2]	= mesh.m_emission[2];
	m_emission[3]	= mesh.m_emission[3];
	m_specular[0]	= mesh.m_specular[0];
	m_specular[1]	= mesh.m_specular[1];
	m_specular[2]	= mesh.m_specular[2];
	m_specular[3]	= mesh.m_specular[3];
	m_bAutoSmooth	= mesh.m_bAutoSmooth;
	m_bclip			= mesh.m_bclip;
	m_bUseTex		= mesh.m_bUseTex;
	m_shininess		= mesh.m_shininess;
	m_transparency	= mesh.m_transparency;
}

CGLMesh::~CGLMesh()
{	
	CleanUp();
}

void CGLMesh::CleanUp()
{
	delete [] m_pNode; m_pNode = 0;
	delete [] m_pFace; m_pFace = 0;
	delete [] m_pLine; m_pLine = 0;

	m_nNodes = 0;
	m_nFaces = 0;
	m_nLines = 0;
}

bool CGLMesh::Create(int nodes, int faces, int lines)
{
	int i;
	// Delete the old mesh
	CleanUp();

	// allocate storage for the mesh
	m_nNodes = nodes;
	m_nFaces = faces;
	m_nLines = lines;

	m_pNode = new CNode[nodes];
	m_pFace = new CFace[faces];
	if (lines > 0) m_pLine = new CLine[lines];

	if (m_pNode == 0 || m_pFace == 0 || (lines != 0 && m_pLine == 0))
	{
		CleanUp();
		return false;
	}

	for (i=0; i<faces; i++)
	{
		m_pFace[i].m_nref = -1;
		m_pFace[i].m_bsel = false;
	}

	for (i=0; i<nodes; i++)
	{
		m_pNode[i].m_nref = -1;
		m_pNode[i].m_bsel = false;
	}

	return true;
}

void CGLMesh::RenderShadowVolume(vec3f n, double inf)
{
	// find all silhouette edges
	CFace* pf = m_pFace;
	vec3f fn, fn2;
	n.Normalize();
	for (int i=0; i<m_nFaces; i++, pf++)
	{
		// only look at front facing faces
		fn = pf->m_FNorm;
		if (fn*n >= 0)
		{
			for (int j=0; j<3; j++)
			{
				CFace* pf2 = pf->m_pFace[j];

				if (pf2)
				{
					fn2 = pf2->m_FNorm;
				}

				// we got one!
				if ((pf2 == 0) || (fn2*n < 0))
				{
					vec3f a, b, c, d;
					a = m_pNode[pf->m_node[j]].m_pos;
					b = m_pNode[pf->m_node[(j+1)%3]].m_pos;

					c = a - n*(float)inf;
					d = b - n*(float)inf;

					glBegin(GL_QUADS);
					{
						vec3f n = (c-a)^(d-a);
						n.Normalize();

						glNormal3d(n.x, n.y, n.z);
						glVertex3d(a.x, a.y, a.z);
						glVertex3d(c.x, c.y, c.z);
						glVertex3d(d.x, d.y, d.z);
						glVertex3d(b.x, b.y, b.z);
					}
					glEnd();
				}
			}
		}
		else
		{
			glBegin(GL_TRIANGLES);
			{
				vec3f n = pf->m_FNorm;
				glNormal3d(n.x, n.y, n.z);
				for (int j=2; j>=0; j--)
				{
					vec3f p = m_pNode[pf->m_node[j]].m_pos;
					glVertex3d(p.x, p.y, p.z);
				}
			}
			glEnd();
		}
	}
}

void CGLMesh::RenderFaces(bool bsmooth)
{
	int i, n;
	CFace* pf = m_pFace;

	SetMaterialProps();

	vec3f r1, r2, r3, r4;
	vec3f n1, n2, n3, n4, fn;

	for (i=0; i<m_nFaces; i++, pf++)
	{
		n = pf->Nodes();

		r1 = m_pNode[pf->m_node[0]].m_pos;
		r2 = m_pNode[pf->m_node[1]].m_pos;
		r3 = m_pNode[pf->m_node[2]].m_pos;
		r4 = m_pNode[pf->m_node[3]].m_pos;

		fn = pf->m_FNorm;

		if (bsmooth)
		{
			n1 = pf->m_VNorm[0];
			n2 = pf->m_VNorm[1];
			n3 = pf->m_VNorm[2];
			n4 = pf->m_VNorm[3];
		}
		else n1 = n2 = n3 = n4 = fn;

		if (n==4)
		{
			glBegin(GL_QUADS);
			{
				glNormal3f(n1.x, n1.y, n1.z); glTexCoord1f(pf->m_tex[0]); glVertex3f(r1.x, r1.y, r1.z);
				glNormal3f(n2.x, n2.y, n2.z); glTexCoord1f(pf->m_tex[1]); glVertex3f(r2.x, r2.y, r2.z);
				glNormal3f(n3.x, n3.y, n3.z); glTexCoord1f(pf->m_tex[2]); glVertex3f(r3.x, r3.y, r3.z);
				glNormal3f(n4.x, n4.y, n4.z); glTexCoord1f(pf->m_tex[3]); glVertex3f(r4.x, r4.y, r4.z);
			}
			glEnd();
		}
		else
		{
			glBegin(GL_TRIANGLES);
			{
				glNormal3f(n1.x, n1.y, n1.z); glTexCoord1f(pf->m_tex[0]); glVertex3f(r1.x, r1.y, r1.z);
				glNormal3f(n2.x, n2.y, n2.z); glTexCoord1f(pf->m_tex[1]); glVertex3f(r2.x, r2.y, r2.z);
				glNormal3f(n3.x, n3.y, n3.z); glTexCoord1f(pf->m_tex[2]); glVertex3f(r3.x, r3.y, r3.z);
			}
		}
		glEnd();
	}
}

void CGLMesh::RenderLines()
{
	if (m_nLines == 0) return;

	CLine* pl = m_pLine;
	glBegin(GL_LINES);
	{
		glNormal3d(0,0,0);

		for (int i=0; i<m_nLines; i++, pl++)
		{
			vec3f& r1 = m_pNode[pl->m_node[0]].m_pos;
			vec3f& r2 = m_pNode[pl->m_node[1]].m_pos;

			GLColor& c1 = pl->m_col[0];
			GLColor& c2 = pl->m_col[1];

			glColor3ub(c1.r, c1.g, c1.b);
			glVertex3d(r1.x, r1.y, r1.z);

			glColor3ub(c2.r, c2.g, c2.b);
			glVertex3d(r2.x, r2.y, r2.z);
		}
	}
	glEnd();
}

void CGLMesh::RenderNodes()
{
	int i;
	CNode* pn = m_pNode;

	// we only render the nodes that are visible
	// (i.e. have non-negative references)
	for (i=0; i<m_nNodes; i++, pn++)
		if (pn->m_nref >= 0)
		{
			glBegin(GL_POINTS);
			{
				vec3f& r = pn->m_pos;

				if (pn->m_bsel)
					glColor3ub(255, 255, 255);
				else
					glColor3ub(0, 0, 255);

				glVertex3d(r.x, r.y, r.z);
			}
			glEnd();
		}
}

void CGLMesh::SetColor(byte r, byte g, byte b)
{
	CFace* pf = m_pFace;
	GLColor col(r, g, b);
	for (int i=0; i<m_nFaces; i++, pf++)
	{
		pf->m_Col[0] = col;
		pf->m_Col[1] = col;
		pf->m_Col[2] = col;
	}
}

void CGLMesh::Update()
{
	int i, j, k;

	CFace* pf = m_pFace;
	for (i=0; i<m_nFaces; i++, pf++) pf->m_nID = i;

	// calculate the nodal valences
	int* pval = new int [m_nNodes];
	for (i=0; i<m_nNodes; i++) pval[i] = 0;

	pf = m_pFace;
	for (i=0; i<m_nFaces; i++, pf++)
	{
		pval[ pf->m_node[0] ]++;
		pval[ pf->m_node[1] ]++;
		pval[ pf->m_node[2] ]++;
	}

	int n = 0, t;
	for (i=0; i<m_nNodes; i++)
	{
		t = pval[i];
		pval[i] = n;
		n += t;
	}

	CFace* *pbuf = new CFace*[3*m_nFaces];
	CFace** *pnf = new CFace**[m_nNodes];
	for (i=0; i<m_nNodes; i++) { pnf[i] = pbuf + pval[i]; pval[i] = 0; }

	pf = m_pFace;
	for (i=0; i<m_nFaces; i++, pf++)
	{
		n = pf->m_node[0]; (pnf[n])[pval[n]] = pf; pval[n]++;
		n = pf->m_node[1]; (pnf[n])[pval[n]] = pf; pval[n]++;
		n = pf->m_node[2]; (pnf[n])[pval[n]] = pf; pval[n]++;
	}

	// find the neighbour faces
	pf = m_pFace;
	CFace** ppf;
	int n1, n2;
	for (i=0; i<m_nFaces; i++, pf++)
	{
		for (j=0; j<3; j++)
		{
			n1 = pf->m_node[j];
			n2 = pf->m_node[(j+1)%3];

			ppf = pnf[n1];
			n = pval[n1];

			pf->m_pFace[j] = 0;
			for (k=0; k<n; k++)
				if ((ppf[k] != pf) && (ppf[k]->HasEdge(n1, n2)))
				{
					pf->m_pFace[j] = ppf[k];
					break;
				}
		}
	}

	// clean up
	delete [] pval;
	delete [] pbuf;
	delete [] pnf;

	// calculate the face normals
	pf = m_pFace;
	for (i=0; i<m_nFaces; i++, pf++)
	{
		vec3f& r1 = m_pNode[ pf->m_node[0] ].m_pos;
		vec3f& r2 = m_pNode[ pf->m_node[1] ].m_pos;
		vec3f& r3 = m_pNode[ pf->m_node[2] ].m_pos;

		vec3f& r01 = m_pNode[ pf->m_node[0] ].m_pos0;
		vec3f& r02 = m_pNode[ pf->m_node[1] ].m_pos0;
		vec3f& r03 = m_pNode[ pf->m_node[2] ].m_pos0;


		pf->m_FNorm = (r2 - r1) ^ (r3 - r1);
		pf->m_FNorm.Normalize();

		pf->m_FNorm0 = (r02 - r01) ^ (r03 - r01);
		pf->m_FNorm0.Normalize();

		pf->m_VNorm[0] = vec3f(0,0,0);
		pf->m_VNorm[1] = vec3f(0,0,0);
		pf->m_VNorm[2] = vec3f(0,0,0);
	}

	// create the smoothing groups
	int* psg = new int [m_nFaces];
	int nsg;

	if (m_bAutoSmooth)
	{
		for (i=0; i<m_nFaces; i++) psg[i] = -1;

		stack<CFace*> stack;
		CFace* pnb;

		n = 0;
		nsg = 0;

		float ref = (float) cos(60*PI/180);

		while (n<m_nFaces)
		{
			pf = m_pFace + n;

			if (psg[pf->m_nID] == -1)
			{
				stack.push(pf);

				while (stack.size() > 0)
				{
					pf = stack.top(); stack.pop();
					psg[pf->m_nID] = nsg;

					// push the neighbours if they are within the feature angle
					// note that we use the normal at state 0
					// that way the smoothing groups don't change when the object deforms
					for (j=0; j<3; j++)
					{
						pnb = pf->m_pFace[j];
				
						if (pnb && (psg[pnb->m_nID] == -1) && (pnb->m_FNorm0*pf->m_FNorm0 >= ref))
							stack.push(pnb);
					}
				}

				nsg++;
				n = 0;
			}
			else n++;
		}
	}
	else
	{
		for (i=0; i<m_nFaces; i++) psg[i] = 0;
		nsg = 1;
	}

	vec3f* pv = new vec3f[m_nNodes];

	// loop over smoothing groups
	for (j=0; j<nsg; j++)
	{
		// clear all normals
		for (i=0; i<m_nNodes; i++) pv[i].x = pv[i].y = pv[i].z = 0;

		pf = m_pFace;
		for (i=0; i<m_nFaces; i++, pf++)
			if (psg[i] == j)
			{
				pv[ pf->m_node[0] ] += pf->m_FNorm;
				pv[ pf->m_node[1] ] += pf->m_FNorm;
				pv[ pf->m_node[2] ] += pf->m_FNorm;
			}

		pf = m_pFace;
		for (i=0; i<m_nFaces; i++, pf++)
			if (psg[i] == j)
			{
				pf->m_VNorm[0] = pv[ pf->m_node[0] ];
				pf->m_VNorm[0].Normalize();

				pf->m_VNorm[1] = pv[ pf->m_node[1] ];
				pf->m_VNorm[1].Normalize();

				pf->m_VNorm[2] = pv[ pf->m_node[2] ];
				pf->m_VNorm[2].Normalize();
			}
	}

	delete [] pv;
	delete [] psg;
}

void CGLMesh::SubSample()
{
	int new_nodes = m_nNodes + 3*m_nFaces;
	int new_faces = 4*m_nFaces;

	// create new storage
	CNode* pNtemp = new CNode[new_nodes];
	memcpy(pNtemp, m_pNode, m_nNodes*sizeof(CNode));
	delete [] m_pNode; m_pNode = pNtemp;

	CFace* pFtemp = new CFace[new_faces];

	// create new faces
	CFace* pfo = m_pFace;
	CFace* pfn = pFtemp;
	int node = m_nNodes;
	int a, b, c;
	GLColor c1, c2, c3;
	vec3f n1, n2, n3;
	for (int i=0; i<m_nFaces; i++)
	{
		a = pfo->m_node[0];
		b = pfo->m_node[1];
		c = pfo->m_node[2];

		c1.r = (byte)(int(pfo->m_Col[0].r + pfo->m_Col[1].r) >> 1);
		c1.g = (byte)(int(pfo->m_Col[0].g + pfo->m_Col[1].g) >> 1);
		c1.b = (byte)(int(pfo->m_Col[0].b + pfo->m_Col[1].b) >> 1);
		c1.a = (byte)(int(pfo->m_Col[0].a + pfo->m_Col[1].a) >> 1);

		c2.r = (byte)(int(pfo->m_Col[1].r + pfo->m_Col[2].r) >> 1);
		c2.g = (byte)(int(pfo->m_Col[1].g + pfo->m_Col[2].g) >> 1);
		c2.b = (byte)(int(pfo->m_Col[1].b + pfo->m_Col[2].b) >> 1);
		c2.a = (byte)(int(pfo->m_Col[1].a + pfo->m_Col[2].a) >> 1);

		c3.r = (byte)(int(pfo->m_Col[2].r + pfo->m_Col[0].r) >> 1);
		c3.g = (byte)(int(pfo->m_Col[2].g + pfo->m_Col[0].g) >> 1);
		c3.b = (byte)(int(pfo->m_Col[2].b + pfo->m_Col[0].b) >> 1);
		c3.a = (byte)(int(pfo->m_Col[2].a + pfo->m_Col[0].a) >> 1);

		n1 = (pfo->m_VNorm[0] + pfo->m_VNorm[1])*0.5f; n1.Normalize();
		n2 = (pfo->m_VNorm[1] + pfo->m_VNorm[2])*0.5f; n2.Normalize();
		n3 = (pfo->m_VNorm[2] + pfo->m_VNorm[0])*0.5f; n3.Normalize();

		// create new faces
		pfn[0].m_node[0] = a; pfn[0].m_node[1] = node; pfn[0].m_node[2] = node+2;
		pfn[0].m_bsel = pfo->m_bsel;
		pfn[0].m_Col[0] = pfo->m_Col[0]; pfn[0].m_Col[1] = c1; pfn[0].m_Col[2] = c3;
		pfn[0].m_nref = pfo->m_nref;
		pfn[0].m_VNorm[0] = pfo->m_VNorm[0]; pfn[0].m_VNorm[1] = n1; pfn[0].m_VNorm[2] = n3;
		pfn[0].m_FNorm = (pfn[0].m_VNorm[0]+pfn[0].m_VNorm[1]+pfn[0].m_VNorm[2])/3;

		pfn[1].m_node[0] = node; pfn[1].m_node[1] = b; pfn[1].m_node[2] = node+1;
		pfn[1].m_bsel = pfo->m_bsel;
		pfn[1].m_Col[0] = c1; pfn[1].m_Col[1] = pfo->m_Col[1]; pfn[1].m_Col[2] = c2;
		pfn[1].m_nref = pfo->m_nref;
		pfn[1].m_VNorm[0] = n1; pfn[1].m_VNorm[1] = pfo->m_VNorm[1]; pfn[1].m_VNorm[2] = n2;
		pfn[1].m_FNorm = (pfn[1].m_VNorm[0]+pfn[1].m_VNorm[1]+pfn[1].m_VNorm[2])/3;

		pfn[2].m_node[0] = node+1; pfn[2].m_node[1] = c; pfn[2].m_node[2] = node+2;
		pfn[2].m_bsel = pfo->m_bsel;
		pfn[2].m_Col[0] = c2; pfn[2].m_Col[1] = pfo->m_Col[2]; pfn[2].m_Col[2] = c3;
		pfn[2].m_nref = pfo->m_nref;
		pfn[2].m_VNorm[0] = n2; pfn[2].m_VNorm[1] = pfo->m_VNorm[2]; pfn[2].m_VNorm[2] = n3;
		pfn[2].m_FNorm = (pfn[2].m_VNorm[0]+pfn[2].m_VNorm[1]+pfn[2].m_VNorm[2])/3;

		pfn[3].m_node[0] = node; pfn[3].m_node[1] = node+1; pfn[3].m_node[2] = node+2;
		pfn[3].m_bsel = pfo->m_bsel;
		pfn[3].m_Col[0] = c1; pfn[3].m_Col[1] = c2; pfn[3].m_Col[2] = c3;
		pfn[3].m_nref = pfo->m_nref;
		pfn[3].m_VNorm[0] = n1; pfn[3].m_VNorm[1] = n2; pfn[3].m_VNorm[2] = n3;
		pfn[3].m_FNorm = (pfn[3].m_VNorm[0]+pfn[3].m_VNorm[1]+pfn[3].m_VNorm[2])/3;

		// create new nodes
		float s1 = (1.f - pfo->m_VNorm[0]*pfo->m_VNorm[1])/ (m_pNode[1].m_pos - m_pNode[0].m_pos).Length();
		m_pNode[node].m_pos = n1*s1+(m_pNode[a].m_pos + m_pNode[b].m_pos)*0.5;
		m_pNode[node].m_nref = -1;
		m_pNode[node].m_bsel = false;

		float s2 = (1.f - pfo->m_VNorm[1]*pfo->m_VNorm[2])/ (m_pNode[1].m_pos - m_pNode[2].m_pos).Length();
		m_pNode[node+1].m_pos = n2*s2+(m_pNode[b].m_pos + m_pNode[c].m_pos)*0.5;
		m_pNode[node+1].m_nref = -1;
		m_pNode[node+1].m_bsel = false;

		float s3 = (1.f - pfo->m_VNorm[2]*pfo->m_VNorm[0])/ (m_pNode[2].m_pos - m_pNode[0].m_pos).Length();
		m_pNode[node+2].m_pos = n3*s3+(m_pNode[c].m_pos + m_pNode[a].m_pos)*0.5;
		m_pNode[node+2].m_nref = -1;
		m_pNode[node+2].m_bsel = false;

		pfo += 1;
		pfn += 4;
		node += 3;
	}

	// cleanup old storage
	delete [] m_pFace; m_pFace = pFtemp;

	m_nNodes = new_nodes;
	m_nFaces = new_faces;
}

void CGLMesh::Project(vec3f norm, float rot)
{
	int i;
	int nodes = Nodes();

	// calculate center node
	double rx = 0, ry = 0, rz = 0;
	CGLNode* pn = Node(0);
	for (i=0; i<nodes; i++, pn++)
	{
		rx += pn->m_pos.x;
		ry += pn->m_pos.y;
		rz += pn->m_pos.z;
	}

	rx /= nodes; ry /= nodes; rz /= nodes;
	vec3f r0 = vec3f((float)rx, (float)ry, (float)rz);

	// move to center of mass
	pn = Node(0);
	for (i=0; i<nodes; i++, pn++) pn->m_pos -= r0;

	// rotate to plane and the range
	vec3f n0(0, 0, 1);
	quat4f q(norm, n0);
	q.Inverse();
	quat4f qz(-rot, vec3f(0,0,1));
	q = qz*q;
	pn = Node(0);
	float xmin = 0.f, xmax = 1.f, ymin = 0.f, ymax = 1.f;
	xmin = xmax = pn->m_pos.x; ymin = ymax = pn->m_pos.y;
	for (i=0; i<nodes; i++, pn++)
	{
		q.RotateVector(pn->m_pos);

		if (pn->m_pos.x < xmin) xmin = pn->m_pos.x;
		if (pn->m_pos.x > xmax) xmax = pn->m_pos.x;
		if (pn->m_pos.y < ymin) ymin = pn->m_pos.y;
		if (pn->m_pos.y > ymax) ymax = pn->m_pos.y;
	}
	if (xmin == xmax) xmax++;
	if (ymin == ymax) ymax++;

	float ar = float(xmax - xmin) / float(ymax - ymin);

	if (ar > 1)
	{
		ymax *= ar;
		ymin *= ar;
	}
	else
	{
		xmax /= ar;
		xmin /= ar;
	}

	// scale down to (-1, +1) range
	pn = Node(0);
	for (i=0; i<nodes; i++, pn++)
	{
		pn->m_pos.x = 2.f*(pn->m_pos.x - xmin)/(xmax - xmin) - 1.f;
		pn->m_pos.y = 2.f*(pn->m_pos.y - ymin)/(ymax - ymin) - 1.f;
	}
	
}
