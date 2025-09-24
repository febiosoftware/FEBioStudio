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

#include "NIKE3DProject.h"
#include <FEMLib/FSProject.h>
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FERigidConstraint.h>
#include <MeshLib/MeshMetrics.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEBodyLoad.h>
#include <GeomLib/GObject.h>
#include <FEMLib/GDiscreteObject.h>
#include <GeomLib/GModel.h>
#include <memory>
//using namespace std;

using std::unique_ptr;

//-----------------------------------------------------------------------------
// NIKE3DProject
//-----------------------------------------------------------------------------

bool NIKE3DProject::Create(FSProject &prj)
{
	// build the data structures
	if (BuildControl        (prj) == false) return false;
	if (BuildMaterials      (prj) == false) return false;
	if (BuildNodes          (prj) == false) return false;
	if (BuildElements       (prj) == false) return false;
	if (BuildRigidNodes     (prj) == false) return false;
	if (BuildDiscrete		(prj) == false) return false;
	if (BuildInterfaces     (prj) == false) return false;
	if (BuildNodalLoads     (prj) == false) return false;
	if (BuildPressureLoads  (prj) == false) return false;
	if (BuildDisplacements  (prj) == false) return false;
	if (BuildBodyForce      (prj) == false) return false;
	if (BuildNodalVelocities(prj) == false) return false;

	return true;
}

//-----------------------------------------------------------------------------

int NIKE3DProject::AddLoadCurve(LoadCurve& lc)
{
	m_LC.push_back(lc);
	m_Ctrl.numlc++;
	int np = lc.Points();
	if (np > m_Ctrl.nptm) m_Ctrl.nptm = np;
	return (int)m_LC.size();
}

//-----------------------------------------------------------------------------

void NIKE3DProject::Defaults()
{
	CONTROL& c = m_Ctrl;

	// CC1
	c.sztitle[0] = 0;

	// CC2
	c.nmmat = 0;
	c.numnp = 0;
	c.numelh = 0;
	c.numelb = 0;
	c.numels = 0;
	c.num1d = 0;
	c.numsi = 0;
	c.nrwsp = 0;
	c.inpde = 0;
	c.numrnf = 0;

	// CC3
	c.ntime = 0;
	c.dt    = 0;
	c.nauto = 0;
	c.mxback = 0;
	c.iteopt = 0;
	c.dtmin = 0;
	c.dtmax = 0;
	c.irfwin = 0;
	c.spf    = 0;
	c.linearb = 0;
	c.linears = 0;
	c.linearbm = 0;
	c.linearpr = 0;

	// CC4
	c.numlc = 0;
	c.nptm = 0;
	c.numcnl = 0;
	c.numpr = 0;
	c.numdis = 0;
	c.numadl = 0;
	c.nrcc = 0;
	c.bfa[0] = 0;
	c.bfa[1] = 0;
	c.bfa[2] = 0;
	c.bfav[0] = 0;
	c.bfav[1] = 0;
	c.bfav[2] = 0;
	c.nsteer = 0;
	c.nfnbc = 0;

	// CC5
	c.ipri = 0;
	c.jpri = 0;
	c.nnpb = -1;
	c.nhpb = -1;
	c.nbpb = -1;
	c.nspb = -1;
	c.jrfreq = 0;
	c.irfreq = 0;
	c.istrn  = 0;
	c.sw[0] = 6;
	c.sw[1] = 0;
	c.sw[2] = 0;
	c.sw[3] = 0;
	c.sw[4] = 0;
	c.iacflg = 0;
	c.maxaug = 0;
	c.ingap = 0;

	// CC6
	c.mthsol = 1;
	c.sflag = 0;
	c.icnt1 = 0;
	c.icnt2 = 0;
	c.ilimit = 0;
	c.maxref = 0;
	c.dtol = 0;
	c.ectl = 0;
	c.rctl = 0;
	c.tolls = 0;
	c.inref = 0;

	// CC7
	c.imass = 0;
	c.intvel = 0;
	c.iteo = 0;
	c.itpro = 0;
	c.neig = 0;
	c.eshift = 0;
	c.fnip = 0;
	c.snip = 0;
	c.alpha = 0;
	c.etol = 0;
	c.ieigit = 0;
	c.ieigens = 1;

	// CC8
	c.iunsym = 0;
	c.nwebin = 0;
	c.ifiss = 1;
	c.ioroin = 1;
	c.ibrfor = 1;
	c.igsoin = 2;
	c.ishfin = 1;
	c.qhg = 0;
	c.isgsin = 2;
	c.ibmfin = 1;
	c.ibgsin = 2;
	c.istold = 0;
	c.irotary = 0;
	c.geotol = 0;

	// CC9
	c.dsx = 0;
	c.irco = 0;
	c.nusbir = 0;
	c.mpubr = 0;
	c.nussir = 0;
	c.mpusr = 0;

	// CC10
	c.itrsol = 0;
	c.itrlmt = 0;
	c.tollin = 0;
	c.lbuf = 0;
	c.itrpnt = 0;
	c.iebeoc = 1;
}


