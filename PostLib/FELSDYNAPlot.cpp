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
#include "FELSDYNAPlot.h"
#include "FEDataManager.h"
#include "constants.h"
#include "FEMeshData_T.h"
#include "FEPostModel.h"
using namespace Post;

// this function performs a big-endian to little endian or vice versa byteswap
// this is used by the plotfile import routine
void byteswap(int* pi, int n)
{
	union
	{
		int m;
		unsigned char uint8_t[4];
	};

	for (int i=0; i<n; i++)
	{
		m = pi[i];

		uint8_t[0] ^= uint8_t[3];
		uint8_t[3] ^= uint8_t[0];
		uint8_t[0] ^= uint8_t[3];

		uint8_t[1] ^= uint8_t[2];
		uint8_t[2] ^= uint8_t[1];
		uint8_t[1] ^= uint8_t[2];

		pi[i] = m;
	}
}

//-----------------------------------------------------------------------------
FELSDYNAPlotImport::FELSDYNAPlotImport(FEPostModel* fem) : FEFileReader(fem)
{
	m_brepeat = false;
	m_naction = 0;
}

FELSDYNAPlotImport::~FELSDYNAPlotImport()
{
}

//-----------------------------------------------------------------------------
bool FELSDYNAPlotImport::Load(const char *szfile)
{
	// reset family plot file counter
	m_ifile = 0;

	// open the plot file
	if (Open(szfile, "rb") == false) return errf("Failed opening file %s.", szfile);

	// read the header
	if (ReadHeader(*m_fem) == false) return false;

	// create materials
	CreateMaterials(*m_fem);

	// read the mesh
	if (ReadMesh(*m_fem) == false) return false;

	// read the states
	if (ReadStates(*m_fem) == false) return false;

	return true;
}

//-----------------------------------------------------------------------------
// This function reads a buffer of data. If an end-of-file is encountered, the
// next file in the family is opened. If the next file cannot be found, this function
// returns -1.
// If bdump is true, the last read data will be discarded when reading the next file
// otherwise, the data is concatenated.
int FELSDYNAPlotImport::ReadData(void* pd, size_t nsize, size_t ncnt, bool bdump)
{
	int nread = (int)fread(pd, nsize, ncnt, m_fp);
	while (nread < ncnt)
	{
		// if we get here, there was a problem reading the data.
		// We assume that we've reached the end of the file and try to load
		// the next plot file
		Close();
		m_ifile++;

		char sznewfile[256] = {0};
		string fileName = GetFileName();
		sprintf(sznewfile, "%s%02d", fileName.c_str(), m_ifile);

		// try to open it
		m_fp = fopen(sznewfile, "rb");
		if (m_fp == 0) return -1;

		// try to read the rest of the data
		if (bdump)
		{
			nread = (int)fread(pd, nsize, ncnt, m_fp);
		}
		else nread += (int)fread((char*)pd + nread*nsize, nsize, ncnt - nread, m_fp);

		// always concatenate if a single state is spread across multiple files.
		bdump = false;
		assert(nread <= ncnt);
	}

	// do a byte swap if necessary
	if (m_bswap) byteswap((int*)pd, (int)ncnt);

	return nread;
}

