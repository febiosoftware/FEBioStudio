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
#include "modelcheck.h"
#include <FEMLib/FSProject.h>
#include <GeomLib/GObject.h>
#include <FEMLib/FEMaterial.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FERigidLoad.h>
#include <GeomLib/GModel.h>
#include <stdarg.h>
#include <sstream>

using std::stringstream;

class FSObjectRef
{
public:
	FSObjectRef(FSObject* po) : m_po(po)
	{
		if (po) m_name = po->GetName();
	}

	FSObjectRef(std::string name, FSObject* po = nullptr) : m_po(po), m_name(name) {}

	const std::string& name() { return m_name; }

	FSObject* object() { return m_po; }

private:
	std::string m_name;
	FSObject* m_po;
};

typedef std::vector<FSObjectRef> FSObjectRefList;

// are there any objects?
void check_000(FSProject& prj, FSObjectRefList& objList);

// are all geometries meshed?
void check_001(FSProject& prj, FSObjectRefList& objList);

// are there any materials
void check_002(FSProject& prj, FSObjectRefList& objList);

// is a material assigned to all parts ?
void check_003(FSProject& prj, FSObjectRefList& objList);

// Are there any analysis steps? 
void check_004(FSProject& prj, FSObjectRefList& objList);

// check for unused materials
void check_005(FSProject& prj, FSObjectRefList& objList);

// check for unconstrained rigid bodies
void check_006(FSProject& prj, FSObjectRefList& objList);

// see if surfaces of solo interfaces are assigned.
void check_007(FSProject& prj, FSObjectRefList& objList);

// see if surfaces of paired interfaces are assigned.
void check_008(FSProject& prj, FSObjectRefList& objList);

// see if loads have assigned selections.
void check_009(FSProject& prj, FSObjectRefList& objList);

// see if BCs have assigned selections.
void check_010(FSProject& prj, FSObjectRefList& objList);

// see if ICs have assigned selections.
void check_011(FSProject& prj, FSObjectRefList& objList);

// see if there are any output variables defined
void check_012(FSProject& prj, FSObjectRefList& objList);

// see if shell thickness are zero for non-rigid parts
void check_013(FSProject& prj, FSObjectRefList& objList);

// do rigid connectors have two different rigid bodies
void check_014(FSProject& prj, FSObjectRefList& objList);

// do rigid connectors have two rigid bodies defined
void check_015(FSProject& prj, FSObjectRefList& objList);

// check if a rigid interface was assigned a rigid body
void check_016(FSProject& prj, FSObjectRefList& objList);

// check if parts have duplicate names
void check_017(FSProject& prj, FSObjectRefList& objList);

// are any required components missing?
void check_018(FSProject& prj, FSObjectRefList& objList);

typedef void(*ERROR_FUNC)(FSProject&, FSObjectRefList&);

struct ERROR_DATA
{
	ERROR_TYPE	errType;
	const char* errStr;
	ERROR_FUNC	errFnc;
};

vector<ERROR_DATA> error = {
	{ CRITICAL, "This model does not contain any geometry.", check_000},
	{ CRITICAL, "The object \"%s\" is not meshed."         , check_001 },
	{ CRITICAL, "There are no materials defined in this model.", check_002 },
	{ CRITICAL, "Part \"%s\" does not have a material assigned.", check_003 },
	{ CRITICAL, "There are no analysis steps defined.", check_004 },
	{ WARNING , "Material \"%s\" is not assigned to any parts.", check_005 },
	{ WARNING , "Rigid body \"%s\" might be underconstrained.", check_006 },
	{ CRITICAL, "No selection was assigned to interface \"%s\".", check_007 },
	{ CRITICAL, "One or both of the selections of interface \"%s\" is unassigned.", check_008 },
	{ CRITICAL, "Load \"%s\" has no selection assigned.", check_009 },
	{ CRITICAL, "Boundary condition \"%s\" has no selection assigned.", check_010 },
	{ CRITICAL, "Initial condition \"%s\" has no selection assigned.", check_011 },
	{ WARNING , "This model does not define any output variables.", check_012 },
	{ CRITICAL, "Some shells in part \"%s\" have zero thickness.", check_013 },
	{ WARNING , "Rigid connector \"%s\" connects the same rigid body.", check_014 },
	{ CRITICAL, "Rigid connector \"%s\" does not connect two rigid bodies.", check_015 },
	{ CRITICAL, "A rigid body was not assigned to rigid interface \"%s\".", check_016 },
	{ WARNING , "Some parts have the same name. \"%s\".", check_017 },
	{ CRITICAL, "Missing required component in \"%s\".", check_018 }
};

