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
#include "FEMeshSmoothingModifier.h"
#include <MeshLib/FSNodeFaceList.h>
#include <MeshLib/FSNodeNodeList.h>
#include <MeshLib/MeshTools.h>
#include "FEFillHole.h"
#include <algorithm>
using namespace std;

//-----------------------------------------------------------------------------
//! Constructor
FEMeshSmoothingModifier::FEMeshSmoothingModifier() : FEModifier("Mesh Smoothing")
{
	m_threshold1 = 0.0;
	m_threshold2 = 0.0;
	m_iteration = 0;
	m_noise = 1.0;
	m_method = 1;
}


//-----------------------------------------------------------------------------
//! Create the decimate mesh. 
//! \todo This implementation will only work with closed surfaces. 

FSMesh* FEMeshSmoothingModifier::Apply(FSMesh* pm)
{
	// make sure this is a triangle mesh
	if (pm->IsType(FE_TRI3) == false) return 0;

	// make a copy of this mesh
	FSMesh* pnew = new FSMesh(*pm);

	//marking the edge nodes.
	vector<int> hashmap; 
	hashmap.reserve(pm->Nodes());
	for (int i = 0 ; i< pm->Nodes();i++)
		hashmap.push_back(0);	
	for (int i =0;i<pm->Edges();++i)
	{
		FSEdge& ed = pm->Edge(i);
		hashmap[ed.n[0]] = -1;
		hashmap[ed.n[1]] = -1;
	}
	switch(m_method)
	{
		case 0:
			Laplacian_Smoothing(pnew,hashmap);
			break;
		case 1:
			Laplacian_Smoothing2(pnew,hashmap);
			break;
		case 2:
			Taubin_Smoothing(pnew,hashmap);
			break;
		case 3:
			Crease_Enhancing_Diffusion(pnew,hashmap);
			break;
		case 4:	
			Add_Noise(pnew,hashmap);
			break;
		default:
			break;
	}

	pnew->RebuildMesh();
	return pnew;
}

void FEMeshSmoothingModifier::Laplacian_Smoothing(FSMesh* pnew,vector<int> hashmap)
{
	//Creating a node node list
	FSNodeNodeList NNL(pnew);

	for(int j =0 ;j<m_iteration;j++)
	{
		for(int i = 0; i < pnew->Nodes() ; i++)
		{
			if(hashmap[i] == 0)
			{
				FSNode& ni = pnew->Node(i);
				vec3d r_new; 
				for (int k = 0; k<NNL.Valence(i);k++)
				{
					vec3d x = pnew->Node(NNL.Node(i, k)).r;
					r_new = r_new + x;
				}
				r_new = r_new/NNL.Valence(i);
				r_new =(r_new * m_threshold1) + (ni.r * (1-m_threshold1));
				ni.r = r_new;
			}
		}
	}
}

void FEMeshSmoothingModifier::Laplacian_Smoothing2(FSMesh* pnew,vector<int> hashmap)
{
	//Creating a node node list
	FSNodeNodeList NNL(pnew);

	for(int j =0 ;j<m_iteration;j++)
	{
		for(int i = 0; i < pnew->Nodes() ; i++)
		{
			if(hashmap[i] == 0)
			{
				FSNode& ni = pnew->Node(i);
				vec3d r_new; 
				double sum_dist=0;
				for (int k = 0; k<NNL.Valence(i);k++)
				{
					vec3d x = pnew->Node(NNL.Node(i, k)).r;
					double dist = (x - ni.r).Length();
					r_new = r_new + (x * dist);
					sum_dist += dist;
				}
				r_new = r_new/sum_dist;
				r_new =(r_new * m_threshold1) + (ni.r * (1-m_threshold1));
				ni.r = r_new;
			}
		}
	}
}

void FEMeshSmoothingModifier::Taubin_Smoothing(FSMesh* pnew,vector<int> hashmap)
{
	//Creating a node node list
	FSNodeNodeList NNL(pnew);
	
	vector<vec3d>phi_node;
	phi_node.reserve(pnew->Nodes());
	for(int j =0 ;j<m_iteration;j++)
	{		
		for(int i = 0; i < pnew->Nodes() ; i++)
		{
			FSNode& ni = pnew->Node(i);
			vec3d r_sum;
			for (int k = 0; k<NNL.Valence(i);k++)
			{
				vec3d x = pnew->Node(NNL.Node(i, k)).r;
				r_sum += x;
			}
			r_sum = r_sum/NNL.Valence(i);
			r_sum -= ni.r;
			phi_node.push_back(r_sum);
		}		
		for(int i = 0; i < pnew->Nodes() ; i++)
		{
			if(hashmap[i] == 0)
			{
				FSNode& ni = pnew->Node(i);
				vec3d phi_old = phi_node[i];

				vec3d r_sq_sum,phi_sq_old; 
				for (int k = 0; k<NNL.Valence(i);k++)
				{
					int neigh_node = NNL.Node(i, k);
					r_sq_sum += phi_node[neigh_node];
				}
				phi_sq_old = r_sq_sum/NNL.Valence(i);
				phi_sq_old -= phi_old;

				ni.r = ni.r - (phi_old * (m_threshold2 - m_threshold1)) - (phi_sq_old *(m_threshold1*m_threshold2));
			}
		}
		phi_node.clear();
	}
}