//-----------------------------------------------------------------------------
bool FELSDYNAPlotImport::ReadHeader(FEPostModel& fem)
{
	// read the plot header
	if (fread(&m_hdr, 64*sizeof(int), 1, m_fp) == -1) return errf("Failed reading header. This is an invalid or corrupted file.");

	// check to see if we need to byteswap
	m_bswap = false;
	int nn1 = m_hdr.nump;
	byteswap((int*) &m_hdr + 10, 54);
	int nn2 = m_hdr.nump;

	if ((nn1<0) || ((nn2 < nn1) && (nn2 > 0))) m_bswap = true; else byteswap((int*) &m_hdr + 10, 54);

	// check the code and dimensions
	// to make sure this is a valid plot file
	if (((m_hdr.icode != 6)&&(m_hdr.icode != 1)) || ((m_hdr.ndim != 4)&&(m_hdr.ndim!=3)))
	{
		Close();
		return errf("Invalid format");
	}

	// store the title
	if (strlen(m_hdr.Title) > 0)
		fem.SetTitle(m_hdr.Title);
	else 
		fem.SetTitle("(No title)");

	// set the data fields we are going to read
	FEDataManager* pdm = fem.GetDataManager();
	pdm->Clear();

	for (int i=0; i<LSDYNA_MAXFIELDS; ++i) m_nfield[i] = -1;

	int nd = 0;
	// nodal data
	if (m_hdr.flagU) { pdm->AddDataField(new FEDataField_T<FENodeData<vec3f> >(&fem, EXPORT_DATA), "displacement"); m_nfield[LSDYNA_DISP] = nd++; }
	if (m_hdr.flagV) { pdm->AddDataField(new FEDataField_T<FENodeData<vec3f> >(&fem, EXPORT_DATA), "velocity"); m_nfield[LSDYNA_VEL ] = nd++; }
	if (m_hdr.flagA) { pdm->AddDataField(new FEDataField_T<FENodeData<vec3f> >(&fem, EXPORT_DATA), "acceleration"); m_nfield[LSDYNA_ACC ] = nd++; }
	if (m_hdr.flagT) { pdm->AddDataField(new FEDataField_T<FENodeData<float> >(&fem, EXPORT_DATA), "temperature"); m_nfield[LSDYNA_TEMP] = nd++; }
	pdm->AddDataField(new FEDataField_T<NodeInitPos >(&fem), "Initial position"); nd++;
	pdm->AddDataField(new FEDataField_T<NodePosition>(&fem), "Position"); nd++;

	// add some additional data
	if (m_hdr.flagU)
	{
		// element data
		pdm->AddDataField(new FEDataField_T<FEElementData<mat3fs,DATA_ITEM> >(&fem, EXPORT_DATA), "stress"); m_nfield[LSDYNA_STRESS] = nd++;
		pdm->AddDataField(new FEDataField_T<FEElementData<float, DATA_ITEM> >(&fem, EXPORT_DATA), "pressure"); m_nfield[LSDYNA_PRESSURE] = nd++;
		pdm->AddDataField(new FEDataField_T<FEElementData<float, DATA_ITEM>  >(&fem, EXPORT_DATA), "plastic strain"); m_nfield[LSDYNA_PLASTIC] = nd++;
		if (m_hdr.nv2d == 44) { pdm->AddDataField(new FEDataField_T<FEElementData<mat3fs,DATA_ITEM> >(&fem, EXPORT_DATA), "shell strain"); m_nfield[LSDYNA_SHELL_STRAIN] = nd++; }

		// additional element data
		pdm->AddDataField(new StrainDataField(&fem, StrainDataField::LAGRANGE), "Lagrange strain");
	}
	else fem.SetDisplacementField(-1);
 
	return true;
}

//-----------------------------------------------------------------------------
void FELSDYNAPlotImport::CreateMaterials(FEPostModel& fem)
{
	// initialize material properties
	fem.ClearMaterials();
	int nmat = m_hdr.nummat4 + m_hdr.nummat8 + m_hdr.nummat2;
	for (int i=0; i<nmat; i++)
	{
		Material m;
		fem.AddMaterial(m);
	}
}