const char* errorString(int error_code)
{
	if (error_code >= error.size())
	{
		assert(false);
		return "undefined error";
	}
	else return error[error_code].errStr;
}

MODEL_ERROR make_error(int error_code, FSObjectRef& objref)
{
	// make the message
	char sztxt[1024] = { 0 };
	string s = objref.name();
	FSObject* po = objref.object();
	if (dynamic_cast<GPart*>(po))
	{
		GPart* pg = dynamic_cast<GPart*>(po);
		GBaseObject* o = pg->Object();
		if (o)
		{
			stringstream ss;
			ss << po->GetName() + " (" + o->GetName() + ")";
			s = ss.str();
		}
	}
	sprintf(sztxt, error[error_code].errStr, s.c_str());

	MODEL_ERROR err;
	err.first = error[error_code].errType;
	err.second = string(sztxt);
	return err;
}

void checkModel(FSProject& prj, std::vector<MODEL_ERROR>& errorList)
{
	for (size_t i = 0; i < error.size(); ++i)
	{
		FSObjectRefList objList;
		error[i].errFnc(prj, objList);

		for (size_t j = 0; j < objList.size(); ++j)
		{
			errorList.push_back(make_error(i, objList[j]));
		}
	}
}


void check_000(FSProject& prj, FSObjectRefList& objList)
{
	// are there any objects?
	GModel& mdl = prj.GetFSModel().GetModel();
	if (mdl.Objects() == 0)
	{
		objList.push_back(&mdl);
	}
}

void check_001(FSProject& prj, FSObjectRefList& objList)
{
	GModel& mdl = prj.GetFSModel().GetModel();
	for (int i = 0; i < mdl.Objects(); ++i)
	{
		GObject* po = mdl.Object(i);
		if (po->GetFEMesh() == nullptr)
		{
			objList.push_back(po);
		}
	}
}

void check_002(FSProject& prj, FSObjectRefList& objList)
{
	FSModel& fem = prj.GetFSModel();
	if (fem.Materials() == 0)
	{
		objList.push_back(&fem);
	}
}

void check_003(FSProject& prj, FSObjectRefList& objList)
{
	FSModel& fem = prj.GetFSModel();

	// build a material lookup table
	int minId = 0;
	int mats = 0;
	vector<int> lut;
	if (fem.Materials() > 0)
	{
		int maxId;
		for (int i = 0; i < fem.Materials(); ++i)
		{
			GMaterial* gm = fem.GetMaterial(i); assert(gm);
			if (gm)
			{
				int matId = gm->GetID();
				if ((i == 0) || (matId < minId)) minId = matId;
				if ((i == 0) || (matId > maxId)) maxId = matId;
			}
		}
		mats = maxId - minId + 1;
		lut.assign(mats, -1);
		for (int i = 0; i < fem.Materials(); ++i)
		{
			GMaterial* gm = fem.GetMaterial(i); assert(gm);
			if (gm)
			{
				int matId = gm->GetID() - minId;
				assert(matId < mats);
				if (matId < mats)
				{
					assert(lut[matId] == -1);
					lut[matId] = i;
				}
			}
		}
	}

	GModel& mdl = prj.GetFSModel().GetModel();
	for (int i = 0; i < mdl.Objects(); ++i)
	{
		GObject* po = mdl.Object(i);
		for (int j = 0; j < po->Parts(); ++j)
		{
			GPart* pj = po->Part(j);
			int matId = pj->GetMaterialID() - minId;
			if ((matId < 0) || (matId >= mats) || (lut[matId] == -1))
			{
				objList.push_back(pj);
			}
		}
	}
}

