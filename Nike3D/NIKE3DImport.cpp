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

// NikeImport.cpp: implementation of the NIKE3DImport class.
//
//////////////////////////////////////////////////////////////////////

#include "NIKE3DImport.h"
#include <FEMLib/FERigidConstraint.h>
#include <GeomLib/GMeshObject.h>
#include <FEMLib/FEMaterial.h>
#include <FEMLib/GDiscreteObject.h>
#include <GeomLib/GModel.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEBioLink/FEBioModule.h>
#include <string.h>
#include <sstream>

#define ABS(x) ((x)>=0?(x):(-(x)))

extern const int COLORS;
extern GLColor col[];

// error constants
enum NIKE_ERR {
	ERR_EOF,
	ERR_CC,
	ERR_FORMAT,
	ERR_ATYPE,
	ERR_MAT,
	ERR_AOPT,
	ERR_NODE,
	ERR_HEX,
	ERR_SHELL,
	ERR_SI,
	ERR_LC,
	ERR_CFORCE,
	ERR_PRESS,
	ERR_DISP,
	ERR_BFORCE,
	ERR_VEL,
	ERR_DEC
};

// error message formats
const char szerr[][256] = {
	"FATAL ERROR: Unexpected end of file in input file %s\n\n",							// 0
	"FATAL ERROR: Error reading control card %d\n\n",									// 1
	"FATAL ERROR: Incorrect input format. Must be FL\n\n",								// 2
	"FATAL ERROR: Unrecognized analysis type on CC 7\n\n"								// 3
	"FATAL ERROR: Error in material card %d of material %d\n\n",						// 4
	"FATAL ERROR: Invalid value for material axis option on CC6 of material %d\n\n",	// 5
	"FATAL ERROR: Error in nodal coordinates deck at node %d\n\n",						// 6
	"FATAL ERROR: Error in hexahedral deck at element %d\n\n",							// 7
	"FATAL ERROR: Error in shell element deck at element %d\n\n",						// 8
	"FATAL ERROR: Error encounted reading sliding surface deck\n\n",					// 9
	"FATAL ERROR: Error in load curve deck of load curve %d\n\n",						// 10
	"FATAL ERROR: Error in concentrated nodal force deck\n\n",							// 11
	"FATAL ERROR: Error in pressure boundary condition deck\n\n",						// 12
	"FATAL ERROR: Invalid boundary condition in displacement deck\n\n"					// 13
	"FATAL ERROR: Error in base acceleration body force deck\n\n",						// 14
	"FATAL ERROR: Error in initial velocity deck\n\n",									// 15
	"FATAL ERROR: Error in discrete element control card\n\n"							// 16
};

//-----------------------------------------------------------------------------
// Constructor
//
NIKE3DImport::NIKE3DImport(FSProject& prj) : FSFileImport(prj)
{
}

//-----------------------------------------------------------------------------
// This function reads a line from a text file, while skipping over comment
// lines (ie. lines which begin with an asterisk (*))
// The function returns 0 if an end-of-file or an error is encountered.
// Otherwise it returns szline;
//

char* NIKE3DImport::read_line(FILE* fp, char* szline, int n, bool bskiptxt)
{
	// read a line while skipping over comment lines
	while (fgets(szline, n, fp) && ((szline[0]=='*') || ((bskiptxt) && isalpha(szline[0]))));

	// make sure we did not encounter a problem
	if (feof(fp) || ferror(fp)) return NULL;

	// remove eol
	char* ch = strrchr(szline, '\n');
	if (ch) *ch = 0;

	return szline;
}

//-----------------------------------------------------------------------------
// Imports a NIKE input deck
//

bool NIKE3DImport::Load(const char* szfile)
{
	m_nmat = 0;

	// store a pointer to the project
	m_po = 0;
	m_fem = &m_prj.GetFSModel();

	// get the model
	FSModel& fem = *m_fem;

	// set the project's module
	m_prj.SetModule(FEBio::GetModuleId("solid"));

	// create a nike project
	NIKE3DProject nike;

	// open the file
	if (Open(szfile, "rt") == 0) return errf("FATAL ERROR: Failed opening input file %s\n\n", szfile);

	// read data
	if (ReadControlDeck     (nike) == false) return false;
	if (ReadMaterialDeck    (nike) == false) return false;
	if (ReadNodes           (nike) == false) return false;
	if (ReadBricks          (nike) == false) return false;
	if (ReadShells          (nike) == false) return false;
	if (ReadRigidFacets     (nike) == false) return false;
	if (ReadDiscreteElements(nike) == false) return false;
	if (ReadInterfaces      (nike) == false) return false;
	if (ReadLoadCurves      (nike) == false) return false;
	if (ReadNodalForces     (nike) == false) return false;
	if (ReadPressureFacets  (nike) == false) return false;
	if (ReadDisplacements   (nike) == false) return false;
	if (ReadBodyForce       (nike) == false) return false;
	if (ReadVelocities      (nike) == false) return false;
	 
	// close the input file
	Close();

	// build the model
	build_model(nike);

	// if we get here we are good to go!
	UpdateFEModel(fem);

	// convert to new structure
	std::stringstream log;
	m_prj.ConvertToNewFormat(log);

	// all done!
	return true;
}

//-----------------------------------------------------------------------------

void NIKE3DImport::build_model(NIKE3DProject& nike)
{
	build_loadcurves(nike);
	build_control(nike);
	build_materials(nike);
	build_mesh(nike);
	build_constraints(nike);
	build_rigidfacets(nike);
	build_discrete(nike);
	build_interfaces(nike);
}

//-----------------------------------------------------------------------------

void NIKE3DImport::build_control(NIKE3DProject& nike)
{
	NIKE3DProject::CONTROL& c = nike.m_Ctrl;
	FSModel& fem = *m_fem;
	FSNonLinearMechanics* pstep = new FSNonLinearMechanics(&fem);
	pstep->SetName("Step01");
	fem.AddStep(pstep);

	STEP_SETTINGS& ops = pstep->GetSettings();

	// set the mustpoint loadcurve
	if (c.dtmax < 0)
	{
		m_nmplc = (int) (-c.dtmax);
		--m_nmplc;
	}
	else m_nmplc = -1;

	// set the control settings
	ops.bauto = (c.nauto == 1);
	ops.dt = c.dt;
	ops.dtmin = c.dtmin;
	ops.dtmax = (c.dtmax >= 0? c.dtmax : 0);
	pstep->SetDisplacementTolerance(c.dtol);
	pstep->SetEnergyTolerance(c.ectl);
	pstep->SetResidualTolerance(c.rctl);
	pstep->SetLineSearchTolerance(c.tolls);
	ops.ilimit = c.ilimit;
	ops.iteopt = c.iteopt;
	ops.maxref = c.maxref;
	ops.mxback = c.mxback;
	ops.ntime = c.ntime;
}

//-----------------------------------------------------------------------------