//-----------------------------------------------------------------------------
bool FELSDYNAPlotImport::ReadMesh(FEPostModel &fem)
{
	int i, j;

	// get the mesh
	FEPostMesh* pm = new FEPostMesh;
	FEPostMesh& mesh = *pm;

	// clear the state data
	fem.ClearStates();

	// allocate storage
	mesh.Create(m_hdr.nump, m_hdr.nel8 + m_hdr.nel4 + m_hdr.nel2);

	// read the nodal coordinates
	float xyz[3];
	for (i=0; i<m_hdr.nump; i++)
	{
		FSNode& n = mesh.Node(i);

		ReadData(xyz, sizeof(float), 3);
		
		n.r.x = xyz[0];
		n.r.y = xyz[1];
		n.r.z = xyz[2];
	}
	fem.AddMesh(pm);

	int nread;
	int n[9];
	// read the element connectivity
	int ne = 0;
	int nmat = fem.Materials();
	for (i=0; i<m_hdr.nel8; i++)
	{
		FSElement& el = static_cast<FSElement&>(mesh.ElementRef(ne++));

		nread = ReadData(n, sizeof(int), 9);
		if (nread != 9)
		{
			Close();
			mesh.ClearAll();
			return errf("Error while reading element connectivity");
		}

		if ((n[7]==n[4])&&(n[6]==n[4])&&(n[5]==n[4])) 
		{
			el.SetType(FE_TET4) ;
			el.m_node[0] = n[0]-1;
			el.m_node[1] = n[1]-1;
			el.m_node[2] = n[2]-1;
			el.m_node[3] = n[4]-1;
		}
		else if (n[7]==n[6]) 
		{
			el.SetType(FE_PENTA6);
			el.m_node[0] = n[4]-1;
			el.m_node[1] = n[1]-1;
			el.m_node[2] = n[0]-1;
			el.m_node[3] = n[6]-1;
			el.m_node[4] = n[2]-1;
			el.m_node[5] = n[3]-1;
		}
		else if ((n[0]==n[4]) && (n[1]==n[5]) && (n[2]==n[6]) && (n[3]==n[7]))
		{
			el.SetType(FE_QUAD4);
			el.m_node[0] = n[0]-1;
			el.m_node[1] = n[1]-1;
			el.m_node[2] = n[2]-1;
			el.m_node[3] = n[3]-1;
		}
		else 
		{
			el.SetType(FE_HEX8);
			for (j=0; j<8; j++) el.m_node[j] = n[j] - 1;
		}

		if ((n[8] <= 0) || (n[8] > nmat))
		{
			Close();
			mesh.ClearAll();
			return errf("Invalid element definition. Data may be corrupted.");
		}

		el.m_MatID = n[8] - 1;
	}

	// read beam elements
	for (i=0; i<m_hdr.nel2; ++i)
	{
		FSElement& el = static_cast<FSElement&>(mesh.ElementRef(ne++));

		ReadData(n, sizeof(int), 6);

		el.SetType(FE_BEAM2);
		assert(n[5] > 0);

		el.m_MatID = n[5]-1;
		el.m_node[0] = n[0]-1;
		el.m_node[1] = n[1]-1;
	}

	// read shells
	for (i=0; i<m_hdr.nel4; ++i)
	{
		FSElement& el = static_cast<FSElement&>(mesh.ElementRef(ne++));

		ReadData(n, sizeof(int), 5);

		if (n[2] == n[3])
		{
			el.SetType(FE_TRI3);
		}
		else
		{
			el.SetType(FE_QUAD4);
		}

		assert(n[4] > 0);

		el.m_MatID = n[4] - 1;
		for (j = 0; j < el.Nodes(); j++) el.m_node[j] = n[j] - 1;
	}

	// set the enabled-ness of the elements and the nodes
	for (i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
		Material* pm = fem.GetMaterial(el.m_MatID);
		if (pm->benable) el.Enable(); else el.Disable();
	}

	for (i=0; i<mesh.Nodes(); ++i) mesh.Node(i).Disable();
	for (i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
		if (el.IsEnabled())
		{
			int n = el.Nodes();
			for (j=0; j<n; ++j) mesh.Node(el.m_node[j]).Enable();
		}
	}

	// update the mesh
	mesh.BuildMesh();

	// set the enabled-ness of the elements and the nodes
	for (i=0; i<mesh.Faces(); ++i)
	{
		FSFace& f = mesh.Face(i);
        assert(f.m_elem[0].eid >= 0);
		FEElement_& el = mesh.ElementRef(f.m_elem[0].eid);
	}

	fem.UpdateBoundingBox();

	return true;
}