void check_004(FSProject& prj, FSObjectRefList& objList)
{
	FSModel& fem = prj.GetFSModel();
	if (fem.Steps() <= 1)
	{
		objList.push_back(&fem);
	}
}

void check_005(FSProject& prj, FSObjectRefList& objList)
{
	FSModel& fem = prj.GetFSModel();
	GModel& mdl = fem.GetModel();
	for (int i = 0; i < fem.Materials(); ++i)
	{
		GMaterial* mat = fem.GetMaterial(i);

		// see if this material is used
		list<GPart*> partList = mdl.FindPartsFromMaterial(mat->GetID());
		if (partList.empty())
		{
			objList.push_back(mat);
		}
	}
}

void check_006(FSProject& prj, FSObjectRefList& objList)
{
	FSModel& fem = prj.GetFSModel();
	GModel& mdl = fem.GetModel();
	for (int i = 0; i < fem.Materials(); ++i)
	{
		GMaterial* mat = fem.GetMaterial(i);
		if (mat->GetMaterialProperties()->IsRigid())
		{
			int matId = mat->GetID();
			bool matUsed = false;
			for (int n = 0; n < fem.Steps(); ++n)
			{
				FSStep* pstep = fem.GetStep(n);
				
				// see if this material is referenced by any rigid constraints
				for (int j = 0; j < pstep->RigidBCs(); ++j)
				{
					FSRigidBC* prc = pstep->RigidBC(j);
					if (prc->GetMaterialID() == matId)
					{
						matUsed = true;
						break;
					}
				}
				for (int j = 0; j < pstep->RigidICs(); ++j)
				{
					FSRigidIC* prc = pstep->RigidIC(j);
					if (prc->GetMaterialID() == matId)
					{
						matUsed = true;
						break;
					}
				}
				for (int j = 0; j < pstep->RigidLoads(); ++j)
				{
					FSRigidLoad* prc = pstep->RigidLoad(j);
					if (prc->GetMaterialID() == matId)
					{
						matUsed = true;
						break;
					}
				}
				if (matUsed) break;

				// see if this material is referenced by any rigid connector
				for (int j = 0; j < pstep->RigidConnectors(); ++j)
				{
					FSRigidConnector* prc = pstep->RigidConnector(j);
					if ((prc->GetRigidBody1() == matId) || (prc->GetRigidBody2() == matId))
					{
						matUsed = true;
						break;
					}
				}
				if (matUsed) break;

				// see if this material is used by any rigid interface
				for (int j = 0; j < pstep->Interfaces(); ++j)
				{
					FSRigidInterface* ri = dynamic_cast<FSRigidInterface*>(pstep->Interface(j));
					if (ri && (ri->GetRigidBody() == mat))
					{
						matUsed = true;
						break;
					}
				}

				// see if this material is used by a part that is connected to another part
				for (int j = 0; j < mdl.Objects(); ++j)
				{
					GObject* po = mdl.Object(j);
					for (int k = 0; k < po->Faces(); ++k)
					{
						GFace* face = po->Face(k);
						for (int l = 0; l < 3; ++l)
						{
							GPart* pgl = po->Part(face->m_nPID[l]);
							if (pgl && (pgl->GetMaterialID() == matId))
							{
								matUsed = true;
							}
						}
					}
					if (matUsed) break;
				}
				if (matUsed) break;
			}

			if (matUsed == false)
			{
				objList.push_back(mat);
			}
		}
	}
}

// see if surfaces of solo interfaces are assigned.
void check_007(FSProject& prj, FSObjectRefList& objList)
{
	FSModel& fem = prj.GetFSModel();
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* step = fem.GetStep(i);
		for (int j = 0; j < step->Interfaces(); ++j)
		{
			FSInterface* pi = step->Interface(j);
			if (dynamic_cast<FSSoloInterface*>(pi))
			{
				FSSoloInterface* psi = dynamic_cast<FSSoloInterface*>(pi);
				if (psi->GetItemList() == nullptr)
				{
					objList.push_back(pi);
				}
			}
		}
	}
}

