#include "FENikeProject.h"
#include <MeshTools/FEProject.h>
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FERigidConstraint.h>
#include <MeshLib/MeshMetrics.h>
#include <FEMLib/FESurfaceLoad.h>
#include <GeomLib/GObject.h>
#include <MeshTools/GDiscreteObject.h>
#include <memory>
using namespace std;

//-----------------------------------------------------------------------------
// FENikeProject
//-----------------------------------------------------------------------------

bool FENikeProject::Create(FEProject &prj)
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

int FENikeProject::AddLoadCurve(FELoadCurve& lc)
{
	m_LC.push_back(lc);
	m_Ctrl.numlc++;
	int np = lc.Size();
	if (np > m_Ctrl.nptm) m_Ctrl.nptm = np;
	return (int)m_LC.size();
}

//-----------------------------------------------------------------------------

void FENikeProject::Defaults()
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
bool FENikeProject::BuildControl(FEProject& prj)
{
	CONTROL& c = m_Ctrl;
	FEModel& fem = prj.GetFEModel();

	STEP_SETTINGS set;
	FEAnalysisStep* pstep = 0;
	if (fem.Steps() > 1)
	{
		pstep = dynamic_cast<FEAnalysisStep*>(fem.GetStep(1));
		assert(pstep);
		set = pstep->GetSettings();
	}
	else set.Defaults();

	// make sure this is a mechanics step
	FENonLinearMechanics* pnlstep = dynamic_cast<FENonLinearMechanics*>(pstep);
	if (pnlstep == 0) return false;

	int nlc = -1;
	if (pstep)
	{
		FELoadCurve* plc = pstep->GetMustPointLoadCurve();
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

bool FENikeProject::BuildMaterials(FEProject& prj)
{
	int i, j, k;

	FEModel& fem = prj.GetFEModel();
	FEStep& step = *fem.GetStep(0);
	int nmat = fem.Materials();

	// update control data
	m_Ctrl.nmmat = nmat;

	// create materials
	m_Mat.resize(nmat);
	for (i=0; i<nmat; i++)
	{
		// get the material and tag it
		GMaterial* pgm = fem.GetMaterial(i);
		FEMaterial* pmat = pgm->GetMaterialProperties();
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
				FEIsotropicElastic* pm = dynamic_cast<FEIsotropicElastic*>(pmat);
				mat.m[0][0] = pm->GetParam(FEIsotropicElastic::MP_E).GetFloatValue();
				mat.m[1][0] = pm->GetParam(FEIsotropicElastic::MP_v).GetFloatValue();
				mat.dens = pm->GetParam(FEIsotropicElastic::MP_DENSITY).GetFloatValue();
			}
			break;
		case FE_MOONEY_RIVLIN:
			{
				mat.ntype = 15;
				FEMooneyRivlin* pm = dynamic_cast<FEMooneyRivlin*>(pmat);
				double A = pm->GetParam(FEMooneyRivlin::MP_A).GetFloatValue();
				double B = pm->GetParam(FEMooneyRivlin::MP_B).GetFloatValue();
				double K = pm->GetParam(FEMooneyRivlin::MP_K).GetFloatValue();

				mat.m[0][0] = A;
				mat.m[1][0] = B;
				mat.m[2][0] = (3*K - 4*(A+B))/(6*K+4*(A+B));
				mat.dens = pm->GetParam(FEMooneyRivlin::MP_DENSITY).GetFloatValue();
			}
			break;
		case FE_OGDEN_MATERIAL:
			{
				mat.ntype = 63;
				FEOgdenMaterial* pm = dynamic_cast<FEOgdenMaterial*>(pmat);
				double c[3], m[3];
				for (int i=0; i<3; ++i)
				{
					c[i] = pm->GetParam(FEOgdenMaterial::MP_C1+i).GetFloatValue();
					m[i] = pm->GetParam(FEOgdenMaterial::MP_M1+i).GetFloatValue();
				}
				mat.dens = pm->GetParam(FEOgdenMaterial::MP_DENSITY).GetFloatValue();

				mat.m[0][0] = pm->GetParam(FEOgdenMaterial::MP_K).GetFloatValue();
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
				FETransMooneyRivlin* pm = dynamic_cast<FETransMooneyRivlin*>(pmat);
				FEOldFiberMaterial& f = *pm->GetFiberMaterial();
				mat.m[0][0] = pm->GetFloatValue(FETransMooneyRivlin::MP_C1);
				mat.m[0][1] = pm->GetFloatValue(FETransMooneyRivlin::MP_C2);
				mat.m[0][2] = f.GetFloatValue(FETransMooneyRivlin::MP_C3);
				mat.m[0][3] = f.GetFloatValue(FETransMooneyRivlin::MP_C4);
				mat.m[0][4] = f.GetFloatValue(FETransMooneyRivlin::MP_C5);
						
				mat.m[1][0] = pm->GetFloatValue(FETransMooneyRivlin::MP_K);
				mat.m[1][1] = f.GetFloatValue(FETransMooneyRivlin::MP_LAM);

				mat.dens = pm->GetParam(FETransMooneyRivlin::MP_DENSITY).GetFloatValue();
					
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

//				FELoadCurve& ac = f.GetParam(FETransMooneyRivlin::Fiber::MP_AC).GetLoadCurve();
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
				FERigidMaterial* pm = dynamic_cast<FERigidMaterial*>(pmat);
				mat.dens = pm->GetFloatValue(FERigidMaterial::MP_DENSITY);

				mat.m[0][0] = pm->GetFloatValue(FERigidMaterial::MP_E);
				mat.m[1][0] = pm->GetFloatValue(FERigidMaterial::MP_V);

				// we need to loop over all the rigid constraints for this step
				// and set the material properties accordingly
				for (j=0; j<6; ++j) mat.m[2][j] = 0;
				for (k=0; k<step.RigidConstraints(); ++k)
				{
					FERigidConstraint& rc = *step.RigidConstraint(k);
					if (rc.GetMaterialID() == pgm->GetID())
					{
						switch (rc.Type())
						{
						case FE_RIGID_FIXED:
							{
								FERigidFixed& rf = dynamic_cast<FERigidFixed&>(rc);
								for (j=0; j<6; ++j)
									if (rf.GetDOF(j)) mat.m[2][j] = -1;
							}
							break;
						case FE_RIGID_DISPLACEMENT:
							{
								FERigidDisplacement& rf = dynamic_cast<FERigidDisplacement&>(rc);
								if (rf.GetDOF() >= 0) mat.m[2][rf.GetDOF()] = AddLoadCurve(*rf.GetLoadCurve());
							}
							break;
						}
					}
				}
				
				vec3d rc = pm->GetVecValue(FERigidMaterial::MP_RC);
				bool bcom = pm->GetBoolValue(FERigidMaterial::MP_COM);
				mat.m[3][0] = (bcom? 1 : 0);
				mat.m[3][1] = (bcom? rc.x: 0);
				mat.m[3][2] = (bcom? rc.y: 0);
				mat.m[3][3] = (bcom? rc.z: 0);
			}
			break;
		case FE_VISCO_ELASTIC:
			{
				mat.ntype = 18;
				FEViscoElastic* pm = dynamic_cast<FEViscoElastic*>(pmat);
				FEMaterial* psub = pm->GetElasticMaterial();

				int G1 = FEViscoElastic::MP_G1;
				int T1 = FEViscoElastic::MP_T1;

				mat.m[0][5] = pm->GetFloatValue(G1  ); mat.m[0][6] = pm->GetFloatValue(G1+1); mat.m[0][7] = pm->GetFloatValue(G1+2);
				mat.m[1][5] = pm->GetFloatValue(T1  ); mat.m[1][6] = pm->GetFloatValue(T1+1); mat.m[1][7] = pm->GetFloatValue(T1+2);
				mat.m[2][5] = pm->GetFloatValue(G1+3); mat.m[2][6] = pm->GetFloatValue(G1+4); mat.m[2][7] = pm->GetFloatValue(G1+5);
				mat.m[3][5] = pm->GetFloatValue(T1+3); mat.m[3][6] = pm->GetFloatValue(T1+4); mat.m[3][7] = pm->GetFloatValue(T1+5);

				if (psub && (psub->Type() == FE_MOONEY_RIVLIN))
				{
					FEMooneyRivlin* pmat = dynamic_cast<FEMooneyRivlin*>(psub);
					double A = pmat->GetParam(FEMooneyRivlin::MP_A).GetFloatValue();
					double B = pmat->GetParam(FEMooneyRivlin::MP_B).GetFloatValue();
					double K = pmat->GetParam(FEMooneyRivlin::MP_K).GetFloatValue();

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

bool FENikeProject::BuildNodes(FEProject& prj)
{
	FEModel& fem = prj.GetFEModel();
	GModel& model = fem.GetModel();
	FEStep& s = *fem.GetStep(0);

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
		FEMesh* pm = po->GetFEMesh();
		if (pm == 0) return false;
		for (int j=0; j<pm->Nodes(); ++j)
		{
			FENode& node = pm->Node(j);
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
		FEFixedDisplacement* pbc = dynamic_cast<FEFixedDisplacement*>(s.BC(i));
		if (pbc)
		{
			int bc;
			FEItemListBuilder* pitem = pbc->GetItemList();
			int nbc = pbc->GetBC();
			auto_ptr<FENodeList> pg(pitem->BuildNodeList());
			FENodeList::Iterator pn = pg->First();
			for (int k=0; k<pg->Size(); ++k, ++pn)
			{
				bc = (nbc & 7);
				NODE& node = m_Node[(pn->m_pi)->m_nid-1];
				node.bc |= bc;
			}
		}
		FEFixedRotation* prc = dynamic_cast<FEFixedRotation*>(s.BC(i));
		if (pbc)
		{
			int rc;
			FEItemListBuilder* pitem = pbc->GetItemList();
			int nbc = pbc->GetBC();
			auto_ptr<FENodeList> pg(pitem->BuildNodeList());
			FENodeList::Iterator pn = pg->First();
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

bool FENikeProject::BuildElements(FEProject &prj)
{
	FEModel& fem = prj.GetFEModel();
	GModel& model = fem.GetModel();

	int i, j, nmat = 0;
	// count nr of solid and shell elements
	int solids = 0;
	int shells = 0;
	for (i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FEMesh* pm = po->GetFEMesh();
		for (j=0; j<pm->Elements(); ++j)
		{
			FEElement& el = pm->Element(j);

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
		FEMesh* pm = po->GetFEMesh();
		for (j=0; j<pm->Elements(); ++j)
		{
			FEElement& el = pm->Element(j);
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

bool FENikeProject::BuildRigidNodes(FEProject &prj)
{
	FEModel& fem = prj.GetFEModel();
	FEStep& s = *fem.GetStep(0);

	for (int i=0; i<s.Interfaces(); ++i)
	{
		FERigidInterface* pi = dynamic_cast<FERigidInterface*>( s.Interface(i) );

		if (pi && pi->IsActive())
		{
			GMaterial* pgm = pi->GetRigidBody();
			FEMaterial* pm = pgm->GetMaterialProperties();
			int nRB = (pgm? pgm->m_ntag+1 : 0);
			RIGID_FACET rf;
			rf.nrb = nRB;
			rf.nsize = 4;
			int n = 0;

			FEItemListBuilder* pitem = pi->GetItemList();
			if (pitem)
			{
				auto_ptr<FENodeList> pg(pitem->BuildNodeList() );
				FENodeList::Iterator pn = pg->First();
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

bool FENikeProject::BuildDiscrete(FEProject& prj)
{
	FEModel& fem = prj.GetFEModel();
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

bool FENikeProject::BuildInterfaces(FEProject &prj)
{
	int i, k, n;

	FEModel& fem = prj.GetFEModel();
	FEStep& s = *fem.GetStep(0);

	// first we export all control data
	for (i=0; i<s.Interfaces(); ++i)
	{
		FESlidingWithGapsInterface* pi = dynamic_cast<FESlidingWithGapsInterface*>(s.Interface(i));

		// sliding interfaces
		if (pi && pi->IsActive())
		{
			SLIDING_INTERFACE si;
			SI_FACET f;

			bool bautopen = pi->GetBoolValue(FESlidingWithGapsInterface::AUTOPEN);
			bool twopass = pi->GetBoolValue(FESlidingWithGapsInterface::TWOPASS);
			double pen = pi->GetFloatValue(FESlidingWithGapsInterface::PENALTY);

			// set sliding interface parameters
			si.nns = 0;
			si.nms = 0;
			si.itype  = (twopass ? -3 : 3);	// TODO: check to see if this is correct
			si.pen    = (bautopen? pen : -pen);
			si.mus    = pi->GetFloatValue(FESlidingWithGapsInterface::MU);
			si.muk    = 0;
			si.fde    = 0;
			si.pend   = 0;
			si.bwrad  = 0;
			si.aicc   = 1;
			si.iaug   = 1;
			si.toln   = pi->GetFloatValue(FESlidingWithGapsInterface::ALTOL);
			si.tolt   = 0;
			si.tkmult = 0;
			si.tdeath = 0;
			si.tbury  = 0;

			// update control counter
			m_Ctrl.numsi++;

			// count slave faces
			n = 0;
			FEItemListBuilder* pss = pi->GetSlaveSurfaceList();
			if (pss)
			{
				auto_ptr<FEFaceList> pgs(pss->BuildFaceList());
				FEMesh* pm;
				FEFaceList::Iterator pf = pgs->First();
				for (k=0; k<pgs->Size(); ++k, ++pf)
				{
					FEFace& rf = *(pf->m_pi);
					pm = dynamic_cast<FEMesh*>(pf->m_pm);
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
			FEItemListBuilder* pms = pi->GetMasterSurfaceList();
			if (pms)
			{
				auto_ptr<FEFaceList> pgm(pms->BuildFaceList());
				FEMesh* pm;
				FEFaceList::Iterator pf = pgm->First();
				for (k=0; k<pgm->Size(); ++k, ++pf)
				{
					FEFace& rf = *(pf->m_pi);
					pm = dynamic_cast<FEMesh*>(pf->m_pm);
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
		FETiedInterface* pt = dynamic_cast<FETiedInterface*>(s.Interface(i));

		if (pt && pt->IsActive())
		{
			SLIDING_INTERFACE si;
			SI_FACET f;

			// set tied interface parameters
			si.nns = 0;
			si.nms = 0;
			si.itype  = 2;
			si.pen    = -pt->GetFloatValue(FETiedInterface::PENALTY);
			si.mus    = 0;
			si.muk    = 0;
			si.fde    = 0;
			si.pend   = 0;
			si.bwrad  = 0;
			si.aicc   = 1;
			si.iaug   = 1;
			si.toln   = pt->GetFloatValue(FETiedInterface::ALTOL);
			si.tolt   = 0;
			si.tkmult = 0;
			si.tdeath = 0;
			si.tbury  = 0;

			// update control counter
			m_Ctrl.numsi++;

			// count slave faces
			n = 0;
			FEItemListBuilder* pss = pt->GetSlaveSurfaceList();
			if (pss)
			{
				auto_ptr<FEFaceList> pgs(pss->BuildFaceList());
				FEMesh* pm;
				FEFaceList::Iterator pf = pgs->First();
				for (k=0; k<pgs->Size(); ++k, ++pf)
				{
					FEFace& rf = *(pf->m_pi);
					pm = dynamic_cast<FEMesh*>(pf->m_pm);
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
			FEItemListBuilder* pms = pt->GetMasterSurfaceList();
			if (pms)
			{
				auto_ptr<FEFaceList> pgm(pms->BuildFaceList());
				FEMesh* pm;
				FEFaceList::Iterator pf = pgm->First();
				for (k=0; k<pgm->Size(); ++k, ++pf)
				{
					FEFace& rf = *(pf->m_pi);
					pm = dynamic_cast<FEMesh*>(pf->m_pm);
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

bool FENikeProject::BuildNodalLoads(FEProject& prj)
{
	FEModel& fem = prj.GetFEModel();
	FEStep& s = *fem.GetStep(0);

	int i, k;
	for (i=0; i<s.Loads(); ++i)
	{
		FENodalLoad* pbc = dynamic_cast<FENodalLoad*>(s.Load(i));
		if (pbc)
		{
			FELoadCurve& lc = *pbc->GetLoadCurve();
			int nlc = AddLoadCurve(lc);
			int bc = pbc->GetBC() + 1;
			FEItemListBuilder* pitem = pbc->GetItemList();
			auto_ptr<FENodeList> pg(pitem->BuildNodeList() );
			FENodeList::Iterator pn = pg->First();
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

		FESurfaceTraction* ptc = dynamic_cast<FESurfaceTraction*>(s.Load(i));
		if (ptc)
		{
			FELoadCurve& lc = *ptc->GetLoadCurve();
			int nlc = AddLoadCurve(lc);

			FEItemListBuilder* pitem = ptc->GetItemList();
			auto_ptr<FEFaceList> ps(pitem->BuildFaceList());

			vec3d t = ptc->GetVecValue(FESurfaceTraction::LOAD);

			vector<vec3d> fn; fn.resize(m_Ctrl.numnp);
			FEFaceList::Iterator pf = ps->First();
			FEMesh* pm;
			for (k=0; k<ps->Size(); ++k, ++pf)
			{
				FEFace& face = *(pf->m_pi);
				pm = dynamic_cast<FEMesh*>(pf->m_pm);
				double a = FEMeshMetrics::SurfaceArea(*pm, face);

				double w = 1.0 / face.Nodes();

				for (int i=0; i<face.Nodes(); ++i)
				{
					fn[pm->Node(face.n[i]).m_nid-1].x += a*w*t.x;
					fn[pm->Node(face.n[i]).m_nid-1].y += a*w*t.y;
					fn[pm->Node(face.n[i]).m_nid-1].z += a*w*t.z;
				}
			}

			auto_ptr<FENodeList> pns(pitem->BuildNodeList());
			FENodeList::Iterator pn = pns->First();
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

bool FENikeProject::BuildPressureLoads(FEProject &prj)
{
	FEModel& fem = prj.GetFEModel();
	FEStep& s = *fem.GetStep(0);

	int i;
	for (i=0; i<s.Loads(); ++i)
	{
		FEPressureLoad* ppl = dynamic_cast<FEPressureLoad*>(s.Load(i));
		if (ppl)
		{
			FELoadCurve& lc = *ppl->GetLoadCurve();
			int nlc = AddLoadCurve(lc);
			
			PRESSURE_LOAD pl;
			FEItemListBuilder* pitem = ppl->GetItemList();
			auto_ptr<FEFaceList> pg(pitem->BuildFaceList());
			FEMesh* pm;
			FEFaceList::Iterator pf = pg->First();
			for (int k=0; k<pg->Size(); ++k, ++pf)
			{
				FEFace& f = *(pf->m_pi);
				pm = dynamic_cast<FEMesh*>(pf->m_pm);
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

bool FENikeProject::BuildDisplacements(FEProject &prj)
{
	FEModel& fem = prj.GetFEModel();
	FEStep& s = *fem.GetStep(0);

	NODAL_DISPLACEMENT nd;
	for (int i=0; i<s.BCs(); ++i)
	{
		FEPrescribedDisplacement* pbc = dynamic_cast<FEPrescribedDisplacement*>(s.BC(i));
		if (pbc)
		{
			FELoadCurve& lc = *pbc->GetLoadCurve();
			int nlc = AddLoadCurve(lc);
			int bc = pbc->GetDOF()+1;

			FEItemListBuilder* pitem = pbc->GetItemList();
			auto_ptr<FENodeList> pg(pitem->BuildNodeList());
			FENodeList::Iterator pn = pg->First();
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
		FEPrescribedRotation* prc = dynamic_cast<FEPrescribedRotation*>(s.BC(i));
		if (prc)
		{
			// TODO: implement this
			assert(false);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------

bool FENikeProject::BuildBodyForce(FEProject& prj)
{
	FEModel& fem = prj.GetFEModel();
	FEStep& s = *fem.GetStep(0);

	for (int i=0; i<s.Loads(); ++i)
	{
		FEBodyForce* pbl = dynamic_cast<FEBodyForce*>(s.Load(i));
		if (pbl)
		{
			for (int j=0; j<3; ++j)
			{
				FELoadCurve* plc = pbl->GetLoadCurve(j);
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

bool FENikeProject::BuildNodalVelocities(FEProject &prj)
{
	FEModel& fem = prj.GetFEModel();
	FEStep& s = *fem.GetStep(0);

	m_Vel.resize(m_Ctrl.numnp);

	for (int i=0; i<s.BCs(); ++i)
	{
		FENodalVelocities* pbc = dynamic_cast<FENodalVelocities*>(s.BC(i));
		if (pbc)
		{
			vec3d v = pbc->GetVelocity();
			FEItemListBuilder* pitem = pbc->GetItemList();
			auto_ptr<FENodeList> pg(pitem->BuildNodeList());
			FENodeList::Iterator pn = pg->First();
			for (int k=0; k<pg->Size(); ++k, ++pn)
			{
				FENode& node = *(pn->m_pi);
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
bool FENikeProject::Convert(FEProject &prj)
{
	FEModel& fem = prj.GetFEModel();
	FENonLinearMechanics* pstep = dynamic_cast<FENonLinearMechanics*>(fem.GetStep(1));
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