//-----------------------------------------------------------------------------
bool FELSDYNAPlotImport::ReadStates(FEPostModel& fem)
{
	// read the timesteps
	int nnd = m_hdr.nump*(m_hdr.flagT + 3*(m_hdr.flagU + m_hdr.flagV + m_hdr.flagA));
	int n8 = m_hdr.nel8*m_hdr.nv3d;
	int n4 = m_hdr.nel4*m_hdr.nv2d;
	int n2 = m_hdr.nel2*m_hdr.nv1d;

	int datasize = 1 + m_hdr.nglbv + nnd + n8 + n4 + n2;
	int* pdata = new int [datasize];
	bool bdone = false;

	float ftime, fprev;
	int nstate = 0;

	FEState* pprev = 0;	// previously read state
	FEState* pstate = 0;

	FEPostMesh& mesh = *fem.GetFEMesh(0);

	bool bfirst = true;

	do
	{
		// read a data section
		int nread = ReadData(pdata, sizeof(int), datasize, true);
		if (nread != datasize) bdone = true;
		else
		{
			// byteswap if necessary
			ftime = *((float*) pdata);

			if (fabs(ftime + 999999) < 1e-8)
			{
				// we've reached the finale timestep
				break;
			}

			// increment state counter
			++nstate;

			// see if we already read in this time value
			// TODO: check the repeat and action logic
			//       This used to be handled by a dialog class
			if (pprev && (pprev->m_time == ftime))
			{
				if (bfirst || !m_brepeat)
				{
					bfirst = false;
					delete [] pdata;
					return false;
				}

				switch (m_naction)
				{
				case 0: // add state
					pstate = new FEState(ftime, &fem, fem.GetFEMesh(0));
					fem.AddState(pstate);
					pprev = pstate;
					break;
				case 1: // replace state
					pstate = pprev;
					break;
				case 2: // ignore state
					pstate = 0;
					break;
				default:
					assert(false);
				}
			}
			else
			{
				// create the next state and add to the FE mesh
				pstate = new FEState(ftime, &fem, fem.GetFEMesh(0));
				fem.AddState(pstate);
				pprev = pstate;
			}

			if (pstate)
			{
				fprev = ftime;

				float* pf = (float*) (pdata + 1 + m_hdr.nglbv);

				// read nodal displacements
				if (m_hdr.flagU)
				{
					// Note that LSDYNA actually stores the current nodal positions, not the displacements
					FENodeData<vec3f>& dsp = dynamic_cast<FENodeData<vec3f>&>(pstate->m_Data[m_nfield[LSDYNA_DISP]]);
					for (int i=0; i<m_hdr.nump; ++i, pf += 3)
					{
						vec3f r0 = to_vec3f(mesh.Node(i).r);
						dsp[i].x = pf[0] - r0.x;
						dsp[i].y = pf[1] - r0.y;
						dsp[i].z = pf[2] - r0.z;
					}
				}

				// read nodal velocity
				if (m_hdr.flagV)
				{
					FENodeData<vec3f>& vel = dynamic_cast<FENodeData<vec3f>&>(pstate->m_Data[m_nfield[LSDYNA_VEL]]);
					for (int i=0; i<m_hdr.nump; i++, pf += 3)
					{
						vel[i].x = pf[0];
						vel[i].y = pf[1];
						vel[i].z = pf[2];
					}
				}

				// read nodal acceleration
				if (m_hdr.flagA)
				{
					FENodeData<vec3f>& acc = dynamic_cast<FENodeData<vec3f>&>(pstate->m_Data[m_nfield[LSDYNA_ACC]]);
					for (int i=0; i<m_hdr.nump; i++, pf += 3)
					{
						acc[i].x = pf[0];
						acc[i].y = pf[1];
						acc[i].z = pf[2];
					}
				}

				// read the nodal temperatures
				if (m_hdr.flagT)
				{
					FENodeData<float>& T = dynamic_cast<FENodeData<float>&>(pstate->m_Data[m_nfield[LSDYNA_TEMP]]);
					for (int i=0; i<m_hdr.nump; ++i, ++pf) T[i] = *pf;
				}

				// load solid stress data
				if (m_hdr.flagU)
				{
					FEElementData<mat3fs,DATA_ITEM>& s  = dynamic_cast<FEElementData<mat3fs,DATA_ITEM>&>(pstate->m_Data[m_nfield[LSDYNA_STRESS  ]]);
					FEElementData<float ,DATA_ITEM>& p  = dynamic_cast<FEElementData<float ,DATA_ITEM>&>(pstate->m_Data[m_nfield[LSDYNA_PRESSURE]]);
					FEElementData<float ,DATA_ITEM>& ps = dynamic_cast<FEElementData<float ,DATA_ITEM>&>(pstate->m_Data[m_nfield[LSDYNA_PLASTIC ]]);
					for (int i=0; i<m_hdr.nel8; i++, pf += m_hdr.nv3d)
					{
						mat3fs m;
						m.x = pf[0];
						m.y = pf[1];
						m.z = pf[2];
						m.xy = pf[3];
						m.yz = pf[4];
						m.xz = pf[5];
						s.add(i, m);
						ps.add(i, pf[6]);
						p.add(i, -m.tr()/3.f);
					}

					// load beam stress data
					for (int i=0; i<m_hdr.nel2; ++i, pf += m_hdr.nv1d)
					{
						s.add(i + m_hdr.nel8, mat3fs(pf[0], 0, 0, 0, 0, 0));
					}

					// load shell stress data
					ELEMDATA* pe = &pstate->m_ELEM[0] + (m_hdr.nel8 + m_hdr.nel2);
					for (int i=0; i<m_hdr.nel4; i++, pf += m_hdr.nv2d)
					{
						int n = i + m_hdr.nel8 + m_hdr.nel2;
						mat3fs m(pf[0], pf[1], pf[2], pf[3], pf[4], pf[5]);
						s.add(n, m);
						ps.add(n, pf[6]);
						p.add(n, -m.tr()/3.f);
						pe[i].m_h[0] = pf[29];
						pe[i].m_h[1] = pf[29];
						pe[i].m_h[2] = pf[29];
						pe[i].m_h[3] = pf[29];

						if (m_hdr.nv2d == 44)
						{
							FEElementData<mat3fs,DATA_ITEM>& E  = dynamic_cast<FEElementData<mat3fs,DATA_ITEM>&>(pstate->m_Data[m_nfield[LSDYNA_SHELL_STRAIN]]);
							mat3fs m;
							m.x = 0.5f*(pf[32] + pf[38]);
							m.y = 0.5f*(pf[33] + pf[39]);
							m.z = 0.5f*(pf[34] + pf[40]);
							m.xy = 0.5f*(pf[35] + pf[41]);
							m.yz = 0.5f*(pf[36] + pf[42]);
							m.xz = 0.5f*(pf[37] + pf[43]);
							E.add(n, m);
						}
					}
				}
			}
		}
	}
	while(!bdone);

	delete [] pdata;

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool FELSDYNAPlotExport::Save(FEPostModel& fem, const char* szfile, bool bflag[6], int ncode[6])
{
	int i, j, N;

	// create the file
	FILE* fp = fopen(szfile, "wb");
	if (fp == 0) return false;

	// write the header
	PLOTHEADER plh = {0};

	// copy the title
	// note that we only have 40 characters to store the title
	string title = fem.GetTitle();
	const char* sztitle = title.c_str();
	if (strlen(sztitle) > 39)
		strncpy(plh.Title, sztitle, 39);
	else
		strcpy(plh.Title, sztitle);

	FEPostMesh& mesh = *fem.GetFEMesh(0);

	plh.neips = 2000;
	plh.flagU = (bflag[2]?1:0);
	plh.flagV = (bflag[3]?1:0);
	plh.flagA = (bflag[4]?1:0);
	plh.flagT = (bflag[5]?1:0);
	plh.icode = 6;
	plh.ndim  = 4;
	plh.nel2  = mesh.BeamElements();
	plh.nel4  = mesh.ShellElements();
	plh.nel8  = mesh.SolidElements();
	plh.nglbv = 0;
	plh.nummat2 = 0;
	plh.nummat4 = 0;
	plh.nummat8 = fem.Materials();
	plh.nump    = mesh.Nodes();
	plh.nv1d    = 6;
	plh.nv2d    = 32;
	plh.nv3d    = 7;
	plh.maxint  = 0;
	plh.neips   = 2000;

	fwrite(&plh, sizeof(PLOTHEADER), 1, fp);

	// write the material coordinates
	float xf[3];
	for (i=0; i<mesh.Nodes(); ++i)
	{
		FSNode& node = mesh.Node(i);

		xf[0] = (float) node.r.x;
		xf[1] = (float) node.r.y;
		xf[2] = (float) node.r.z;

		fwrite(xf, sizeof(float), 3, fp);
	}

	// write the connectivity and material number
	// Note that we increment all numbers by 1 since
	// the plot database expects 1-based arrays
	int n[9];

	// write solid connectivity data
	// note that we reindex all elements so that the ID
	// corresponds to the nr in the plot file
	for (i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
		if (el.IsSolid())
		{
			N = el.Nodes();
			switch (el.Type())
			{
			case FE_HEX8:
				for (j=0; j<N; ++j) n[j] = el.m_node[j]+1;
				break;
			case FE_PENTA6:
				n[0] = el.m_node[2]+1;
				n[1] = el.m_node[1]+1;
				n[2] = el.m_node[4]+1;
				n[3] = el.m_node[5]+1;
				n[4] = el.m_node[0]+1;
				n[5] = el.m_node[0]+1;
				n[6] = el.m_node[3]+1;
				n[7] = el.m_node[3]+1;
				break;
			case FE_TET4:
				n[0] = el.m_node[0]+1;
				n[1] = el.m_node[1]+1;
				n[2] = el.m_node[2]+1;
				n[3] = el.m_node[2]+1;
				n[4] = n[5] = n[6] = n[7] = el.m_node[3]+1;
				break;
			}
			
			n[8] = el.m_MatID+1;
		
			fwrite(n, sizeof(int), 9, fp);
		}
	}

	// write beam connectivity data
	for (i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
		if (el.IsBeam())
		{
			N = el.Nodes();
			assert(N==2);
			n[0] = el.m_node[0]+1;
			n[1] = el.m_node[1]+1;
			n[5] = el.m_MatID+1;
			fwrite(n, sizeof(int), 6, fp);
		}
	}

	// write shell connectivity data
	for (i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
		if (el.IsShell())
		{
			N = el.Nodes();
			switch (el.Type())
			{
			case FE_QUAD4:
				n[0] = el.m_node[0]+1;
				n[1] = el.m_node[1]+1;
				n[2] = el.m_node[2]+1;
				n[3] = el.m_node[3]+1;
				break;
			case FE_TRI3:
				n[0] = el.m_node[0]+1;
				n[1] = el.m_node[1]+1;
				n[2] = el.m_node[2]+1;
				n[3] = n[2];
				break;
			}	
			
			n[4] = el.m_MatID+1;
		
			fwrite(n, sizeof(int), 5, fp);
		}
	}

	// write the states to the file
	// TODO: for now we write nothing. we need to figure out
	//       a way to let the user decide what to write to the plotfile
	for (int l=0; l<fem.GetStates(); ++l)
	{
		FEState* ps = fem.GetState(l);
	
		// write the time value
		float time = ps->m_time;
		fwrite(&time, sizeof(float), 1, fp);

		// write the spatial coordinates
		if (plh.flagU)
		{
			float xf[3];
			for (i=0; i<mesh.Nodes(); ++i)
			{
				FSNode& node = mesh.Node(i);
				vec3f r = fem.EvaluateNodeVector(i, l, ncode[2]);

				// since the PLOT file requires floats we need to convert
				// the doubles to single precision
				xf[0] = node.r.x + r.x;
				xf[1] = node.r.y + r.y;
				xf[2] = node.r.z + r.z;

				fwrite(xf, sizeof(float), 3, fp);
			}
		}	

		// write the velocities
		if (plh.flagV)
		{
			float vf[3];
			for (i=0; i<mesh.Nodes(); ++i)
			{
				vec3f r = fem.EvaluateNodeVector(i, l, ncode[3]);

				vf[0] = r.x;
				vf[1] = r.y;
				vf[2] = r.z;

				fwrite(vf, sizeof(float), 3, fp);
			}
		}

		// write the accelerations
		if (plh.flagA)
		{
			float af[3];
			for (i=0; i<mesh.Nodes(); ++i)
			{
				vec3f r = fem.EvaluateNodeVector(i, l, ncode[4]);

				af[0] = r.x;
				af[1] = r.y;
				af[2] = r.z;

				fwrite(af, sizeof(float), 3, fp);
			}
		}

		// write the "temperatures"
		if (plh.flagT)
		{
			NODEDATA n;
			for (i=0; i<mesh.Nodes(); ++i)
			{
				fem.EvaluateNode(i, l, ncode[5], n);
				fwrite(&n.m_val, sizeof(float), 1, fp);
			}
		}

		// write solid element data
		float s[32] = {0};
		float data[FSElement::MAX_NODES] = {0.f}, val;
		for (i=0; i<mesh.Elements(); ++i)
		{
			FEElement_& el = mesh.ElementRef(i);
			if (el.IsSolid())
			{
				if (bflag[0])
				{
					mat3f m = fem.EvaluateElemTensor(i, l, ncode[0], DATA_MAT3FS);
					mat3fs a = m.sym();
					s[ 0] = a.x;
					s[ 1] = a.y;
					s[ 2] = a.z;
					s[ 3] = a.xy;
					s[ 4] = a.yz;
					s[ 5] = a.xz;
				}
				if (bflag[1]) 
				{
					fem.EvaluateElement(i, l, ncode[1], data, val);
					s[6] = val;
				}

				fwrite(s, sizeof(float), 7, fp);
			}
		}

		// write beam stress data
		s[0] = s[1] = s[2] = s[3] = s[4] = s[5];
		for (i=0; i<mesh.Elements(); ++i)
		{
			FEElement_& el = mesh.ElementRef(i);
			if (el.IsBeam())
			{
				fwrite(s, sizeof(float), 6, fp);
			}
		}

		// write shell element data
		for (i=0; i<mesh.Elements(); ++i)
		{
			FEElement_& el = mesh.ElementRef(i);
			if (el.IsShell())
			{
				if (bflag[0])
				{
					// mid-surface stresses
					mat3f m = fem.EvaluateElemTensor(i, l, ncode[0], DATA_MAT3FS);
					mat3fs a = m.sym();
					s[ 0] = a.x;
					s[ 1] = a.y;
					s[ 2] = a.z;
					s[ 3] = a.xy;
					s[ 4] = a.yz;
					s[ 5] = a.xz;
				}

				// inner surface stresses
				s[ 7] = s[0];
				s[ 8] = s[1];
				s[ 9] = s[2];
				s[10] = s[3];
				s[11] = s[4];
				s[12] = s[5];

				// outer surface stresses
				s[14] = s[0];
				s[15] = s[1];
				s[16] = s[2];
				s[17] = s[3];
				s[18] = s[4];
				s[19] = s[5];

				// shell thicknesses
				float* h = ps->m_ELEM[i].m_h;
				s[29] += 0.25f*(h[0] + h[1] + h[2] + h[3]);

				fwrite(s, sizeof(float), 32, fp);
			}
		}
	}

	fclose(fp);

	return true;
}