// see if surfaces of paired interfaces are assigned.
void check_008(FSProject& prj, FSObjectRefList& objList)
{
	FSModel& fem = prj.GetFSModel();
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* step = fem.GetStep(i);
		for (int j = 0; j < step->Interfaces(); ++j)
		{
			FSInterface* pi = step->Interface(j);
			if (dynamic_cast<FSPairedInterface*>(pi))
			{
				FSPairedInterface* psi = dynamic_cast<FSPairedInterface*>(pi);
				if ((psi->GetPrimarySurface() == nullptr) || (psi->GetSecondarySurface() == nullptr))
				{
					objList.push_back(pi);
				}
			}
		}
	}
}

// see if loads have assigned selections.
void check_009(FSProject& prj, FSObjectRefList& objList)
{
	FSModel& fem = prj.GetFSModel();
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* step = fem.GetStep(i);
		for (int j = 0; j < step->Loads(); ++j)
		{
			FSLoad* pl = step->Load(j);
			if ((dynamic_cast<FSBodyLoad*>(pl) == nullptr) && (pl->GetItemList() == nullptr))
			{
				objList.push_back(pl);
			}
		}
	}
}

// see if BCs have assigned selections.
void check_010(FSProject& prj, FSObjectRefList& objList)
{
	FSModel& fem = prj.GetFSModel();
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* step = fem.GetStep(i);
		for (int j = 0; j < step->BCs(); ++j)
		{
			FSBoundaryCondition* pl = step->BC(j);
			if ((pl->GetMeshItemType() != 0) && (pl->GetItemList() == nullptr))
			{
				objList.push_back(pl);
			}
		}
	}
}

// see if ICs have assigned selections.
void check_011(FSProject& prj, FSObjectRefList& objList)
{
	FSModel& fem = prj.GetFSModel();
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* step = fem.GetStep(i);
		for (int j = 0; j < step->ICs(); ++j)
		{
			FSInitialNodalDOF* pl = dynamic_cast<FSInitialNodalDOF*>(step->IC(j));
			if (pl && pl->GetItemList() == nullptr)
			{
				objList.push_back(pl);
			}
		}
	}
}

// see if there are any output variables defined
void check_012(FSProject& prj, FSObjectRefList& objList)
{
	FSModel& fem = prj.GetFSModel();
	CPlotDataSettings& plt = prj.GetPlotDataSettings();

	// count the nr of active plot variables
	int na = 0;
	for (int i = 0; i<plt.PlotVariables(); ++i) if (plt.PlotVariable(i).isActive()) na++;

	if (na == 0)
	{
		objList.push_back(&fem);
	}
}

// see if shell thickness are zero for non-rigid parts
void check_013(FSProject& prj, FSObjectRefList& objList)
{
	FSModel& fem = prj.GetFSModel();
	GModel& mdl = fem.GetModel();
	for (int i = 0; i < mdl.Objects(); ++i)
	{
		GObject* obj_i = mdl.Object(i);
		FSMesh* pm = obj_i->GetFEMesh();
		if (pm)
		{
			int parts = obj_i->Parts();
			for (int n = 0; n < parts; ++n)
			{
				GPart* pg = obj_i->Part(n);

				double h = 0.0;
				GShellSection* shell = dynamic_cast<GShellSection*>(pg->GetSection());
				if (shell) h = shell->shellThickness();

				int zeroShells = 0;
				int NE = pm->Elements();
				for (int j = 0; j < NE; ++j)
				{
					FSElement& el = pm->Element(j);
					if (el.IsShell() && (el.m_gid == n))
					{
						int ne = el.Nodes();
						for (int k = 0; k < ne; ++k)
						{
							if ((h == 0) && (el.m_h[k] == 0.0)) zeroShells++;
						}
					}
					if (zeroShells) break;
				}

				if (zeroShells > 0)
				{
					// see if the material is rigid
					int mid = pg->GetMaterialID();
					GMaterial* pm = fem.GetMaterialFromID(mid);
					if (pm)
					{
						FSMaterial* mat = pm->GetMaterialProperties();
						if (mat && mat->IsRigid())
						{
							// we allow zero thickness for rigid parts
							zeroShells = 0;
						}
					}
				}

				if (zeroShells)
				{
					objList.push_back(pg);
				}
			}
		}
	}
}