void NIKE3DImport::build_mesh(NIKE3DProject &nike)
{
	int i, j;

	// get the model
	FSModel& fem = *m_fem;

	int nodes = (int)nike.m_Node.size();
	int nhel  = (int)nike.m_Brick.size();
	int nsel  = (int)nike.m_Shell.size();

	if ((nodes == 0) || (nhel + nsel == 0)) return;

	// create storage for the mesh
	FSMesh* pm = new FSMesh();
	pm->Create(nodes, nhel + nsel);

	// copy nodes
	for (i=0; i<nodes; ++i)
	{
		FSNode& n = pm->Node(i);
		NIKE3DProject::NODE& N = nike.m_Node[i];

		n.r.x = N.x;
		n.r.y = N.y;
		n.r.z = N.z;
	}

	// copy elements
	int nmat;
	for (i=0; i<nhel; ++i)
	{
		FSElement& el = pm->Element(i);
		NIKE3DProject::BRICK& E = nike.m_Brick[i];
		nmat = E.nmat - 1;

		// TODO: what do we do if nmat < -1 ?
		// we'll create a new part for each material
		if (nmat >= 0) el.m_gid = nmat;

		if ((E.n[7] == E.n[6]) && (E.n[6] == E.n[5]) && (E.n[5] == E.n[4]) && (E.n[4] == E.n[3]))
		{
			el.SetType(FE_TET4);
			el.m_node[0] = E.n[0]-1;
			el.m_node[1] = E.n[1]-1;
			el.m_node[2] = E.n[2]-1;
			el.m_node[3] = E.n[3]-1;
		}
		else if ((E.n[7] == E.n[6]) && (E.n[5] == E.n[4]))
		{
			el.SetType(FE_PENTA6);
			el.m_node[0] = E.n[4]-1;
			el.m_node[1] = E.n[1]-1;
			el.m_node[2] = E.n[0]-1;
			el.m_node[3] = E.n[6]-1;
			el.m_node[4] = E.n[2]-1;
			el.m_node[5] = E.n[3]-1;
		}
		else 
		{
			el.SetType(FE_HEX8);
			for (j=0; j<8; ++j) el.m_node[j] = E.n[j]-1;
		}
	}

	// copy elements
	for (i=0; i<nsel; ++i)
	{
		FSElement& el = pm->Element(i + nhel);
		NIKE3DProject::SHELL& S = nike.m_Shell[i];

		nmat = S.nmat - 1;

		// TODO: what do we do if nmat < -1 ?
		// we'll create a new part for each material
		if (nmat >= 0) el.m_gid = nmat;

		if (S.n[3] == S.n[2])
		{
			el.SetType(FE_TRI3);
			el.m_node[0] = S.n[0]-1;
			el.m_node[1] = S.n[1]-1;
			el.m_node[2] = S.n[2]-1;
		}
		else 
		{
			el.SetType(FE_QUAD4);
			for (j=0; j<4; ++j) el.m_node[j] = S.n[j]-1;
		}

		// copy shell thickness
		for (j=0; j<4; ++j) el.m_h[j] = S.h[j];
	}

	// create a new object from this mesh
	pm->RebuildMesh();
	m_po = new GMeshObject(pm);

	// assign the materials to the parts
	for (int i=0; i<pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		assert((el.m_gid >= 0)&&(el.m_gid < m_po->Parts()));
		GPart* pg = m_po->Part(el.m_gid);
		pg->SetMaterialID(fem.GetMaterial(el.m_gid)->GetID());
	}

	UpdateMesh(*pm);
}

//-----------------------------------------------------------------------------