//-----------------------------------------------------------------------------
// Only the default parameters are intialized here. The rest of the control 
// parameters are filled out when the corresponding data becomes available
//
bool NIKE3DProject::BuildControl(FSProject& prj)
{
	CONTROL& c = m_Ctrl;
	FSModel& fem = prj.GetFSModel();

	STEP_SETTINGS set;
	FSAnalysisStep* pstep = 0;
	if (fem.Steps() > 1)
	{
		pstep = dynamic_cast<FSAnalysisStep*>(fem.GetStep(1));
		assert(pstep);
		set = pstep->GetSettings();
	}
	else set.Defaults();

	// make sure this is a mechanics step
	FSNonLinearMechanics* pnlstep = dynamic_cast<FSNonLinearMechanics*>(pstep);
	if (pnlstep == 0) return false;

	int nlc = -1;
	if (pstep)
	{
		LoadCurve* plc = pstep->GetMustPointLoadCurve();
		if (set.bmust) nlc = AddLoadCurve(*plc);
	}
	else set.bmust = false;

	// set default values
	Defaults();

	// set the project title
	sprintf(c.sztitle, "%-40s", prj.GetTitle().c_str());

	// time settings
	c.ntime = set.ntime;
	c.dt    = set.dt;
	c.nauto = (set.bauto? 1 : 0);
	c.mxback = set.mxback;
	c.iteopt = set.iteopt;
	c.dtmin = set.dtmin;
	c.dtmax = (nlc > 0 ? -nlc : set.dtmax);

	// convergence tolerances
	c.dtol = pnlstep->GetDisplacementTolerance();
	c.ectl = pnlstep->GetEnergyTolerance();
	c.rctl = pnlstep->GetResidualTolerance();
	c.tolls = pnlstep->GetLineSearchTolerance();

	// analysis type
	c.imass = set.nanalysis;

	// solver settings
	c.maxref = set.maxref;
	c.ilimit = set.ilimit;

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DProject::BuildMaterials(FSProject& prj)
{
	int i, j, k;

	FSModel& fem = prj.GetFSModel();
	FSStep& step = *fem.GetStep(0);
	int nmat = fem.Materials();

	// update control data
	m_Ctrl.nmmat = nmat;

	// create materials
	m_Mat.resize(nmat);
	for (i=0; i<nmat; i++)
	{
		// get the material and tag it
		GMaterial* pgm = fem.GetMaterial(i);
		FSMaterial* pmat = pgm->GetMaterialProperties();
		pgm->m_ntag = i;

		// set the default material properties
		MATERIAL& mat = m_Mat[i];

		mat.ntype = 0;
		mat.dens  = 0;
		mat.nelem = 0;
		mat.Tref  = 0;
		mat.rda   = 0;
		mat.rdb   = 0;
		mat.hrgl  = 0;
		mat.flag  = 0;
		for (j=0; j<8; ++j)
			for (k=0; k<8; ++k) mat.m[j][k] = 0;

		strcpy(mat.szname, pgm->GetName().c_str());

		switch (pmat->Type())
		{
		case FE_ISOTROPIC_ELASTIC:
			{
				mat.ntype = 1;
				FSIsotropicElastic* pm = dynamic_cast<FSIsotropicElastic*>(pmat);
				mat.m[0][0] = pm->GetParam(FSIsotropicElastic::MP_E).GetFloatValue();
				mat.m[1][0] = pm->GetParam(FSIsotropicElastic::MP_v).GetFloatValue();
				mat.dens = pm->GetParam(FSIsotropicElastic::MP_DENSITY).GetFloatValue();
			}
			break;
		case FE_MOONEY_RIVLIN:
			{
				mat.ntype = 15;
				FSMooneyRivlin* pm = dynamic_cast<FSMooneyRivlin*>(pmat);
				double A = pm->GetParam(FSMooneyRivlin::MP_A).GetFloatValue();
				double B = pm->GetParam(FSMooneyRivlin::MP_B).GetFloatValue();
				double K = pm->GetParam(FSMooneyRivlin::MP_K).GetFloatValue();

				mat.m[0][0] = A;
				mat.m[1][0] = B;
				mat.m[2][0] = (3*K - 4*(A+B))/(6*K+4*(A+B));
				mat.dens = pm->GetParam(FSMooneyRivlin::MP_DENSITY).GetFloatValue();
			}
			break;
		case FE_OGDEN_MATERIAL:
			{
				mat.ntype = 63;
				FSOgdenMaterial* pm = dynamic_cast<FSOgdenMaterial*>(pmat);
				double c[3], m[3];
				for (int i=0; i<3; ++i)
				{
					c[i] = pm->GetParam(FSOgdenMaterial::MP_C1+i).GetFloatValue();
					m[i] = pm->GetParam(FSOgdenMaterial::MP_M1+i).GetFloatValue();
				}
				mat.dens = pm->GetParam(FSOgdenMaterial::MP_DENSITY).GetFloatValue();

				mat.m[0][0] = pm->GetParam(FSOgdenMaterial::MP_K).GetFloatValue();
				mat.m[0][1] = c[0] / m[0];
				mat.m[0][2] = m[0];
				mat.m[0][3] = c[1] / m[1];
				mat.m[0][4] = m[1];
				mat.m[0][5] = c[2] / m[2];
				mat.m[0][6] = m[2];
			}
			break;
		case FE_TRANS_ISO_MOONEY_RIVLIN:
			{
				mat.ntype = 18;
				FSTransMooneyRivlin* pm = dynamic_cast<FSTransMooneyRivlin*>(pmat);
				FSOldFiberMaterial& f = *pm->GetFiberMaterial();
				mat.m[0][0] = pm->GetFloatValue(FSTransMooneyRivlin::MP_C1);
				mat.m[0][1] = pm->GetFloatValue(FSTransMooneyRivlin::MP_C2);
				mat.m[0][2] = f.GetFloatValue(FSTransMooneyRivlin::MP_C3);
				mat.m[0][3] = f.GetFloatValue(FSTransMooneyRivlin::MP_C4);
				mat.m[0][4] = f.GetFloatValue(FSTransMooneyRivlin::MP_C5);
						
				mat.m[1][0] = pm->GetFloatValue(FSTransMooneyRivlin::MP_K);
				mat.m[1][1] = f.GetFloatValue(FSTransMooneyRivlin::MP_LAM);

				mat.dens = pm->GetParam(FSTransMooneyRivlin::MP_DENSITY).GetFloatValue();
					
				if (f.m_naopt == FE_FIBER_LOCAL) 
				{
					mat.m[3][0] = 0;
					mat.m[4][0] = f.m_n[0];
					mat.m[4][1] = f.m_n[1];
					mat.m[4][2] = 4;
				}
				if (f.m_naopt == FE_FIBER_SPHERICAL)
				{
					mat.m[3][0] = 1;

					mat.m[4][0] = f.m_r.x;
					mat.m[4][1] = f.m_r.y;
					mat.m[4][2] = f.m_r.z;
				}
				if (f.m_naopt == FE_FIBER_VECTOR)
				{
					mat.m[3][0] = 2;

					mat.m[4][0] = f.m_a.x;
					mat.m[4][1] = f.m_a.y;
					mat.m[4][2] = f.m_a.z;
							
					mat.m[5][0] = f.m_d.x;
					mat.m[5][1] = f.m_d.y;
					mat.m[5][2] = f.m_d.z;
				}

//				LoadCurve& ac = f.GetParam(FETransMooneyRivlin::Fiber::MP_AC).GetLoadCurve();
//				mat.m[5][3] = ac.m_nID;
//				mat.m[5][4] = f.GetFloatValue(FETransMooneyRivlin::Fiber::MP_CA0);
//				mat.m[5][5] = f.GetFloatValue(FETransMooneyRivlin::Fiber::MP_BETA);
//				mat.m[5][6] = f.GetFloatValue(FETransMooneyRivlin::Fiber::MP_LREF);
//				mat.m[5][7] = f.GetFloatValue(FETransMooneyRivlin::Fiber::MP_L0);
			}
			break;
		case FE_RIGID_MATERIAL:
			{
				mat.ntype = 20;
				FSRigidMaterial* pm = dynamic_cast<FSRigidMaterial*>(pmat);
				mat.dens = pm->GetFloatValue(FSRigidMaterial::MP_DENSITY);

				mat.m[0][0] = pm->GetFloatValue(FSRigidMaterial::MP_E);
				mat.m[1][0] = pm->GetFloatValue(FSRigidMaterial::MP_V);

				// we need to loop over all the rigid constraints for this step
				// and set the material properties accordingly
				for (j=0; j<6; ++j) mat.m[2][j] = 0;
				for (k=0; k<step.RigidConstraints(); ++k)
				{
					FSRigidConstraint& rc = *step.RigidConstraint(k);
					if (rc.GetMaterialID() == pgm->GetID())
					{
						switch (rc.Type())
						{
						case FE_RIGID_FIXED:
							{
								FSRigidFixed& rf = dynamic_cast<FSRigidFixed&>(rc);
								for (j=0; j<6; ++j)
									if (rf.GetDOF(j)) mat.m[2][j] = -1;
							}
							break;
						case FE_RIGID_DISPLACEMENT:
							{
								FSRigidDisplacement& rf = dynamic_cast<FSRigidDisplacement&>(rc);
//								if (rf.GetDOF() >= 0) mat.m[2][rf.GetDOF()] = AddLoadCurve(*rf.GetLoadCurve(FSRigidDisplacement::VALUE));
							}
							break;
						}
					}
				}
				
				vec3d rc = pm->GetVecValue(FSRigidMaterial::MP_RC);
				bool bcom = pm->GetBoolValue(FSRigidMaterial::MP_COM);
				mat.m[3][0] = (bcom? 1 : 0);
				mat.m[3][1] = (bcom? rc.x: 0);
				mat.m[3][2] = (bcom? rc.y: 0);
				mat.m[3][3] = (bcom? rc.z: 0);
			}
			break;
		case FE_VISCO_ELASTIC:
			{
				mat.ntype = 18;
				FSViscoElastic* pm = dynamic_cast<FSViscoElastic*>(pmat);
				FSMaterial* psub = pm->GetElasticMaterial();

				int G1 = FSViscoElastic::MP_G1;
				int T1 = FSViscoElastic::MP_T1;

				mat.m[0][5] = pm->GetFloatValue(G1  ); mat.m[0][6] = pm->GetFloatValue(G1+1); mat.m[0][7] = pm->GetFloatValue(G1+2);
				mat.m[1][5] = pm->GetFloatValue(T1  ); mat.m[1][6] = pm->GetFloatValue(T1+1); mat.m[1][7] = pm->GetFloatValue(T1+2);
				mat.m[2][5] = pm->GetFloatValue(G1+3); mat.m[2][6] = pm->GetFloatValue(G1+4); mat.m[2][7] = pm->GetFloatValue(G1+5);
				mat.m[3][5] = pm->GetFloatValue(T1+3); mat.m[3][6] = pm->GetFloatValue(T1+4); mat.m[3][7] = pm->GetFloatValue(T1+5);

				if (psub && (psub->Type() == FE_MOONEY_RIVLIN))
				{
					FSMooneyRivlin* pmat = dynamic_cast<FSMooneyRivlin*>(psub);
					double A = pmat->GetParam(FSMooneyRivlin::MP_A).GetFloatValue();
					double B = pmat->GetParam(FSMooneyRivlin::MP_B).GetFloatValue();
					double K = pmat->GetParam(FSMooneyRivlin::MP_K).GetFloatValue();

					mat.m[0][0] = A;
					mat.m[1][0] = B;
					mat.m[2][0] = (3*K - 4*(A+B))/(6*K+4*(A+B));
				}
			}
			break;
/*		case FE_NIKE_MATERIAL:
			{
				FENIKEMaterial* pm = dynamic_cast<FENIKEMaterial*>(pmat);
				mat.ntype = pm->GetIntValue  (FENIKEMaterial::MP_TYPE);
				mat.dens  = pm->GetFloatValue(FENIKEMaterial::MP_DENSITY);
				mat.nelem = pm->GetIntValue  (FENIKEMaterial::MP_ELEM);
				mat.Tref  = pm->GetFloatValue(FENIKEMaterial::MP_TREF);
				mat.rda   = pm->GetFloatValue(FENIKEMaterial::MP_RDA);
				mat.rdb   = pm->GetFloatValue(FENIKEMaterial::MP_RDB);
				mat.hrgl  = pm->GetFloatValue(FENIKEMaterial::MP_HRGL);
				mat.flag  = pm->GetFloatValue(FENIKEMaterial::MP_FORM);

				memcpy(mat.m, pm->m_d, sizeof(double)*64);
			}
			break;
*/
		default:
//			flx_alert("NIKE3D does not support %s.\nIt will save an empty material deck for this material.", pgm->GetFullName());
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DProject::BuildNodes(FSProject& prj)
{
	FSModel& fem = prj.GetFSModel();
	GModel& model = fem.GetModel();
	FSStep& s = *fem.GetStep(0);

	// allocate nodes
	int i;
	int nodes = model.FENodes();
	m_Node.resize(nodes);

	// update control data
	m_Ctrl.numnp = nodes;

	// create nodes
	nodes = 0;
	vec3d r;
	for (i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FSMesh* pm = po->GetFEMesh();
		if (pm == 0) return false;
		for (int j=0; j<pm->Nodes(); ++j)
		{
			FSNode& node = pm->Node(j);
			NODE& m = m_Node[nodes];
			node.m_nid = ++nodes;
			r = po->GetTransform().LocalToGlobal(node.r);
			m.x = r.x;
			m.y = r.y;
			m.z = r.z;
			m.bc = m.rc = 0;
		}
	}

	// calculate the nodal BC and RC values
	for (i=0; i<s.BCs(); ++i)
	{
		FSFixedDisplacement* pbc = dynamic_cast<FSFixedDisplacement*>(s.BC(i));
		if (pbc)
		{
			int bc;
			FSItemListBuilder* pitem = pbc->GetItemList();
			int nbc = pbc->GetBC();
			unique_ptr<FSNodeList> pg(pitem->BuildNodeList());
			FSNodeList::Iterator pn = pg->First();
			for (int k=0; k<pg->Size(); ++k, ++pn)
			{
				bc = (nbc & 7);
				NODE& node = m_Node[(pn->m_pi)->m_nid-1];
				node.bc |= bc;
			}
		}
		FSFixedRotation* prc = dynamic_cast<FSFixedRotation*>(s.BC(i));
		if (pbc)
		{
			int rc;
			FSItemListBuilder* pitem = pbc->GetItemList();
			int nbc = pbc->GetBC();
			unique_ptr<FSNodeList> pg(pitem->BuildNodeList());
			FSNodeList::Iterator pn = pg->First();
			for (int k=0; k<pg->Size(); ++k, ++pn)
			{
				rc = (nbc & 7);
				NODE& node = m_Node[(pn->m_pi)->m_nid-1];
				node.rc |= rc;
			}
		}
	}

	// NIKE uses a different convention for constraints so we need to convert them
	int BC[8] = {0, 1, 2, 4, 3, 6, 5, 7};
	for (i=0; i<nodes; ++i)
	{
		m_Node[i].bc = BC[m_Node[i].bc];
		m_Node[i].rc = BC[m_Node[i].rc];
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DProject::BuildElements(FSProject &prj)
{
	FSModel& fem = prj.GetFSModel();
	GModel& model = fem.GetModel();

	int i, j, nmat = 0;
	// count nr of solid and shell elements
	int solids = 0;
	int shells = 0;
	for (i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FSMesh* pm = po->GetFEMesh();
		for (j=0; j<pm->Elements(); ++j)
		{
			FSElement& el = pm->Element(j);

			GMaterial* pmat = fem.GetMaterialFromID(po->Part(el.m_gid)->GetMaterialID());
			nmat = (pmat? pmat->m_ntag : -1);
			switch (el.Type())
			{
			case FE_HEX8:
			case FE_PENTA6:
			case FE_TET4:
				++solids;
				if (nmat >= 0) m_Mat[nmat].nelem = 0;
				break;
			case FE_TET10:
				++solids;
				if (nmat >= 0) m_Mat[nmat].nelem = 50;
				break;
			case FE_QUAD4:
			case FE_TRI3:
				++shells;
				if (nmat >= 0) m_Mat[nmat].nelem = 2;
				break;
			}
		}
	}

	// allocate storage
	if (solids) m_Brick.resize(solids);
	if (shells) m_Shell.resize(shells);

	// update control data
	m_Ctrl.numelh = solids;
	m_Ctrl.numels = shells;

	// build data
	solids = 0;
	shells = 0;
	for (i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FSMesh* pm = po->GetFEMesh();
		for (j=0; j<pm->Elements(); ++j)
		{
			FSElement& el = pm->Element(j);
			GMaterial* pmat = fem.GetMaterialFromID(po->Part(el.m_gid)->GetMaterialID());
			nmat = (pmat? pmat->m_ntag : -1);
			switch (el.Type())
			{
			case FE_TET4:
				{
					BRICK& b = m_Brick[solids++];
					b.nnew = false;
					b.nmat = nmat;
					b.n[0] = pm->Node(el.m_node[0]).m_nid;
					b.n[1] = pm->Node(el.m_node[1]).m_nid;
					b.n[2] = pm->Node(el.m_node[2]).m_nid;
					b.n[3] = pm->Node(el.m_node[3]).m_nid;
					b.n[4] = pm->Node(el.m_node[3]).m_nid;
					b.n[5] = pm->Node(el.m_node[3]).m_nid;
					b.n[6] = pm->Node(el.m_node[3]).m_nid;
					b.n[7] = pm->Node(el.m_node[3]).m_nid;
				}
				break;
			case FE_PENTA6:
				{
					BRICK& b = m_Brick[solids++];
					b.nnew = false;
					b.nmat = nmat;
					b.n[0] = pm->Node(el.m_node[2]).m_nid;
					b.n[1] = pm->Node(el.m_node[1]).m_nid;
					b.n[2] = pm->Node(el.m_node[4]).m_nid;
					b.n[3] = pm->Node(el.m_node[5]).m_nid;
					b.n[4] = pm->Node(el.m_node[0]).m_nid;
					b.n[5] = pm->Node(el.m_node[0]).m_nid;
					b.n[6] = pm->Node(el.m_node[3]).m_nid;
					b.n[7] = pm->Node(el.m_node[3]).m_nid;

				}
				break;
			case FE_HEX8:
				{
					BRICK& b = m_Brick[solids++];
					b.nnew = false;
					b.nmat = nmat;
					b.n[0] = pm->Node(el.m_node[0]).m_nid;
					b.n[1] = pm->Node(el.m_node[1]).m_nid;
					b.n[2] = pm->Node(el.m_node[2]).m_nid;
					b.n[3] = pm->Node(el.m_node[3]).m_nid;
					b.n[4] = pm->Node(el.m_node[4]).m_nid;
					b.n[5] = pm->Node(el.m_node[5]).m_nid;
					b.n[6] = pm->Node(el.m_node[6]).m_nid;
					b.n[7] = pm->Node(el.m_node[7]).m_nid;
				}
				break;
			case FE_TET10:
				{
					BRICK& b = m_Brick[solids++];
					b.nnew = true;
					b.nmat = nmat;
					b.n[0] = pm->Node(el.m_node[0]).m_nid;
					b.n[1] = pm->Node(el.m_node[1]).m_nid;
					b.n[2] = pm->Node(el.m_node[2]).m_nid;
					b.n[3] = pm->Node(el.m_node[3]).m_nid;
					b.n[4] = pm->Node(el.m_node[4]).m_nid;
					b.n[5] = pm->Node(el.m_node[5]).m_nid;
					b.n[6] = pm->Node(el.m_node[6]).m_nid;
					b.n[7] = pm->Node(el.m_node[7]).m_nid;
					b.n[8] = pm->Node(el.m_node[8]).m_nid;
					b.n[9] = pm->Node(el.m_node[9]).m_nid;
				}
				break;
			case FE_QUAD4:
				{
					SHELL& s = m_Shell[shells++];
					s.nmat = nmat;
					s.n[0] = pm->Node(el.m_node[0]).m_nid;
					s.n[1] = pm->Node(el.m_node[1]).m_nid;
					s.n[2] = pm->Node(el.m_node[2]).m_nid;
					s.n[3] = pm->Node(el.m_node[3]).m_nid;
					s.h[0] = el.m_h[0];
					s.h[1] = el.m_h[1];
					s.h[2] = el.m_h[2];
					s.h[3] = el.m_h[3];
				}
				break;
			case FE_TRI3:
				{
					SHELL& s = m_Shell[shells++];
					s.nmat = nmat;
					s.n[0] = pm->Node(el.m_node[0]).m_nid;
					s.n[1] = pm->Node(el.m_node[1]).m_nid;
					s.n[2] = pm->Node(el.m_node[2]).m_nid;
					s.n[3] = pm->Node(el.m_node[2]).m_nid;
					s.h[0] = el.m_h[0];
					s.h[1] = el.m_h[1];
					s.h[2] = el.m_h[2];
					s.h[3] = el.m_h[2];
				}
				break;
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DProject::BuildRigidNodes(FSProject &prj)
{
	FSModel& fem = prj.GetFSModel();
	FSStep& s = *fem.GetStep(0);

	for (int i=0; i<s.Interfaces(); ++i)
	{
		FSRigidInterface* pi = dynamic_cast<FSRigidInterface*>( s.Interface(i) );

		if (pi && pi->IsActive())
		{
			GMaterial* pgm = pi->GetRigidBody();
			FSMaterial* pm = pgm->GetMaterialProperties();
			int nRB = (pgm? pgm->m_ntag+1 : 0);
			RIGID_FACET rf;
			rf.nrb = nRB;
			rf.nsize = 4;
			int n = 0;

			FSItemListBuilder* pitem = pi->GetItemList();
			if (pitem)
			{
				unique_ptr<FSNodeList> pg(pitem->BuildNodeList() );
				FSNodeList::Iterator pn = pg->First();
				for (int k=0; k<pg->Size(); ++k, ++pn)
				{
					rf.node[n] = (pn->m_pi)->m_nid;
					n = (n+1)%4;
					if (n == 0) m_Rigid.push_back(rf);
				}
				if (n != 0)
				{
					rf.nsize = n;
					m_Rigid.push_back(rf);
				}
			}
		}
	}

	// update control data
	m_Ctrl.numrnf = (int)m_Rigid.size();

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DProject::BuildDiscrete(FSProject& prj)
{
	FSModel& fem = prj.GetFSModel();
	GModel& m = fem.GetModel();

	int n = 1;

	for (int i=0; i<m.DiscreteObjects(); ++i)
	{
		GLinearSpring* po = dynamic_cast<GLinearSpring*>(m.DiscreteObject(i));
		if (po)
		{
			GNode* pn0 = m.FindNode(po->m_node[0]);
			GNode* pn1 = m.FindNode(po->m_node[1]);

			if (pn0 && pn1)
			{
				GObject* po0 = dynamic_cast<GObject*>(pn0->Object());
				GObject* po1 = dynamic_cast<GObject*>(pn1->Object());
				int n1 = po0->GetFENode(pn0->GetLocalID())->m_nid;
				int n2 = po1->GetFENode(pn1->GetLocalID())->m_nid;
				double E = po->GetFloatValue(GLinearSpring::MP_E);

				DISCRETE_MATERIAL m;
				m.nid = n;
				m.ntype = 1;
				m.m[0] = E;
				m_DMA.push_back(m);

				DISCRETE_SPRING s;
				s.n1 = n1;
				s.n2 = n2;
				s.nid = n;
				s.nmat = n;
				s.s = 1;
				m_DSP.push_back(s);

				n++;
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DProject::BuildInterfaces(FSProject &prj)
{
	int i, k, n;

	FSModel& fem = prj.GetFSModel();
	FSStep& s = *fem.GetStep(0);

	// first we export all control data
	for (i=0; i<s.Interfaces(); ++i)
	{
		FSSlidingWithGapsInterface* pi = dynamic_cast<FSSlidingWithGapsInterface*>(s.Interface(i));

		// sliding interfaces
		if (pi && pi->IsActive())
		{
			SLIDING_INTERFACE si;
			SI_FACET f;

			bool bautopen = pi->GetBoolValue(FSSlidingWithGapsInterface::AUTOPEN);
			bool twopass = pi->GetBoolValue(FSSlidingWithGapsInterface::TWOPASS);
			double pen = pi->GetFloatValue(FSSlidingWithGapsInterface::PENALTY);

			// set sliding interface parameters
			si.nns = 0;
			si.nms = 0;
			si.itype  = (twopass ? -3 : 3);	// TODO: check to see if this is correct
			si.pen    = (bautopen? pen : -pen);
			si.mus    = pi->GetFloatValue(FSSlidingWithGapsInterface::MU);
			si.muk    = 0;
			si.fde    = 0;
			si.pend   = 0;
			si.bwrad  = 0;
			si.aicc   = 1;
			si.iaug   = 1;
			si.toln   = pi->GetFloatValue(FSSlidingWithGapsInterface::ALTOL);
			si.tolt   = 0;
			si.tkmult = 0;
			si.tdeath = 0;
			si.tbury  = 0;

			// update control counter
			m_Ctrl.numsi++;

			// count slave faces
			n = 0;
			FSItemListBuilder* pss = pi->GetPrimarySurface();
			if (pss)
			{
				unique_ptr<FSFaceList> pgs(pss->BuildFaceList());
				FSMesh* pm;
				FSFaceList::Iterator pf = pgs->First();
				for (k=0; k<pgs->Size(); ++k, ++pf)
				{
					FSFace& rf = *(pf->m_pi);
					pm = dynamic_cast<FSMesh*>(pf->m_pm);
					f.nid = ++n;
					f.n[0] = pm->Node(rf.n[0]).m_nid;
					f.n[1] = pm->Node(rf.n[1]).m_nid;
					f.n[2] = pm->Node(rf.n[2]).m_nid;
					f.n[3] = pm->Node((rf.Nodes()==3? rf.n[2]: rf.n[3])).m_nid;
	
					si.nns++;
					m_Face.push_back(f);
				}
			}
			
			// count master surfaces
			n = 0;
			FSItemListBuilder* pms = pi->GetSecondarySurface();
			if (pms)
			{
				unique_ptr<FSFaceList> pgm(pms->BuildFaceList());
				FSMesh* pm;
				FSFaceList::Iterator pf = pgm->First();
				for (k=0; k<pgm->Size(); ++k, ++pf)
				{
					FSFace& rf = *(pf->m_pi);
					pm = dynamic_cast<FSMesh*>(pf->m_pm);
					f.nid = ++n;
					f.n[0] = pm->Node(rf.n[0]).m_nid;
					f.n[1] = pm->Node(rf.n[1]).m_nid;
					f.n[2] = pm->Node(rf.n[2]).m_nid;
					f.n[3] = pm->Node((rf.Nodes()==3? rf.n[2]: rf.n[3])).m_nid;
	
					si.nms++;
					m_Face.push_back(f);
				}
			}
			
			// add sliding interface to list
			m_SI.push_back(si);
		}

		// tied interfaces
		FSTiedInterface* pt = dynamic_cast<FSTiedInterface*>(s.Interface(i));

		if (pt && pt->IsActive())
		{
			SLIDING_INTERFACE si;
			SI_FACET f;

			// set tied interface parameters
			si.nns = 0;
			si.nms = 0;
			si.itype  = 2;
			si.pen    = -pt->GetFloatValue(FSTiedInterface::PENALTY);
			si.mus    = 0;
			si.muk    = 0;
			si.fde    = 0;
			si.pend   = 0;
			si.bwrad  = 0;
			si.aicc   = 1;
			si.iaug   = 1;
			si.toln   = pt->GetFloatValue(FSTiedInterface::ALTOL);
			si.tolt   = 0;
			si.tkmult = 0;
			si.tdeath = 0;
			si.tbury  = 0;

			// update control counter
			m_Ctrl.numsi++;

			// count slave faces
			n = 0;
			FSItemListBuilder* pss = pt->GetPrimarySurface();
			if (pss)
			{
				unique_ptr<FSFaceList> pgs(pss->BuildFaceList());
				FSMesh* pm;
				FSFaceList::Iterator pf = pgs->First();
				for (k=0; k<pgs->Size(); ++k, ++pf)
				{
					FSFace& rf = *(pf->m_pi);
					pm = dynamic_cast<FSMesh*>(pf->m_pm);
					f.nid = ++n;
					f.n[0] = pm->Node(rf.n[0]).m_nid;
					f.n[1] = pm->Node(rf.n[1]).m_nid;
					f.n[2] = pm->Node(rf.n[2]).m_nid;
					f.n[3] = pm->Node((rf.Nodes()==3? rf.n[2]: rf.n[3])).m_nid;

					si.nns++;
					m_Face.push_back(f);
				}
			}

			// count master surfaces
			n = 0;
			FSItemListBuilder* pms = pt->GetSecondarySurface();
			if (pms)
			{
				unique_ptr<FSFaceList> pgm(pms->BuildFaceList());
				FSMesh* pm;
				FSFaceList::Iterator pf = pgm->First();
				for (k=0; k<pgm->Size(); ++k, ++pf)
				{
					FSFace& rf = *(pf->m_pi);
					pm = dynamic_cast<FSMesh*>(pf->m_pm);
					f.nid = ++n;
					f.n[0] = pm->Node(rf.n[0]).m_nid;
					f.n[1] = pm->Node(rf.n[1]).m_nid;
					f.n[2] = pm->Node(rf.n[2]).m_nid;
					f.n[3] = pm->Node((rf.Nodes()==3? rf.n[2]: rf.n[3])).m_nid;
	
					si.nms++;
					m_Face.push_back(f);
				}
			}

			// add tied interface to list
			m_SI.push_back(si);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DProject::BuildNodalLoads(FSProject& prj)
{
	FSModel& fem = prj.GetFSModel();
	FSStep& s = *fem.GetStep(0);

	int i, k;
	for (i=0; i<s.Loads(); ++i)
	{
		FSNodalDOFLoad* pbc = dynamic_cast<FSNodalDOFLoad*>(s.Load(i));
		if (pbc)
		{
//			LoadCurve& lc = *fem.GetParamCurve(pbc->GetParam(FSNodalDOFLoad::LOAD));
			int nlc = -1;// AddLoadCurve(lc);
			int bc = pbc->GetDOF() + 1;
			FSItemListBuilder* pitem = pbc->GetItemList();
			unique_ptr<FSNodeList> pg(pitem->BuildNodeList() );
			FSNodeList::Iterator pn = pg->First();
			for (k=0; k<pg->Size(); ++k, ++pn)
			{
				NODAL_LOAD nl;
				nl.bc = bc;
				nl.lc = nlc;
				nl.node = (pn->m_pi)->m_nid;
				//nl.s = 1;
				nl.s = pbc->GetLoad();
				m_NF.push_back(nl);
				m_Ctrl.numcnl++;
			}
		}

		FSSurfaceTraction* ptc = dynamic_cast<FSSurfaceTraction*>(s.Load(i));
		if (ptc)
		{
//			LoadCurve& lc = *ptc->GetLoadCurve(FSSurfaceTraction::LOAD);
			int nlc = -1;// AddLoadCurve(lc);

			FSItemListBuilder* pitem = ptc->GetItemList();
			unique_ptr<FSFaceList> ps(pitem->BuildFaceList());

			vec3d t = ptc->GetVecValue(FSSurfaceTraction::LOAD);

			std::vector<vec3d> fn; fn.resize(m_Ctrl.numnp);
			FSFaceList::Iterator pf = ps->First();
			FSMesh* pm;
			for (k=0; k<ps->Size(); ++k, ++pf)
			{
				FSFace& face = *(pf->m_pi);
				pm = dynamic_cast<FSMesh*>(pf->m_pm);
				double a = FEMeshMetrics::SurfaceArea(*pm, face);

				double w = 1.0 / face.Nodes();

				for (int i=0; i<face.Nodes(); ++i)
				{
					fn[pm->Node(face.n[i]).m_nid-1].x += a*w*t.x;
					fn[pm->Node(face.n[i]).m_nid-1].y += a*w*t.y;
					fn[pm->Node(face.n[i]).m_nid-1].z += a*w*t.z;
				}
			}

			unique_ptr<FSNodeList> pns(pitem->BuildNodeList());
			FSNodeList::Iterator pn = pns->First();
			for (k=0; k<pns->Size(); ++k, ++pn)
			{
				NODAL_LOAD lx, ly, lz;
				lx.bc = 1;
				lx.lc = nlc;
				lx.node = (pn->m_pi)->m_nid;
				lx.s = fn[lx.node-1].x;

				ly.bc = 2;
				ly.lc = nlc;
				ly.node = (pn->m_pi)->m_nid;
				ly.s = fn[ly.node-1].y;

				lz.bc = 3;
				lz.lc = nlc;
				lz.node = (pn->m_pi)->m_nid;
				lz.s = fn[lz.node-1].z;

				int nc = 0;
				if (lx.s != 0.0f) {	m_NF.push_back(lx); ++nc; }
				if (ly.s != 0.0f) {	m_NF.push_back(ly); ++nc; }
				if (lz.s != 0.0f) {	m_NF.push_back(lz); ++nc; }

				m_Ctrl.numcnl += nc;
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DProject::BuildPressureLoads(FSProject &prj)
{
	FSModel& fem = prj.GetFSModel();
	FSStep& s = *fem.GetStep(0);

	int i;
	for (i=0; i<s.Loads(); ++i)
	{
		FSPressureLoad* ppl = dynamic_cast<FSPressureLoad*>(s.Load(i));
		if (ppl)
		{
//			LoadCurve& lc = *ppl->GetLoadCurve(FSPressureLoad::LOAD);
			int nlc = -1;// AddLoadCurve(lc);
			
			PRESSURE_LOAD pl;
			FSItemListBuilder* pitem = ppl->GetItemList();
			unique_ptr<FSFaceList> pg(pitem->BuildFaceList());
			FSMesh* pm;
			FSFaceList::Iterator pf = pg->First();
			for (int k=0; k<pg->Size(); ++k, ++pf)
			{
				FSFace& f = *(pf->m_pi);
				pm = dynamic_cast<FSMesh*>(pf->m_pm);
				pl.lc = nlc;
				pl.n[0] = pm->Node(f.n[0]).m_nid;
				pl.n[1] = pm->Node(f.n[1]).m_nid;
				pl.n[2] = pm->Node(f.n[2]).m_nid;
				pl.n[3] = pm->Node(f.n[3]).m_nid;

				//pl.s[0] = 1;
				//pl.s[1] = 1;
				//pl.s[2] = 1;
				//pl.s[3] = 1;

				pl.s[0] = ppl->GetLoad();
				pl.s[1] = ppl->GetLoad();
				pl.s[2] = ppl->GetLoad();
				pl.s[3] = ppl->GetLoad();

				m_PF.push_back(pl);
				m_Ctrl.numpr++;
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DProject::BuildDisplacements(FSProject &prj)
{
	FSModel& fem = prj.GetFSModel();
	FSStep& s = *fem.GetStep(0);

	NODAL_DISPLACEMENT nd;
	for (int i=0; i<s.BCs(); ++i)
	{
		FSPrescribedDisplacement* pbc = dynamic_cast<FSPrescribedDisplacement*>(s.BC(i));
		if (pbc)
		{
//			LoadCurve& lc = *fem.GetParamCurve(pbc->GetParam(FSPrescribedDOF::SCALE));
			int nlc = -1;// AddLoadCurve(lc);
			int bc = pbc->GetDOF()+1;

			FSItemListBuilder* pitem = pbc->GetItemList();
			unique_ptr<FSNodeList> pg(pitem->BuildNodeList());
			FSNodeList::Iterator pn = pg->First();
			for (int k=0; k<pg->Size(); ++k, ++pn)
			{
				nd.bc = bc;
				nd.lc = nlc;
				nd.node = (pn->m_pi)->m_nid;
				nd.nstat = 0; 
				//nd.s = 1;
				nd.s = pbc->GetScaleFactor();
				m_DC.push_back(nd);
				m_Ctrl.numdis++;
			}
		}
		FSPrescribedRotation* prc = dynamic_cast<FSPrescribedRotation*>(s.BC(i));
		if (prc)
		{
			// TODO: implement this
			assert(false);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DProject::BuildBodyForce(FSProject& prj)
{
	FSModel& fem = prj.GetFSModel();
	FSStep& s = *fem.GetStep(0);

	for (int i=0; i<s.Loads(); ++i)
	{
		FSConstBodyForce* pbl = dynamic_cast<FSConstBodyForce*>(s.Load(i));
		if (pbl)
		{
			for (int j=0; j<3; ++j)
			{
				LoadCurve* plc = nullptr;// pbl->GetLoadCurve(j);
				if (plc)
				{
					int nlc = AddLoadCurve(*plc);
					BODY_FORCE bf;
					bf.lc = nlc;
					//bf.s  = 1;
					bf.s = pbl->GetLoad(j);
					m_BF.push_back(bf);
					m_Ctrl.bfa[j] = 1;
				}
			}
		}

		// We can only add one body load in NIKE so we have to quit here
		break;
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DProject::BuildNodalVelocities(FSProject &prj)
{
	FSModel& fem = prj.GetFSModel();
	FSStep& s = *fem.GetStep(0);

	m_Vel.resize(m_Ctrl.numnp);

	for (int i=0; i<s.BCs(); ++i)
	{
		FSNodalVelocities* pbc = dynamic_cast<FSNodalVelocities*>(s.BC(i));
		if (pbc)
		{
			vec3d v = pbc->GetVelocity();
			FSItemListBuilder* pitem = pbc->GetItemList();
			unique_ptr<FSNodeList> pg(pitem->BuildNodeList());
			FSNodeList::Iterator pn = pg->First();
			for (int k=0; k<pg->Size(); ++k, ++pn)
			{
				FSNode& node = *(pn->m_pi);
				NODAL_VELOCITY& nv = m_Vel[node.m_nid-1];

				nv.ninc = node.m_nid;
				nv.vx = v.x;
				nv.vy = v.y;
				nv.vz = v.z;
				nv.ninc = 0;

				m_Ctrl.intvel = 1;
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// convert project settings
//
bool NIKE3DProject::Convert(FSProject &prj)
{
	FSModel& fem = prj.GetFSModel();
	FSNonLinearMechanics* pstep = dynamic_cast<FSNonLinearMechanics*>(fem.GetStep(1));
	if (pstep == 0) return false;

	STEP_SETTINGS& set = pstep->GetSettings();
	CONTROL& c = m_Ctrl;

	set.Defaults();
	set.ntime = c.ntime;
	set.dt    = c.dt;
	set.dtmin = c.dtmin;
	set.dtmax = c.dtmax;
	set.bauto = (c.nauto == 1);
//	set.bshellstr = (c.istrn == 0? false : true);

	pstep->SetDisplacementTolerance(c.dtol);
	pstep->SetEnergyTolerance(c.ectl);
	pstep->SetResidualTolerance(c.rctl);
	pstep->SetLineSearchTolerance(c.tolls);
	set.ilimit = c.ilimit;
	set.iteopt = c.iteopt;
	set.maxref = c.maxref;
	set.mthsol = c.mthsol;
	set.mxback = c.mxback;
	set.nanalysis = c.imass;

	return true;
}