// do rigid connectors have two different rigid bodies
void check_014(FSProject& prj, FSObjectRefList& objList)
{
	FSModel& fem = prj.GetFSModel();
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		int nrc = pstep->RigidConnectors();
		for (int j = 0; j < nrc; ++j)
		{
			FSRigidConnector* rc = pstep->RigidConnector(j);
			if (rc->GetRigidBody1() == rc->GetRigidBody2())
			{
				objList.push_back(rc);
			}
		}
	}
}

// do rigid connectors have two rigid bodies defined
void check_015(FSProject& prj, FSObjectRefList& objList)
{
	FSModel& fem = prj.GetFSModel();
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		int nrc = pstep->RigidConnectors();
		for (int j = 0; j < nrc; ++j)
		{
			FSRigidConnector* rc = pstep->RigidConnector(j);
			if ((rc->GetRigidBody1() == -1) || (rc->GetRigidBody2() == -1))
			{
				objList.push_back(rc);
			}
		}
	}
}

// check if a rigid interface was assigned a rigid body
void check_016(FSProject& prj, FSObjectRefList& objList)
{
	FSModel& fem = prj.GetFSModel();
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		int ni = pstep->Interfaces();
		for (int j = 0; j < ni; ++j)
		{
			FSRigidInterface* ri = dynamic_cast<FSRigidInterface*>(pstep->Interface(j));
			if (ri && (ri->GetRigidBody() == nullptr))
			{
				objList.push_back(ri);
			}
		}
	}
}

// check if parts have duplicate names
void check_017(FSProject& prj, FSObjectRefList& objList)
{
	FSModel& fem = prj.GetFSModel();
	GModel& gm = fem.GetModel();

	for (int i = 0; i < gm.Objects(); ++i)
	{
		GObject* poi = gm.Object(i);
		for (int j = 0; j < poi->Parts(); ++j)
		{
			GPart* pj = poi->Part(j);

			for (int k = 0; k < gm.Objects(); ++k)
			{
				GObject* pok = gm.Object(k);
				for (int l = 0; l < pok->Parts(); ++l)
				{
					GPart* pl = pok->Part(l);

					if ((pl != pj) && (pj->GetName() == pl->GetName()))
					{
						objList.push_back(pj);
					}
				}
			}
		}
	}
}

//=============================================================================
// check for missing required components
void check_required_components(FSModelComponent* pc, FSObjectRefList& objList, const std::string& rootName)
{
	for (int j = 0; j < pc->Properties(); ++j)
	{
		FSProperty& pj = pc->GetProperty(j);
		if (pj.IsRequired())
		{
			for (int k = 0; k < pj.Size(); ++k)
			{
				FSModelComponent* pck = dynamic_cast<FSModelComponent*>(pj.GetComponent(k));
				if (pck == nullptr)
				{
					string name = rootName + ".";
					name += pj.GetName();
					objList.push_back(FSObjectRef(name));
					break;
				}
				else
				{
					string name = rootName + ".";
					name += pj.GetName();
					check_required_components(pck, objList, name);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// are any required components missing?
void check_018(FSProject& prj, FSObjectRefList& objList)
{
	FSModel& fem = prj.GetFSModel();
	for (int i = 0; i < fem.Materials(); ++i)
	{
		GMaterial* gmat = fem.GetMaterial(i);
		FSMaterial* pm = gmat->GetMaterialProperties();
		if (pm == nullptr) objList.push_back(gmat);
		else check_required_components(pm, objList, gmat->GetName());
	}

	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* step = fem.GetStep(i);
		check_required_components(step, objList, step->GetName());
	}
}