void NIKE3DImport::build_constraints(NIKE3DProject& nike)
{
	// get the model
	FSModel& fem = *m_fem;
	FSMesh* pm = m_po->GetFEMesh();
	FSStep& s = *fem.GetStep(0);

	int i;
	int N = (int)nike.m_Node.size();
	vector<int> BC; BC.resize(N);
	vector<int> RBC; RBC.resize(N);
	for (i=0; i<N; ++i) 
	{
		NIKE3DProject::NODE& n = nike.m_Node[i];
		BC[i] = n.bc;  //displacement boundary conditions
		RBC[i] = n.rc; //rotation boundary conditions
	}

	char szname[256];

	// fixed displacement and rotaional boundary conditions
	int nbc = 0;
	int nfc = 1;
	for(int k =0;k<2;k++)
	{
		do
		{
			// find a non-zero bc
			nbc = 0;
			if(k==0)
			    {for (i=0; i<N; ++i) if (BC[i] != 0) { nbc = BC[i]; break; }}
			else
				{for (i=0; i<N; ++i) if (RBC[i] != 0) { nbc = RBC[i]; break; }}

			if (nbc)
			{
				// create a nodeset of this BC
				FSNodeSet* pg = new FSNodeSet(m_po);
				sprintf(szname, "FixedNodeset%02d", nfc++);
				pg->SetName(szname);
				m_po->AddFENodeSet(pg);

				// count all nodes that have this BC
				// assign the nodes to this group
				int nfix = 0;
				for (i=0; i<N; ++i)
				{
					if (k==0 && BC[i] == nbc) 
					{
						++nfix;
						pg->add(i); 
						BC[i] = 0;
					}
					else if(k == 1 && RBC[i] == nbc)
					{
						++nfix;
						pg->add(i); 
						RBC[i] = 0;
					}
				}

				// create the constraint
				int bc = 0;
				const int BCT[8] = {0, 8, 16, 32, 24, 48, 40, 56};
				if(k == 0)
					bc = nbc;
				else
					bc = BCT[nbc];
				
				if (bc < 8)
				{
					FSFixedDisplacement* pbc = new FSFixedDisplacement(&fem, pg, bc);
					sprintf(szname, "FixedConstraint%02d", nfc-1);
					pbc->SetName(szname);
					pbc->SetBC(bc);
					s.AddComponent(pbc);
				}
				else
				{
					bc = bc >> 3;
					FSFixedRotation* prc = new FSFixedRotation(&fem, pg, bc);
					sprintf(szname, "FixedConstraint%02d", nfc-1);
					prc->SetName(szname);
					prc->SetBC(bc);
					s.AddComponent(prc);
				}
			}
		}
		while (nbc);
	}
	// prescribe boundary conditions
	N = (int) nike.m_DC.size();
	BC.resize(N);
	list<NIKE3DProject::NODAL_DISPLACEMENT>::iterator pn = nike.m_DC.begin();
	for (i=0; i<N; ++i, ++pn) BC[i] = pn->bc; 
	nbc = 0;
	nfc = 1;
	int nlc = 0;
	do
	{
		// find a non-zero bc
		nbc = 0;
		pn = nike.m_DC.begin();
		for (i=0; i<N; ++i, ++pn) if (BC[i] != 0) { nbc = BC[i]; nlc = pn->lc; break; }

		if (nbc)
		{
			// count all nodes that have this BC
			int nfix = 0;
			pn = nike.m_DC.begin();
			for (i=0; i<N; ++i, ++pn) if ((BC[i] == nbc) && (pn->lc == nlc)) ++nfix;

			// create a nodeset of this BC
			FSNodeSet* pg = new FSNodeSet(m_po);
			sprintf(szname, "DisplacementNodeset%02d", nfc);
			pg->SetName(szname);
			m_po->AddFENodeSet(pg);

			// assign the nodes to this group
			pn = nike.m_DC.begin();
			double scale = pn->s;
			for (i=0; i<N; ++i, ++pn) if ((BC[i] == nbc) && (pn->lc == nlc)) { pg->add(pn->node - 1); BC[i] = 0; }
			
			// create the constraint
			FSPrescribedDisplacement* pbc = new FSPrescribedDisplacement(&fem, pg, nbc-1, scale);
			sprintf(szname, "PrescribedConstraint%02d", nfc++);
			pbc->SetName(szname);
			pbc->SetDOF(nbc-1);
//			LoadCurve* plc = pbc->GetLoadCurve();
//			*plc = m_LC[nlc-1];
			s.AddComponent(pbc);
		}
	}
	while (nbc);

	// build the rigid constraints
	// We need to add rigid constraints for each rigid material. 
	int NMAT = (int)nike.m_Mat.size();
	int nrc = 1;
	for (int i=0; i<NMAT; ++i)
	{
		NIKE3DProject::MATERIAL& mat = nike.m_Mat[i];
		if (mat.ntype==20)
		{
			// get the constraints
			int BC[6], nfix = 0, npre = 0;
			BC[0] = (int) mat.m[2][0]; if (BC[0] == -1) nfix++; if (BC[0] > 0) npre++;
			BC[1] = (int) mat.m[2][1]; if (BC[1] == -1) nfix++; if (BC[1] > 0) npre++;
			BC[2] = (int) mat.m[2][2]; if (BC[2] == -1) nfix++; if (BC[2] > 0) npre++;
			BC[3] = (int) mat.m[2][3]; if (BC[3] == -1) nfix++; if (BC[3] > 0) npre++;
			BC[4] = (int) mat.m[2][4]; if (BC[4] == -1) nfix++; if (BC[4] > 0) npre++;
			BC[5] = (int) mat.m[2][5]; if (BC[5] == -1) nfix++; if (BC[5] > 0) npre++;
			
			// see if any are fixed
			if (nfix > 0)
			{
				sprintf(szname, "RigidConstraint%d", nrc++);
				FSRigidFixed* prc = new FSRigidFixed(0);
				prc->SetName(szname);
				prc->SetMaterialID(fem.GetMaterial(i)->GetID());
				prc->SetDOF(0, (BC[0] == -1));
				prc->SetDOF(1, (BC[1] == -1));
				prc->SetDOF(2, (BC[2] == -1));
				prc->SetDOF(3, (BC[3] == -1));
				prc->SetDOF(4, (BC[4] == -1));
				prc->SetDOF(5, (BC[5] == -1));
				s.AddRC(prc);
			}

			// see if any are prescribed
			if (npre > 0)
			{
				for (int j=0; j<6; ++j)
				{
					if (BC[j] > 0)
					{
						sprintf(szname, "RigidConstraint%d", nrc++);
						FSRigidDisplacement* prc = new FSRigidDisplacement(0);
						prc->SetName(szname);
						prc->SetMaterialID(fem.GetMaterial(i)->GetID());

						prc->SetDOF(j);
						prc->SetValue(1.0);
	//					prc->SetLoadCurve(m_LC[BC[j]-1]);
						s.AddRC(prc);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------

void NIKE3DImport::build_materials(NIKE3DProject& nike)
{
	int i;

	FSModel& fem = *m_fem;
	FSStep& step = *fem.GetStep(0);

	// assign materials to elements
	m_nmat = (int)nike.m_Mat.size();
	for (i=0; i<m_nmat; ++i)
	{
		NIKE3DProject::MATERIAL& m = nike.m_Mat[i];

		FSMaterial* pmat = 0;

		switch (m.ntype)
		{
		case 1:		// isotropic elastic
			{
				FSIsotropicElastic* pm = new FSIsotropicElastic(&fem);
				pmat = pm;

				pm->SetFloatValue(FSIsotropicElastic::MP_E, m.m[0][0]);
				pm->SetFloatValue(FSIsotropicElastic::MP_v, m.m[1][0]);
				pm->SetFloatValue(FSIsotropicElastic::MP_DENSITY, m.dens);
			}
			break;
		case 15:	// mooney-rivlin
			{
				FSMooneyRivlin* pm = new FSMooneyRivlin(&fem);
				pmat = pm;

				double A = m.m[0][0];
				double B = m.m[1][0];
				double v = m.m[2][0];
				double K = 4*(A + B)*(1 + v)/(3*(1-2*v));

				pm->SetFloatValue(FSMooneyRivlin::MP_A, A);
				pm->SetFloatValue(FSMooneyRivlin::MP_B, B);
				pm->SetFloatValue(FSMooneyRivlin::MP_K, K);
				pm->SetFloatValue(FSMooneyRivlin::MP_DENSITY, m.dens);
			}
			break;
		case 20:
			{
				FSRigidMaterial* pm = new FSRigidMaterial(&fem);
				pmat = pm;
				pm->SetFloatValue(FSRigidMaterial::MP_DENSITY, m.dens);
				pm->SetFloatValue(FSRigidMaterial::MP_E, m.m[0][0]);
				pm->SetFloatValue(FSRigidMaterial::MP_V, m.m[1][0]);

/*				FSRigidConstraint* pc = 0;
				FSRigidConstraint* pd = 0;

				for (j=0; j<6; ++j)
				{
					int lc = (int) m.m[2][j];
					if (lc == -1)
					{
						if (pc == 0) pc = new FSRigidConstraint(FE_RIGID_FIXED, step.GetID());
						pc->m_BC[j] = 1;
					}
					else if (lc > 0)
					{
						if (pd == 0) pd = new FSRigidConstraint(FE_RIGID_PRESCRIBED, step.GetID());
						pd->m_BC[j] = 1;
						pd->m_LC[j].SetID(lc-1);
						pd->m_val[j] = 1;
					}
				}
*/
				bool bcom = (m.m[3][0] == 0);
				pm->SetBoolValue(FSRigidMaterial::MP_COM, bcom);
				pm->SetVecValue(FSRigidMaterial::MP_RC, vec3d(m.m[3][1], m.m[3][2], m.m[3][3]));
			}
			break;
		case 18:
			{
				// first we need to figure out which flavor of material 18 this is
				// currently, we only check for the EFD material (random-fiber MR)
				// and if this is not the case, we assume it's the normal material 18
				// that is, the trans-iso MR.
				double beta[3], ksi[3];
				ksi [0] = m.m[3][3]; ksi [1] = m.m[5][3]; ksi [2] = m.m[5][5];
				beta[0] = m.m[3][4]; beta[1] = m.m[5][4]; beta[2] = m.m[5][6];

				if ((beta[0]>=2) && (beta[1] >= 2) && (beta[2] >= 2))
				{
					// it's the EFD material
					FSEFDMooneyRivlin* pm = new FSEFDMooneyRivlin(&fem);
					pmat = pm;
					pm->SetFloatValue(FSEFDMooneyRivlin::MP_C1, m.m[0][0]);
					pm->SetFloatValue(FSEFDMooneyRivlin::MP_C2, m.m[0][1]);
					pm->SetFloatValue(FSEFDMooneyRivlin::MP_K , m.m[1][0]);
					pm->SetVecValue(FSEFDMooneyRivlin::MP_BETA, vec3d(beta[0], beta[1], beta[2]));
					pm->SetVecValue(FSEFDMooneyRivlin::MP_KSI , vec3d(ksi [0], ksi [1], ksi [2]));
				}
				else
				{
					// assume normal trans-iso MR
					FSTransMooneyRivlin* pm = new FSTransMooneyRivlin(&fem);
					pmat = pm;
					pm->SetFloatValue(FSTransMooneyRivlin::MP_DENSITY, m.dens);
					pm->SetFloatValue(FSTransMooneyRivlin::MP_C1, m.m[0][0]);
					pm->SetFloatValue(FSTransMooneyRivlin::MP_C2, m.m[0][1]);
					pm->SetFloatValue(FSTransMooneyRivlin::MP_C3, m.m[0][2]);
					pm->SetFloatValue(FSTransMooneyRivlin::MP_C4, m.m[0][3]);
					pm->SetFloatValue(FSTransMooneyRivlin::MP_C5, m.m[0][4]);
					pm->SetFloatValue(FSTransMooneyRivlin::MP_LAM, m.m[1][1]);
					pm->SetFloatValue(FSTransMooneyRivlin::MP_K, m.m[1][0]);

					// read fiber parameters
					FSOldFiberMaterial* pf = pm->GetFiberMaterial();
					int naopt = (int) m.m[3][0];
					switch (naopt)
					{
					case 0: pf->m_naopt = FE_FIBER_LOCAL; pf->m_n[0] = (int) m.m[4][0]; pf->m_n[1] = (int) m.m[4][1]; break;
					case 1: pf->m_naopt = FE_FIBER_SPHERICAL; pf->m_r = vec3d(m.m[4][0], m.m[4][1], m.m[4][2]); pf->m_d = vec3d(1,0,0); break;
					case 2:
						pf->m_naopt = FE_FIBER_VECTOR;
						pf->m_a = vec3d(m.m[4][0], m.m[4][1], m.m[4][2]);
						pf->m_d = vec3d(m.m[5][0], m.m[5][1], m.m[5][2]);
						break;
					}
				}
			}
			break;
		default:
			{
				pmat = new FSIsotropicElastic(&fem);
/*				FENIKEMaterial* pm = new FENIKEMaterial();
				pmat = pm;
				pm->SetIntValue(FENIKEMaterial::MP_TYPE, m.ntype);
				pm->SetFloatValue(FENIKEMaterial::MP_DENSITY, m.dens);
				pm->SetIntValue(FENIKEMaterial::MP_ELEM, m.nelem);
				pm->SetFloatValue(FENIKEMaterial::MP_TREF, m.Tref);
				pm->SetFloatValue(FENIKEMaterial::MP_RDA, m.rda);
				pm->SetFloatValue(FENIKEMaterial::MP_RDB, m.rdb);
				pm->SetFloatValue(FENIKEMaterial::MP_HRGL, m.hrgl);
				pm->SetFloatValue(FENIKEMaterial::MP_FORM, m.flag);
				memcpy(pm->m_d, m.m, sizeof(double)*64);
*/
			}
		}

		GMaterial* pgm = new GMaterial(pmat);

		pgm->SetName(m.szname);
		pgm->AmbientDiffuse(col[i%16]);
		m_pMat.push_back(pgm);

		// add materials to model
		fem.AddMaterial(pgm);
	}
}

//-----------------------------------------------------------------------------

void NIKE3DImport::build_rigidfacets(NIKE3DProject& nike)
{
	list<NIKE3DProject::RIGID_FACET>::iterator pf = nike.m_Rigid.begin();

	FSModel* ps = m_fem;
	FSMesh* pm = m_po->GetFEMesh();

	int nrns = 1;
	char szname[256] = {0};

	while (pf != nike.m_Rigid.end())
	{
		FSNodeSet* pn = new FSNodeSet(m_po);
		sprintf(szname, "RigidNodeset%2d", nrns);
		pn->SetName(szname);
		m_po->AddFENodeSet(pn);

		int nrb = pf->nrb;
		GMaterial* pgm = m_pMat[nrb-1];
		FSMaterial* pmat = pgm->GetMaterialProperties();
		if (dynamic_cast<FSRigidMaterial*>(pmat) == 0) { delete pn; pn = 0; }

		do
		{
			if (pn) for (int i=0; i<pf->nsize; ++i) pn->add(pf->node[i] - 1);
			++pf;
		}
		while ((pf != nike.m_Rigid.end()) && (pf->nrb == nrb));

		// create the interface
		if (pn)
		{
			FSRigidInterface* pi = new FSRigidInterface(ps, pgm, pn, ps->GetStep(0)->GetID());
			sprintf(szname, "RigidInterface%02d", nrns);
			pi->SetName(szname);
			ps->GetStep(0)->AddComponent(pi);
			++nrns;
		}
	}
}

//-----------------------------------------------------------------------------

void NIKE3DImport::build_discrete(NIKE3DProject& nike)
{
	GModel& model = m_fem->GetModel();
	GMeshObject* po = dynamic_cast<GMeshObject*>(m_po);

	char szname[256];

	for (int i=0; i< (int) nike.m_DSP.size(); ++i)
	{
		NIKE3DProject::DISCRETE_SPRING& s = nike.m_DSP[i];
		int nmat = s.nmat - 1;
		if ((nmat >= 0) && (nmat < (int) nike.m_DMA.size()))
		{
			NIKE3DProject::DISCRETE_MATERIAL& m = nike.m_DMA[ nmat ];
			if (m.ntype == 1)
			{
				int n1 = s.n1 - 1;
				int n2 = s.n2 - 1;
				double E = m.m[0];

				// the nodes are not GNodes yet, so we upgrade them to GNode
				n1 = po->MakeGNode(n1);
				n2 = po->MakeGNode(n2);

				// make a discrete spring
				GLinearSpring* ps = new GLinearSpring(&model, n1, n2);

				sprintf(szname, "Spring%02d", i+1);
				ps->SetName(szname);
				ps->GetParam(GLinearSpring::MP_E).SetFloatValue(E);

				// add it to the model
				model.AddDiscreteObject(ps);
			}
		}
	}
}

//-----------------------------------------------------------------------------

void NIKE3DImport::build_interfaces(NIKE3DProject& nike)
{
	int i, j;

	FSModel* ps = m_fem;
	FSMesh* pm = m_po->GetFEMesh();
	char szname[256];

	list<NIKE3DProject::SI_FACET>::iterator pf = nike.m_Face.begin();

	for (i=0; i<(int) nike.m_SI.size(); ++i)
	{
		NIKE3DProject::SLIDING_INTERFACE& si = nike.m_SI[i];

		// create the slave surface
		FSSurface* pss = new FSSurface(m_po);
		for (j=0; j<si.nns; ++j, ++pf) pss->add(FindFace(pf->n, 1));
		sprintf(szname, "SlaveSurface%02d", i+1);
		pss->SetName(szname);

		// create the master surface
		FSSurface* pms = new FSSurface(m_po);
		for (j=0; j<si.nms; ++j, ++pf) pms->add(FindFace(pf->n, 1));
		sprintf(szname, "MasterSurface%02d", i+1);
		pms->SetName(szname);

		// create the contact interface
		switch (si.itype)
		{
		case 2: // tied contact
			{
				FSTiedInterface* pi = new FSTiedInterface(ps);
				sprintf(szname, "TiedInterface%02d", i+1);
				pi->SetName(szname);
				pi->SetSecondarySurface(pms);
				pi->SetPrimarySurface(pss);
				ps->GetStep(0)->AddComponent(pi);

				pi->SetFloatValue(FSTiedInterface::ALTOL, si.toln);
				pi->SetFloatValue(FSTiedInterface::PENALTY, si.pen);
			}
			break;
		case -3:
		case 3: // sliding contact
			{
				FSSlidingWithGapsInterface* pi = new FSSlidingWithGapsInterface(ps);
				sprintf(szname, "SlidingInterface%02d", i+1);
				pi->SetName(szname);
				ps->GetStep(0)->AddComponent(pi);
				pi->SetSecondarySurface(pms);
				pi->SetPrimarySurface(pss);

				bool twopass = (si.itype == -3);
				bool auto_pen = (si.pen >= 0);

				pi->SetFloatValue(FSSlidingWithGapsInterface::ALTOL, si.toln);
				pi->SetFloatValue(FSSlidingWithGapsInterface::PENALTY, fabs(si.pen));
				pi->SetBoolValue (FSSlidingWithGapsInterface::AUTOPEN, auto_pen);
				pi->SetBoolValue (FSSlidingWithGapsInterface::TWOPASS, twopass);
				pi->SetFloatValue(FSSlidingWithGapsInterface::MU, si.mus);
				pi->SetFloatValue(FSSlidingWithGapsInterface::EPSF, fabs(si.pen));
			}
			break;
		}
	}
}

//-----------------------------------------------------------------------------
void NIKE3DImport::build_loadcurves(NIKE3DProject &nike)
{
	int nlc = (int)nike.m_LC.size();
	m_LC.resize(nlc);
	list<LoadCurve>::iterator plc = nike.m_LC.begin();
	for (int i=0; i<nlc; ++i, ++plc) 
	{
		m_LC[i] = *plc;
		m_LC[i].SetID(plc->GetID());
	}
}


//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Reads the control deck from a nike input file
//
bool NIKE3DImport::ReadControlDeck(NIKE3DProject& prj)
{
	// storage for input line
	const int MAX_LINE = 256;
	char szline[MAX_LINE];
	int nread;

	NIKE3DProject::CONTROL& c = prj.m_Ctrl;

	string fileName = GetFileName();


	// -------- control card 1 --------
	if (read_line(m_fp, szline, MAX_LINE, false) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());
	strcpy(c.sztitle, szline);

	// -------- control card 2 --------
	if (read_line(m_fp, szline, MAX_LINE, false) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());
	char szf[5] = {0};
	nread = sscanf(szline, "%2s%3d%10d%10d%10d%10d%5d%5d%5d%5d%5d", szf, &c.nmmat, &c.numnp, &c.numelh, &c.numelb, &c.numels, &c.num1d, &c.numsi, &c.nrwsp, &c.inpde, &c.numrnf);
	if (nread < 6) return errf(szerr[ERR_CC], 2);

	// make sure input format is correct
	if (strcmp(szf,"FL") != 0) return errf(szerr[ERR_FORMAT]);

	// -------- control card 3 --------
	if (read_line(m_fp, szline, MAX_LINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());
	char szauto[5] = {0};
	nread = sscanf(szline, "%10d%10lg %4s%5d%5d%10lg%10lg%5d%5d%5d%5d%5d%5d", &c.ntime, &c.dt, szauto, &c.mxback, &c.iteopt, &c.dtmin, &c.dtmax, &c.irfwin, &c.spf, &c.linearb, &c.linears, &c.linearbm, &c.linearpr);
	if (nread < 7) return errf(szerr[ERR_CC], 3);
	c.nauto = 0;
	if ((strcmp(szauto, "auto") == 0) || 
		(strcmp(szauto, "AUTO") == 0)) c.nauto = 1;

	// -------- control card 4 --------
	if (read_line(m_fp, szline, MAX_LINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());
	nread = sscanf(szline, "%5d%5d%5d%5d%5d%5d%5d%5d%5d%5d%5d%5d%5d%5d%5d", &c.numlc, &c.nptm, &c.numcnl, &c.numpr, &c.numdis, &c.numadl, &c.nrcc, &c.bfa[0], &c.bfa[1], &c.bfa[2], &c.bfav[0], &c.bfav[1], &c.bfav[2], &c.nsteer, &c.nfnbc);
	if (nread < 2) return errf(szerr[ERR_CC], 4);

	// -------- control card 5 --------
	if (read_line(m_fp, szline, MAX_LINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());
	char sw[6] = {0};
	nread = sscanf(szline, "%5d%5d%5d%5d%5d%5d%5d%5d%5d%5s%5d%5d%5d", &c.ipri, &c.jpri, &c.nnpb, &c.nhpb, &c.nbpb, &c.nspb, &c.jrfreq, &c.irfreq, &c.istrn, sw, &c.iacflg, &c.maxaug, &c.ingap);
//	if (nread != 13) return errf(szerr[ERR_CC], 5);
	c.sw[0] = (int) (sw[0] - '0');
	c.sw[1] = (int) (sw[1] - '0');
	c.sw[2] = (int) (sw[2] - '0');
	c.sw[3] = (int) (sw[3] - '0');
	c.sw[4] = (int) (sw[4] - '0');

	// -------- control card 6 --------
	if (read_line(m_fp, szline, MAX_LINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());
	nread = sscanf(szline, "%5d%5d%10d%10d%5d%5d%10lg%10lg%10lg%10lg%10lg", &c.mthsol, &c.sflag, &c.icnt1, &c.icnt2, &c.ilimit, &c.maxref, &c.dtol, &c.ectl, &c.rctl, &c.tolls, &c.inref);
	if (nread < 10) return errf(szerr[ERR_CC], 6);

	// -------- control card 7 --------
	if (read_line(m_fp, szline, MAX_LINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());
	nread  = sscanf(szline, "%5d%5d%5d%5d%5d%10lg%10lg%10lg%10lg%10lg%5d%5d", &c.imass, &c.intvel, &c.iteo, &c.itpro, &c.neig, &c.eshift, &c.fnip, &c.snip, &c.alpha, &c.etol, &c.ieigit, &c.ieigens);
//	if (nread != 12) return errf(szerr[ERR_CC], 7);

	// -------- control card 8 --------
	if (read_line(m_fp, szline, MAX_LINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());
	nread = sscanf(szline, "%5d%10d%5d%5d%5d%5d%5d%10lg%5d%5d%5d%5d%5d%5d", &c.iunsym, &c.nwebin, &c.ifiss, &c.ioroin, &c.ibrfor, &c.igsoin, &c.ishfin, &c.qhg, &c.isgsin, &c.ibmfin, &c.ibgsin, &c.istold, &c.irotary, &c.geotol);
//	if (nread != 14) return errf(szerr[ERR_CC], 8);

	// -------- control card 9 --------
	if (read_line(m_fp, szline, MAX_LINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());
	nread = sscanf(szline, "%*20s%10lg%5d%5d%5d%5d%5d", &c.dsx, &c.irco, &c.nusbir, &c.mpubr, &c.nussir, &c.mpusr);
//	if (nread != 6) return errf(szerr[ERR_CC], 9);

	// -------- control card 10 --------
	if (read_line(m_fp, szline, MAX_LINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());
	nread = sscanf(szline, "%5d%5d%10lg%5d%5d%5d", &c.itrsol, &c.itrlmt, &c.tollin, &c.lbuf, &c.itrpnt, &c.iebeoc);
//	if (nread != 6) return errf(szerr[ERR_CC], 10);

	return true;
}



//-----------------------------------------------------------------------------
// Reads the material deck from a nike input file
//
bool NIKE3DImport::ReadMaterialDeck(NIKE3DProject& prj)
{
	int i, j, l;

	const int MAX_LINE = 256;
	char szline[MAX_LINE];
	int nread;

	string fileName = GetFileName();

	// see if there are any materials in the file
	NIKE3DProject::CONTROL& c = prj.m_Ctrl;
	if (c.nmmat == 0) return true;

	// create new materials
	prj.m_Mat.resize(c.nmmat);

	// read material data
	for (i=0; i<c.nmmat; ++i)
	{
		NIKE3DProject::MATERIAL& m = prj.m_Mat[i];

		// -------- material card 1 --------
		if (read_line(m_fp, szline, MAX_LINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());

		char stype[6] = { 0 };
		char sdens[11] = { 0 };
		char selem[6] = { 0 };
		char sTref[11] = { 0 };
		char srda[11] = { 0 };
		char srdb[11] = { 0 };
		char shrgl[11] = { 0 };
		char sflag[11] = { 0 };
		nread = sscanf(szline, "%*5c%5c%10c%5c%10c%10c%10c%10c%10c", stype, sdens, selem, sTref, srda, srdb, shrgl, sflag);
		if (nread < 3) return errf(szerr[ERR_MAT], 1, i);
		m.ntype = atoi(stype);
		m.dens  = atof(sdens);
		m.nelem = atoi(selem);
		m.Tref  = atof(sTref);
		m.rda   = atof(srda);
		m.rdb   = atof(srdb);
		m.hrgl  = atof(shrgl);
		m.flag  = atof(sflag);

		// -------- material card 2 --------
		if (read_line(m_fp, szline, MAX_LINE, false) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());
		// remove trailing whitepsace
		l = (int)strlen(szline);
		while ((l>0) && isspace(szline[l-1])) --l;
		szline[l] = 0;
		//remove leading whitespace
		l = 0;
		while (isspace(szline[l])) ++l;
		strcpy(m.szname, szline+l);

		// -------- material card 3 - 8 --------
		int n = (m.nelem == 2? 8 : 6);
		for (j=0; j<n; j++)
		{
			if (read_line(m_fp, szline, MAX_LINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());
			double* mp = m.m[j];
			for (l=0; l<8; ++l) mp[l] = 0;
			nread = sscanf(szline, "%lg%lg%lg%lg%lg%lg%lg%lg", mp, mp+1, mp+2, mp+3, mp+4, mp+5, mp+6, mp+7);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Reads the nodes 
//
bool NIKE3DImport::ReadNodes(NIKE3DProject& prj)
{
	// make sure there are any nodes to read
	NIKE3DProject::CONTROL& c = prj.m_Ctrl;
	int nn = c.numnp;
	if (nn == 0) return true;

	// allocate storage
	prj.m_Node.resize(nn);

	string fileName = GetFileName();

	// read nodes
	char szline[MAXLINE];
	int nread;
	for (int i=0; i<nn; ++i)
	{
		if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());

		NIKE3DProject::NODE& n = prj.m_Node[i];

		// read initial coordinates
		nread = sscanf(szline, "%*8d%5d%20lg%20lg%20lg%5d", &n.bc, &n.x, &n.y, &n.z, &n.rc);
		if (nread < 5) return errf(szerr[ERR_NODE], i+1);
	}

	return true;
}

//-----------------------------------------------------------------------------
// read the solid element section
//
bool NIKE3DImport::ReadBricks(NIKE3DProject& prj)
{
	// make sure there are any bricks to read
	NIKE3DProject::CONTROL& c = prj.m_Ctrl;
	int nelh = c.numelh;
	if (nelh == 0) return true;

	// allocate storage
	prj.m_Brick.resize(nelh);

	string fileName = GetFileName();

	// read elements
	char szline[MAXLINE];
	int nread;
	for (int i=0; i<nelh; ++i)
	{
		if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());

		NIKE3DProject::BRICK& e = prj.m_Brick[i];
		int* n = e.n;
		nread = sscanf(szline,"%*8d%5d%8d%8d%8d%8d%8d%8d%8d%8d", &e.nmat, n,n+1,n+2,n+3,n+4,n+5,n+6,n+7);
		if (nread != 9) return errf(szerr[ERR_HEX], i+1);
	}

	return true;
}

//-----------------------------------------------------------------------------
// read the shell elements
//
bool NIKE3DImport::ReadShells(NIKE3DProject &prj)
{
	// make sure we have shells
	NIKE3DProject::CONTROL& c = prj.m_Ctrl;
	int nels = c.numels;
	if (nels == 0) return true;

	// allocate storage
	prj.m_Shell.resize(nels);

	string fileName = GetFileName();

	// read shell data
	char szline[MAXLINE];
	int nread, i;
	for (i=0; i<nels; ++i)
	{
		NIKE3DProject::SHELL& s = prj.m_Shell[i];

		// read the first shell card
		if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());

		nread = sscanf(szline,"%*8d%5d%8d%8d%8d%8d", &s.nmat, s.n,s.n+1,s.n+2,s.n+3);
		if (nread != 5) return errf(szerr[ERR_SHELL], i+1);

		// read the second shell card
		if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());

		nread = sscanf(szline,"%10lg%10lg%10lg%10lg", s.h, s.h+1, s.h+2, s.h+3);
		if (nread != 4) return errf(szerr[ERR_SHELL], i+1);
	}

	// now we check the material of the shells, because sometimes the thickness is
	// defined in the material interface
	for (i=0; i<nels; ++i)
	{
		NIKE3DProject::SHELL& s = prj.m_Shell[i];
		if ((s.h[0] == 0) && (s.h[1] == 0) && (s.h[2] == 0) && (s.h[3] == 0))
		{
			if ((s.nmat >= 1) && (s.nmat < (int) prj.m_Mat.size()))
			{
				NIKE3DProject::MATERIAL& m = prj.m_Mat[s.nmat-1];
				s.h[0] = m.m[7][0];
				s.h[1] = m.m[7][1];
				s.h[2] = m.m[7][2];
				s.h[3] = m.m[7][3];
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// read the rigid nodes and/or facets
//
bool NIKE3DImport::ReadRigidFacets(NIKE3DProject &prj)
{
	// make sure there is something to read
	NIKE3DProject::CONTROL& c = prj.m_Ctrl;
	int nrf = c.numrnf;
	if (nrf == 0) return true;

	string fileName = GetFileName();

	// read data
	char szline[MAXLINE];
	int nread;
	for (int i=0; i<nrf; ++i)
	{
		NIKE3DProject::RIGID_FACET f;

		if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());
		nread = sscanf(szline, "%5d%8d%8d%8d%8d", &f.nrb, f.node,f.node+1, f.node+2,f.node+3);
		f.nsize = nread-1;

		prj.m_Rigid.push_back(f);
	}

	return true;
}

//-----------------------------------------------------------------------------
// read discrete elements deck
// TODO: for now, we just skip this section

bool NIKE3DImport::ReadDiscreteElements(NIKE3DProject& prj)
{
	// see if there are any discrete elements
	NIKE3DProject::CONTROL& c = prj.m_Ctrl;
	if (c.inpde == 0) return true;

	string fileName = GetFileName();

	int ndemat, numdel, nummas;
	char szline[MAXLINE];
	int i, nread;

	// read the control card
	if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());
	nread = sscanf(szline, "%5d%5d%5d", &ndemat, &numdel, &nummas);
	if (nread != 3) return errf(szerr[ERR_DEC], fileName.c_str());

	// read the materials
	prj.m_DMA.resize(ndemat);
	for (i=0; i<ndemat; ++i) 
	{
		NIKE3DProject::DISCRETE_MATERIAL& m = prj.m_DMA[i];

		// material card 1
		if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());
		sscanf(szline, "%5d%5d", &m.nid, &m.ntype);

		// material card 2
		if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());
		sscanf(szline, "%10lg%10lg%10lg%10lg%10lg", m.m, m.m+1, m.m+2, m.m+3, m.m+4);
	}

	// read the discrete elements
	prj.m_DSP.resize(numdel);
	for (i=0; i<numdel; ++i)
	{
		NIKE3DProject::DISCRETE_SPRING& s = prj.m_DSP[i];
		if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());
		sscanf(szline, "%5d%8d%8d%5d%10lg", &s.nid, &s.n1, &s.n2, &s.nmat, &s.s);
	}

	// read the discrete masses
	for (i=0; i<nummas; ++i)
	{
		if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());
	}

	return true;
}

//-----------------------------------------------------------------------------
// read contact interfaces
//
bool NIKE3DImport::ReadInterfaces(NIKE3DProject &prj)
{
	// make sure there are any interfaces
	NIKE3DProject::CONTROL& c = prj.m_Ctrl;
	int ni = c.numsi;
	if (ni == 0) return true;

	// allocate storage
	int nfaces = 0;
	prj.m_SI.resize(ni);

	// read interfaces
	char szline[MAXLINE];
	int nread, i;
	for (i=0; i<ni; ++i)
	{
		NIKE3DProject::SLIDING_INTERFACE& si = prj.m_SI[i];

		if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF]);
		nread = sscanf(szline, "%8d%8d%4d%10lg%10lg%10lg%10lg%10lg%10d%d", &si.nns, &si.nms, &si.itype, &si.pen, &si.mus, &si.muk, &si.fde, &si.pend, &si.bwrad, &si.aicc);
		if (nread != 10) return errf(szerr[ERR_SI]);

		// read the auxiliary control card
		if (si.aicc != 0)
		{
			if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF]);
			nread = sscanf(szline, "%5d%10lg%10lg%10lg%10lg%10lg", &si.iaug, &si.toln, &si.tolt, &si.tkmult, &si.tdeath, &si.tbury);
			if (nread != 6) return errf(szerr[ERR_SI]);
		}

		// update face count
		nfaces += si.nns + si.nms;
	}

	// read faces
	for (i=0; i<nfaces; ++i)
	{
		NIKE3DProject::SI_FACET f;

		if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF]);
		nread = sscanf(szline, "%8d%8d%8d%8d%8d", &f.nid, f.n, f.n+1, f.n+2, f.n+3);
		if (nread != 5) return errf(szerr[ERR_SI]);

		prj.m_Face.push_back(f);
	}

	return true;
}

//-----------------------------------------------------------------------------
// Reads the load curve deck from a nike input file
//

bool NIKE3DImport::ReadLoadCurves(NIKE3DProject& prj)
{
	// make sure there is something to read
	NIKE3DProject::CONTROL& c = prj.m_Ctrl;
	int nlc = c.numlc;
	if (nlc == 0) return true;

	string fileName = GetFileName();

	// read load curves
	char szline[MAXLINE];
	int nread, np;
	for (int i=0; i<nlc; ++i)
	{
		LoadCurve lc;

		// -------- load card 1 --------
		if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());

		nread = sscanf(szline, "%*5d%5d", &np);
		if (nread != 1) return errf(szerr[ERR_LC], i+1);

		// -------- load card 2 - n --------
		lc.Clear();
		for (int j=0; j<np; ++j)
		{
			if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());

			double d[2];
			nread = sscanf(szline, "%10lg%10lg", &d[0], &d[1]);
			if (nread != 2) return errf(szerr[ERR_LC], i+1);

			lc.Add(d[0], d[1]);
		}

		// add the loadcurve to the project
		prj.m_LC.push_back(lc);
	}

	return true;
}

//-----------------------------------------------------------------------------
// read concentrated nodal forces
//
bool NIKE3DImport::ReadNodalForces(NIKE3DProject &prj)
{
	// make sure there is something to read
	NIKE3DProject::CONTROL& c = prj.m_Ctrl;
	int ncnf = c.numcnl;
	if (ncnf == 0) return true;

	string fileName = GetFileName();

	// read data
	char szline[MAXLINE];
	int nread;
	for (int i=0; i<ncnf; ++i)
	{
		if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());

		NIKE3DProject::NODAL_LOAD nl;

		nread = sscanf(szline, "%8d%5d%5d%10lg", &nl.node, &nl.bc, &nl.lc, &nl.s);
		if (nread != 4) return errf(szerr[ERR_CFORCE]);

		// add it to the project
		prj.m_NF.push_back(nl);
	}

	return true;
}

//-----------------------------------------------------------------------------
// read pressure facets
//

bool NIKE3DImport::ReadPressureFacets(NIKE3DProject& prj)
{
	// make sure there is something to read
	NIKE3DProject::CONTROL& c = prj.m_Ctrl;
	int npl = c.numpr;
	if (npl == 0) return true;

	string fileName = GetFileName();

	// read data
	char szline[MAXLINE];
	int nread;
	for (int i=0; i<npl; ++i)
	{
		if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());

		NIKE3DProject::PRESSURE_LOAD pl;
		int* n = pl.n;
		double* s = pl.s;

		nread = sscanf(szline, "%5d%8d%8d%8d%8d%10lg%10lg%10lg%10lg", &pl.lc,n,n+1,n+2,n+3,s,s+1,s+2,s+3);
		if (nread != 9) return errf(szerr[ERR_PRESS]);

		prj.m_PF.push_back(pl);
	}

	return true;
}

//-----------------------------------------------------------------------------
// read nodal displacements
//
bool NIKE3DImport::ReadDisplacements(NIKE3DProject &prj)
{
	// make sure there is something to read
	NIKE3DProject::CONTROL& c = prj.m_Ctrl;
	int ndc = c.numdis;
	if (ndc == 0) return true;

	string fileName = GetFileName();

	// read data
	char szline[MAXLINE];
	int nread;
	for (int i=0; i<ndc; ++i)
	{
		if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());

		NIKE3DProject::NODAL_DISPLACEMENT dc;

		nread = sscanf(szline, "%8d%5d%5d%10lg%5d", &dc.node, &dc.bc, &dc.lc, &dc.s, &dc.nstat);
		if (nread != 5) return errf(szerr[ERR_DISP]);

		prj.m_DC.push_back(dc);
	}

	return true;
}

//-----------------------------------------------------------------------------
// read  body forces
//
bool NIKE3DImport::ReadBodyForce(NIKE3DProject &prj)
{
	char szline[MAXLINE];
	int nread, i;

	NIKE3DProject::CONTROL& c = prj.m_Ctrl;

	string fileName = GetFileName();

	// read body force
	for (i=0; i<3; ++i)
	{
		NIKE3DProject::BODY_FORCE bf;
		if (c.bfa[i] != 0)
		{
			if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());

			nread = sscanf(szline, "%5d%10lg", &bf.lc, &bf.s);
			if (nread != 2) return errf(szerr[ERR_BFORCE]);

			prj.m_BF.push_back(bf);
		}
	}

	// read angular acceleration
	for (i=0; i<3; ++i)
	{
		NIKE3DProject::BODY_FORCE bf;
		if (c.bfav[i] != 0)
		{
			if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());

			nread = sscanf(szline, "%5d%10lg", &bf.lc, &bf.s);
			if (nread != 2) return errf(szerr[ERR_BFORCE]);

			prj.m_BF.push_back(bf);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// read initial nodal velocities
//
bool NIKE3DImport::ReadVelocities(NIKE3DProject &prj)
{
	// make sure there is something to read
	NIKE3DProject::CONTROL& c = prj.m_Ctrl;
	int nvel = c.intvel;
	if (nvel == 0) return true;

	// allocate storage
	prj.m_Vel.resize(nvel);

	string fileName = GetFileName();

	// read data
	char szline[MAXLINE];
	int nread;
	for (int i=0; i<nvel; ++i)
	{
		NIKE3DProject::NODAL_VELOCITY& v = prj.m_Vel[i];

		if (read_line(m_fp, szline, MAXLINE) == NULL) return errf(szerr[ERR_EOF], fileName.c_str());

		nread = sscanf(szline, "%8d%10lg%10lg%10lg%5d", &v.node, &v.vx, &v.vy, &v.vz, &v.ninc);
		if (nread != 5) return errf(szerr[ERR_VEL]);
	}

	return true;
}

//-----------------------------------------------------------------------------
void NIKE3DImport::UpdateFEModel(FSModel& fem)
{
	int i;
	FSStep& s = *fem.GetStep(0);

	// set control settings
	// TODO: not sure yet how to fix this.
/*	if (m_io.bctrl)
	{
		if (m_nmplc >= 0) 
		{
			plc = fem.GetMustPointLoadCurve();
			*plc = m_LC[m_nmplc];
			plc->Activate();
			plc->SetType(LoadCurve::LC_STEP);
		}
	}
*/
	// set boundary conditions
//	if (m_io.bboundary && m_io.bgeom)
	{
		// loop over all boundary conditions
		for (i=0; i<s.BCs(); ++i)
		{
			FSBoundaryCondition* pbc = s.BC(i);

			// nodal displacements
			FSPrescribedDisplacement* pdc = dynamic_cast<FSPrescribedDisplacement*>(pbc);
			if (pdc)
			{
//				plc = pdc->GetLoadCurve();
//				n = plc->GetID(); if (n >= 0) *plc = m_LC[n];
			}
		}

		// loop over all boundary conditions
		for (i=0; i<s.Loads(); ++i)
		{
			FSLoad* pl = s.Load(i);

			// nodal forces
			FSNodalDOFLoad* pfc = dynamic_cast<FSNodalDOFLoad*>(pl);
			if (pfc)
			{
//				plc = pfc->GetLoadCurve();
//				n = plc->GetID(); if (n >= 0) *plc = m_LC[n];
			}

			// pressure forces
			FSPressureLoad* ppc = dynamic_cast<FSPressureLoad*>(pl);
			if (ppc)
			{
//				plc = ppc->GetLoadCurve();
//				n = plc->GetID(); if (n >= 0) *plc = m_LC[n];
			}
		}
	}

	// set materials
//	if (m_io.bmat)
	{
		// find all material loadcurves
		for (i=0; i<m_nmat; ++i)
		{
			FSMaterial* pmat = m_pMat[i]->GetMaterialProperties();
			ParamBlock& pb = pmat->GetParamBlock();
			for (int j=0; j<pb.Size(); ++j)
			{
				Param& p = pb[j];
	//			LoadCurve* plc = p.GetLoadCurve();
//				if (plc && (plc->Size() > 0))
//				{
//					LoadCurve& lc = *plc;
//					n = lc.GetID(); if (n >= 0) lc = m_LC[n];
//				}
			}
		}
	}

	// set body forces
//	if (m_io.bboundary)
	{
		// set rigid wall interfaces
		for (i=0; i<s.Interfaces(); ++i)
		{
			FSRigidWallInterface* pi = dynamic_cast<FSRigidWallInterface*>(s.Interface(i));
			if (pi)
			{
//				LoadCurve* plc = pi->GetParamLC(FSRigidWallInterface::OFFSET);
//				if (plc && plc->GetID() >= 0) *plc = m_LC[plc->GetID()];
			}
		}
	}

	// rigid constraints
	for (i=0; i<s.RigidConstraints(); ++i)
	{
		FSRigidPrescribed* rc = dynamic_cast<FSRigidPrescribed*>(s.RigidConstraint(i));
		if (rc)
		{
//			LoadCurve& lc = *rc->GetLoadCurve();
//			if (lc.GetID() >= 0) lc = m_LC[lc.GetID()];
		}
	}

	// set geometry
//	if (m_io.bgeom && m_po)
	{
		char fileTitle[256] = { 0 };
		FileTitle(fileTitle);
		m_po->SetName(fileTitle);
		fem.GetModel().AddObject(m_po);
	}

	// add the load curves
	for (int i = 0; i < m_LC.size(); ++i)
	{
		fem.AddLoadCurve(m_LC[i]);
	}
}


///////////////////////////////////////////////////////////////////////////////
// FUNCTION: FSFileImport::UpdateMesh
//

void NIKE3DImport::UpdateMesh(FSMesh& mesh)
{
//	mesh.Update();

	int nsize = 0, i, j, n, m, l;

	int nodes = mesh.Nodes();
	int faces = mesh.Faces();

	m_nFace.resize(nodes);
	for (i=0; i<nodes; ++i) m_nFace[i] = 0;

	for (i=0; i<faces; ++i)
	{
		FSFace& face = mesh.Face(i);
		n = face.Nodes();
		for (j=0; j<n; ++j) m_nFace[ face.n[j] ]++;
		nsize += n;
	}

	m_iFace.resize(nsize);
	m_pFace.resize(nodes);
	int *pi = &m_iFace[0];

	for (i=0; i<nodes; ++i)
	{
		m_pFace[i] = pi;
		n = m_nFace[i];
		pi += n;
		m_nFace[i] = 0;
	}

	for (i=0; i<faces; ++i)
	{
		FSFace& face = mesh.Face(i);
		n = face.Nodes();
		for (j=0; j<n; ++j) 
		{
			m = face.n[j];

			l = m_nFace[m];
			m_pFace[m][l] = i;
			m_nFace[m]++;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION: FSFileImport::FindFace
//

int NIKE3DImport::FindFace(int n[4], int noff)
{
	int* pf = m_pFace[n[0] - noff];
	int N = m_nFace[n[0] - noff];
	int i;

	FSMesh* pm = m_po->GetFEMesh();

	for (i=0; i<N; ++i)
	{
		FSFace& face = pm->Face( pf[i] );
		if (face.HasNode(n[1] - noff) &&
			face.HasNode(n[2] - noff) &&
			face.HasNode(n[3] - noff)) return pf[i];
	}

	return -1;
}