void FEMeshSmoothingModifier::Crease_Enhancing_Diffusion(FSMesh* pnew,vector<int> hashmap)
{
	//creating Node Element list
	FSNodeFaceList NFL;
	NFL.Build(pnew);

	//calculating m(R) for each face i.e for each triangle	
	vector<FSFace*> mR ;
	vector<vec3d>m_R;
	vector<vec3d>m_R_new;	
	mR.reserve(pnew->Faces());
	m_R.reserve(pnew->Faces());
	m_R_new.reserve(pnew->Faces());

	//for first iteration m_R are normals
	for(int i =0; i< pnew->Faces();i++)
	{
		FSFace& fa = pnew->Face(i);
		m_R.push_back(to_vec3d(fa.m_fn));
	}
	for (int iter = 0 ; iter< m_iteration;iter++)
	{				
		//for each face calculate m_R
		for(int i =0;i<pnew->Faces();i++)
		{
			FSFace& fa = pnew->Face(i);				
			vec3d centroid_R = (pnew->Node(fa.n[0]).r + pnew->Node(fa.n[1]).r + pnew->Node(fa.n[2]).r )/3;
			//finding the neighbouring faces
			for(int j =0 ;j<3 ;j++)
			{
				int nodeID = fa.n[j];
				for (int k = 0; k<NFL.Valence(nodeID);k++)
				{
					FSFace *fa1 = NFL.Face(nodeID,k);
					if(fa1->m_ntag != 1 && fa1->m_elem[0].eid != i)
					{
						fa1->m_ntag = 1;
						mR.push_back(fa1);
					}
				}					
			}
			//now we have the neighbouring faces in mR list
			double weight =0;
			m_R_new.push_back(vec3d(0,0,0));
			for(int k =0;k<mR.size();k++)
			{
				FSFace *fa1 = mR[k];
				vec3d r[3]; //three nodes of the face
				r[0] = pnew->Node(fa1->n[0]).r;
				r[1] = pnew->Node(fa1->n[1]).r;
				r[2] = pnew->Node(fa1->n[2]).r;
				vec3d centroid_S = (r[0]+r[1]+r[2])/3;
				double dist = (centroid_S - centroid_R).Length();
				double angle = acos((fa.m_fn * fa1->m_fn)/(fa.m_fn.Length() * fa1->m_fn.Length()));//angle between the normals
				double weight1 = area_triangle(r) * exp(-m_threshold1 * angle*angle*dist*dist);
				weight += weight1;
				m_R_new[i] += m_R[fa1->m_elem[0].eid] * weight1;
				fa1->m_ntag = 0;
			}
			m_R_new[i] = m_R_new[i]/weight;	
			mR.clear();
		}
		//we have m_R_new for each face.
		for(int i =0 ;i<m_R_new.size();i++)
		{
			m_R[i] = m_R_new[i];
		}
		m_R_new.clear();
		//For each node modify its coodinates
		for(int i = 0 ;i < pnew->Nodes();i++)
		{
			if(hashmap[i] == 0) //not the edge node
			{
				FSNode& ni = pnew->Node(i);
				vec3d vR; 
				double weight=0;
				for (int k = 0; k<NFL.Valence(i);k++)
				{
					FSFace *fa1 = NFL.Face(i,k);
					vec3d r[3]; //three nodes of the face
					r[0] = pnew->Node(fa1->n[0]).r;
					r[1] = pnew->Node(fa1->n[1]).r;
					r[2] = pnew->Node(fa1->n[2]).r;
					vec3d centroid = (r[0]+r[1]+r[2])/3;
					weight += area_triangle(r);
					vec3d PC = centroid - ni.r;
					double temp = PC * m_R[fa1->m_elem[0].eid];
					vR += (m_R[fa1->m_elem[0].eid] * temp)*area_triangle(r);
				}	
				vR = vR/weight;
				ni.r = ni.r + vR;
			}				
		}
	}//end of one iteration
}

double frand(double dmin = 0.0, double dmax = 1.0)
{
	double f = (double)rand() / (double)RAND_MAX;
	return (dmin + f*(dmax - dmin));
}

void FEMeshSmoothingModifier::Add_Noise(FSMesh* pnew, vector<int> hashmap)
{
	for (int j = 0; j<m_iteration; j++)
	{
		for (int i = 0; i < pnew->Nodes(); i++)
		{
			if (hashmap[i] == 0)
			{
				FSNode& ni = pnew->Node(i);
				int temp = 1;
				ni.r.x += frand(-m_noise, m_noise);
				ni.r.y += frand(-m_noise, m_noise);
				ni.r.z += frand(-m_noise, m_noise);
			}
		}
	}
}
