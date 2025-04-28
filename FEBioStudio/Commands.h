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

#pragma once
#include "Command.h"
#include <MeshTools/FEModifier.h>
#include <FEMLib/FEInterface.h>
#include <GeomLib/GObject.h>
#include <FEMLib/FEBoundaryCondition.h>
#include <FEMLib/FEConnector.h>
#include <FEMLib/FELoad.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FSModel.h>
#include <FSCore/ParamBlock.h>
#include <GeomLib/GGroup.h>
#include <FEMLib/GDiscreteObject.h>
#include <GeomLib/GMeshObject.h>
#include <MeshTools/GModifiedObject.h>
#include <MeshTools/FESurfaceModifier.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <GLLib/GLCamera.h>

class ObjectMeshList;
class MeshLayer;
class CModelDocument;
class CGLDocument;
class CGView;
class CImageAnalysis;
class GMaterial;

class FSRigidLoad;
class FSRigidBC;
class FSRigidIC;
class FSMeshAdaptor;
class FSLoadController;
class FSMeshData;
class FSMeshDataGenerator;

//-----------------------------------------------------------------------------

class CCmdAddObject : public CCommand
{
public:
	CCmdAddObject(GModel* model, GObject* po);
	~CCmdAddObject();

	void Execute();
	void UnExecute();

protected:
	GModel*		m_model;
	GObject*	m_pobj;
	bool		m_bdel;
};

//-----------------------------------------------------------------------------

class CCmdAddDiscreteObject : public CCommand
{
public:
	CCmdAddDiscreteObject(GModel* model, GDiscreteObject* po);
	~CCmdAddDiscreteObject();

	void Execute();
	void UnExecute();

protected:
	GModel*				m_model;
	GDiscreteObject*	m_pobj;
	bool				m_bdel;
};

//-----------------------------------------------------------------------------

class CCmdAddInterface : public CCommand
{
public:
	CCmdAddInterface(FSStep* ps, FSInterface* pi) : CCommand("Add interface") { m_ps = ps; m_pint = pi; m_bdel = true; }
	~CCmdAddInterface() { if (m_bdel) delete m_pint; }

	void Execute();
	void UnExecute();

protected:
	FSStep*			m_ps;
	FSInterface*	m_pint;
	bool			m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddRigidConnector : public CCommand
{
public:
	CCmdAddRigidConnector(FSStep* ps, FSRigidConnector* pi) : CCommand("Add rigid connector") { m_ps = ps; m_pint = pi; m_bdel = true; }
	~CCmdAddRigidConnector() { if (m_bdel) delete m_pint; }

	void Execute();
	void UnExecute();

protected:
	FSStep*			m_ps;
	FSRigidConnector*	m_pint;
	bool			m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddConstraint : public CCommand
{
public:
	CCmdAddConstraint(FSStep* ps, FSModelConstraint* pmc);
	~CCmdAddConstraint();

	void Execute();
	void UnExecute();

protected:
	FSStep*				m_ps;
	FSModelConstraint*	m_pmc;
	bool				m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddRigidLoad : public CCommand
{
public:
	CCmdAddRigidLoad(FSStep* ps, FSRigidLoad* pmc);
	~CCmdAddRigidLoad();

	void Execute();
	void UnExecute();

protected:
	FSStep*			m_ps;
	FSRigidLoad*	m_prl;
	bool			m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddRigidBC : public CCommand
{
public:
	CCmdAddRigidBC(FSStep* ps, FSRigidBC* pmc);
	~CCmdAddRigidBC();

	void Execute();
	void UnExecute();

protected:
	FSStep*		m_ps;
	FSRigidBC*	m_prc;
	bool		m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddRigidIC : public CCommand
{
public:
	CCmdAddRigidIC(FSStep* ps, FSRigidIC* pmc);
	~CCmdAddRigidIC();

	void Execute();
	void UnExecute();

protected:
	FSStep*		m_ps;
	FSRigidIC*	m_prc;
	bool		m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddMeshAdaptor : public CCommand
{
public:
	CCmdAddMeshAdaptor(FSStep* ps, FSMeshAdaptor* pmc);
	~CCmdAddMeshAdaptor();

	void Execute();
	void UnExecute();

protected:
	FSStep* m_ps;
	FSMeshAdaptor* m_pma;
	bool		m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddLoadController : public CCommand
{
public:
	CCmdAddLoadController(FSModel* fem, FSLoadController* pmc);
	~CCmdAddLoadController();

	void Execute();
	void UnExecute();

protected:
	FSModel*			m_fem;
	FSLoadController*	m_plc;
	bool				m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddMeshDataField : public CCommand
{
public:
	CCmdAddMeshDataField(FSMesh* pm, FSMeshData* pmd);
	~CCmdAddMeshDataField();

	void Execute();
	void UnExecute();

protected:
	FSMesh* m_pm;
	FSMeshData* m_pmd;
	bool		m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddMeshDataGenerator : public CCommand
{
public:
	CCmdAddMeshDataGenerator(FSModel* fem, FSMeshDataGenerator* pmd);
	~CCmdAddMeshDataGenerator();

	void Execute();
	void UnExecute();

protected:
	FSModel* m_fem;
	FSMeshDataGenerator* m_pmd;
	bool				m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddPart : public CCommand
{
public:
	CCmdAddPart(GObject* po, FSElemSet* pg) : CCommand("Add Element Set") { m_po = po; m_pg = pg; m_bdel = true; }
	~CCmdAddPart() { if (m_bdel) delete m_pg; }

	void Execute();
	void UnExecute();

protected:
	GObject*		m_po;
	FSElemSet*		m_pg;
	bool		m_bdel;
};

//-----------------------------------------------------------------------------

class CCmdAddSurface : public CCommand
{
public:
	CCmdAddSurface(GObject* po, FSSurface* pg) : CCommand("Add Surface") { m_po = po; m_pg = pg; m_bdel = true; }
	~CCmdAddSurface() { if (m_bdel) delete m_pg; }

	void Execute();
	void UnExecute();

protected:
	GObject*	m_po;
	FSSurface*	m_pg;
	bool		m_bdel;
};

//-----------------------------------------------------------------------------

class CCmdAddFEEdgeSet : public CCommand
{
public:
	CCmdAddFEEdgeSet(GObject* po, FSEdgeSet* pg) : CCommand("Add EdgeSet") { m_po = po; m_pg = pg; m_bdel = true; }
	~CCmdAddFEEdgeSet() { if (m_bdel) delete m_pg; }

	void Execute();
	void UnExecute();

protected:
	GObject*	m_po;
	FSEdgeSet*	m_pg;
	bool		m_bdel;
};

//-----------------------------------------------------------------------------

class CCmdAddNodeSet : public CCommand
{
public:
	CCmdAddNodeSet(GObject* po, FSNodeSet* pg) : CCommand("Add Nodeset") { m_po = po; m_pg = pg; m_bdel = true; }
	~CCmdAddNodeSet() { if (m_bdel) delete m_pg; }

	void Execute();
	void UnExecute();

protected:
	GObject*	m_po;
	FSNodeSet*	m_pg;
	bool		m_bdel;
};

//-----------------------------------------------------------------------------

class CCmdAddGPartGroup : public CCommand
{
public:
	CCmdAddGPartGroup(GModel* model, GPartList* pg);
	~CCmdAddGPartGroup();

	void Execute();
	void UnExecute();

protected:
	GModel*			m_model;
	GPartList*		m_pg;
	bool			m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddGFaceGroup : public CCommand
{
public:
	CCmdAddGFaceGroup(GModel* model, GFaceList* pg);
	~CCmdAddGFaceGroup();

	void Execute();
	void UnExecute();

protected:
	GModel*			m_model;
	GFaceList*		m_pg;
	bool			m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddGEdgeGroup : public CCommand
{
public:
	CCmdAddGEdgeGroup(GModel* model, GEdgeList* pg);
	~CCmdAddGEdgeGroup();

	void Execute();
	void UnExecute();

protected:
	GModel*			m_model;
	GEdgeList*		m_pg;
	bool			m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddGNodeGroup : public CCommand
{
public:
	CCmdAddGNodeGroup(GModel* model, GNodeList* pg);
	~CCmdAddGNodeGroup();

	void Execute();
	void UnExecute();

protected:
	GModel*			m_model;
	GNodeList*		m_pg;
	bool			m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddBC : public CCommand
{
public:
	CCmdAddBC(FSStep* ps, FSBoundaryCondition* pbc) : CCommand("Add Boundary Condition") { m_ps = ps; m_pbc = pbc; m_bdel = true; }
	~CCmdAddBC() { if (m_bdel) delete m_pbc; }

	void Execute();
	void UnExecute();

protected:
	FSStep*					m_ps;
	FSBoundaryCondition*	m_pbc;
	bool					m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddIC : public CCommand
{
public:
	CCmdAddIC(FSStep* ps, FSInitialCondition* pic) : CCommand("Add Initial Condition") { m_ps = ps; m_pic = pic; m_bdel = true; }
	~CCmdAddIC() { if (m_bdel) delete m_pic; }

	void Execute();
	void UnExecute();

protected:
	FSStep*					m_ps;
	FSInitialCondition*		m_pic;
	bool					m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddLoad : public CCommand
{
public:
	CCmdAddLoad(FSStep* ps, FSLoad* pfc) : CCommand("Add Load") { m_ps = ps; m_pfc = pfc; m_bdel = true; }
	~CCmdAddLoad() { if (m_bdel) delete m_pfc; }

	void Execute();
	void UnExecute();

protected:
	FSStep*		m_ps;
	FSLoad*		m_pfc;
	bool		m_bdel;
};

//----------------------------------------------------------------
class CCmdDeleteDiscreteObject : public CCommand
{
public:
	CCmdDeleteDiscreteObject(GModel* model, GDiscreteObject* po);
	~CCmdDeleteDiscreteObject();

	void Execute();
	void UnExecute();

protected:
	GModel*				m_model;
	GDiscreteObject*	m_pobj;
	bool	m_bdel;
	int		m_npos;
};

//-----------------------------------------------------------------------------
class CCmdTransformObject : public CCommand
{
public:
	CCmdTransformObject(GObject* po, const Transform& Q);

	void Execute() override;
	void UnExecute() override;

private:
	GObject* m_po;
	Transform	m_oldQ;
};

//-----------------------------------------------------------------------------
class CCmdTranslateSelection : public CCommand
{
public:
	CCmdTranslateSelection(CGLDocument* doc, vec3d dr);

	void Execute();
	void UnExecute();

protected:
	CGLDocument* m_doc;
	vec3d	m_dr;
};

//-----------------------------------------------------------------------------

class CCmdRotateSelection : public CCommand
{
public:
	CCmdRotateSelection(CGLDocument* doc, quatd q, vec3d rc);

	void Execute();
	void UnExecute();

protected:
	CGLDocument*	m_doc;
	quatd	m_q;
	vec3d	m_rc;
};

//-----------------------------------------------------------------------------

class CCmdScaleSelection : public CCommand
{
public:
	CCmdScaleSelection(CGLDocument* doc, double s, vec3d dr, vec3d rc);

	void Execute();
	void UnExecute();

protected:
	CGLDocument*	m_doc;
	double	m_s;
	vec3d	m_dr;
	vec3d	m_rc;
};

//-----------------------------------------------------------------------------
class CCmdToggleObjectVisibility : public CCommand
{
public:
	CCmdToggleObjectVisibility(GModel* model);

	void Execute();
	void UnExecute();

private:
	GModel*	m_model;
};

//-----------------------------------------------------------------------------
class CCmdTogglePartVisibility : public CCommand
{
public:
	CCmdTogglePartVisibility(GModel* model);

	void Execute();
	void UnExecute();

private:
	GModel*	m_model;
};

//-----------------------------------------------------------------------------
class CCmdToggleDiscreteVisibility : public CCommand
{
public:
	CCmdToggleDiscreteVisibility(GModel* model);

	void Execute();
	void UnExecute();

private:
	GModel*	m_model;
};

//-----------------------------------------------------------------------------
class CCmdToggleElementVisibility : public CCommand
{
public:
	CCmdToggleElementVisibility(GObject* po);

	void Execute();
	void UnExecute();

private:
	GObject* m_po;
	FSMesh*	m_mesh;
};

//-----------------------------------------------------------------------------
class CCmdToggleFEFaceVisibility : public CCommand
{
public:
	CCmdToggleFEFaceVisibility(FSMeshBase* mesh);

	void Execute();
	void UnExecute();

private:
	FSMeshBase*	m_mesh;
};

//-----------------------------------------------------------------------------

class CCmdSelectObject : public CCommand
{
public:
	CCmdSelectObject(GModel* model, GObject* po, bool badd);
	CCmdSelectObject(GModel* model, GObject** ppo, int n, bool badd);
	CCmdSelectObject(GModel* model, const std::vector<GObject*>& po, bool badd);
	~CCmdSelectObject() { delete[] m_ptag; delete[] m_ppo; }

	void Execute();
	void UnExecute();

protected:
	GModel*		m_model;
	bool*		m_ptag;	// old selection state of meshes
	GObject**	m_ppo;	// pointers to objects we need to select
	bool		m_badd; // add meshes to selection or not
	int			m_N;	// Nr of meshes to select
};

//-----------------------------------------------------------------------------

class CCmdUnselectObject : public CCommand
{
public:
	CCmdUnselectObject(GModel* model, GObject* po);
	CCmdUnselectObject(GModel* model, GObject** ppo, int n);
	CCmdUnselectObject(GModel* model, const std::vector<GObject*>& po);
	~CCmdUnselectObject() { delete[] m_ptag; delete[] m_ppo; }

	void Execute();
	void UnExecute();

protected:
	GModel*		m_model;
	bool*		m_ptag;	// old selection state of meshes
	GObject**	m_ppo;	// pointers to meshes we need to unselect
	int			m_N;	// Nr of meshes to select
};


//-----------------------------------------------------------------------------

class CCmdSelectPart : public CCommand
{
public:
	CCmdSelectPart(GModel* model, int* npart, int n, bool badd);
	CCmdSelectPart(GModel* model, const std::vector<int>& part, bool badd);

	void Execute();
	void UnExecute();

protected:
	GModel*			m_model;	// pointer to model
	std::vector<int>		m_npart;	// list of parts to select
	std::vector<bool>	m_bold;		// old selection state of parts
	bool			m_badd;		// add to current selection
};

//-----------------------------------------------------------------------------

class CCmdUnSelectPart : public CCommand
{
public:
	CCmdUnSelectPart(GModel* model, int* npart, int n);
	CCmdUnSelectPart(GModel* model, const std::vector<int>& part);

	void Execute();
	void UnExecute();

protected:
	GModel*			m_model;	// pointer to model
	std::vector<int>		m_npart;	// list of parts to select
	std::vector<bool>	m_bold;		// old selection state of parts
};

//-----------------------------------------------------------------------------

class CCmdSelectSurface : public CCommand
{
public:
	CCmdSelectSurface(GModel* model, int* nsurf, int n, bool badd);
	CCmdSelectSurface(GModel* model, const std::vector<int>& surf, bool badd);

	void Execute();
	void UnExecute();

protected:
	GModel*			m_model;	// pointer to model
	std::vector<int>		m_nsurf;	// list of surfaces to select
	std::vector<bool>	m_bold;		// old selection state of surfaces
	bool			m_badd;		// add to current selection
};

//-----------------------------------------------------------------------------

class CCmdUnSelectSurface : public CCommand
{
public:
	CCmdUnSelectSurface(GModel* ps, int* nsurf, int n);
	CCmdUnSelectSurface(GModel* ps, const std::vector<int>& surf);

	void Execute();
	void UnExecute();

protected:
	GModel*			m_model;	// pointer to model
	std::vector<int>		m_nsurf;	// list of surfaces to select
	std::vector<bool>	m_bold;		// old selection state of surfaces
	bool			m_badd;		// add to current selection
};

//-----------------------------------------------------------------------------

class CCmdSelectEdge : public CCommand
{
public:
	CCmdSelectEdge(GModel* ps, int* nedge, int n, bool badd);
	CCmdSelectEdge(GModel* ps, const std::vector<int>& edge, bool badd);

	void Execute();
	void UnExecute();

protected:
	GModel*			m_model;	// pointer to model
	std::vector<int>		m_nedge;	// list of edges to select
	std::vector<bool>	m_bold;		// old selection state of surfaces
	bool			m_badd;		// add to current selection
};

//-----------------------------------------------------------------------------

class CCmdUnSelectEdge : public CCommand
{
public:
	CCmdUnSelectEdge(GModel* model, int* nedge, int n);
	CCmdUnSelectEdge(GModel* model, const std::vector<int>& edge);

	void Execute();
	void UnExecute();

protected:
	GModel*			m_model;	// pointer to model
	std::vector<int>		m_nedge;	// list of edges to select
	std::vector<bool>	m_bold;		// old selection state of surfaces
	bool			m_badd;		// add to current selection
};

//-----------------------------------------------------------------------------

class CCmdSelectNode : public CCommand
{
public:
	CCmdSelectNode(GModel* model, int* node, int n, bool badd);
	CCmdSelectNode(GModel* model, const std::vector<int>& node, bool badd);

	void Execute();
	void UnExecute();

protected:
	GModel*			m_model;	// pointer to model
	std::vector<int>		m_node;		// list of edges to select
	std::vector<bool>	m_bold;		// old selection state of surfaces
	bool			m_badd;		// add to current selection
};

//-----------------------------------------------------------------------------

class CCmdUnSelectNode : public CCommand
{
public:
	CCmdUnSelectNode(GModel* model, int* node, int n);
	CCmdUnSelectNode(GModel* model, const std::vector<int>& node);

	void Execute();
	void UnExecute();

protected:
	GModel*			m_model;	// pointer to model
	std::vector<int>		m_node;		// list of edges to select
	std::vector<bool>	m_bold;		// old selection state of surfaces
	bool			m_badd;		// add to current selection
};

//-----------------------------------------------------------------------------
class CCmdSelectDiscrete : public CCommand
{
public:
	CCmdSelectDiscrete(GModel* ps, int* pobj, int n, bool badd);
	CCmdSelectDiscrete(GModel* ps, const std::vector<int>& obj, bool badd);
	CCmdSelectDiscrete(GModel* ps, const std::vector<GDiscreteObject*>& obj, bool badd);

	void Execute();
	void UnExecute();

protected:
	GModel*						m_pgm;		// pointer to model
	std::vector<GDiscreteObject*>	m_obj;		// list of discrete objects to select
	std::vector<bool>	m_bold;		// old selection state of surfaces
	bool			m_badd;		// add to current selection
};

//-----------------------------------------------------------------------------
class CCmdSelectDiscreteElements : public CCommand
{
public:
	CCmdSelectDiscreteElements(GDiscreteElementSet* set, const std::vector<int>& elemList, bool badd);

	void Execute();
	void UnExecute();

protected:
	GDiscreteElementSet*	m_ps;	// pointer to discrete element set
	std::vector<int>		m_elemList;		// list of discrete objects to select
	std::vector<bool>	m_bold;			// old selection state of surfaces
	bool			m_badd;			// add to current selection
};

//-----------------------------------------------------------------------------
class CCmdUnSelectDiscreteElements : public CCommand
{
public:
	CCmdUnSelectDiscreteElements(GDiscreteElementSet* set, const std::vector<int>& elemList);

	void Execute();
	void UnExecute();

protected:
	GDiscreteElementSet* m_ps;	// pointer to discrete element set
	std::vector<int>		m_elemList;		// list of discrete objects to select
	std::vector<bool>	m_bold;			// old selection state of surfaces
};

//-----------------------------------------------------------------------------

class CCmdUnSelectDiscrete : public CCommand
{
public:
	CCmdUnSelectDiscrete(GModel* ps, int* pobj, int n);
	CCmdUnSelectDiscrete(GModel* ps, const std::vector<int>& obj);
	CCmdUnSelectDiscrete(GModel* ps, const std::vector<GDiscreteObject*>& obj);

	void Execute();
	void UnExecute();

protected:
	GModel*			m_model;	// pointer to model
	std::vector<GDiscreteObject*>		m_obj;		// list of discrete objects to select
	std::vector<bool>	m_bold;		// old selection state of surfaces
};


//-----------------------------------------------------------------------------

class CCmdAddAndSelectObject : public CCmdGroup
{
public:
	CCmdAddAndSelectObject(GModel* model, GObject* po) : CCmdGroup("Add object")
	{
		AddCommand(new CCmdAddObject(model, po));
		AddCommand(new CCmdSelectObject(model, po, false));
	}
};

//-----------------------------------------------------------------------------

class CCmdInvertSelection : public CCommand
{
public:
	CCmdInvertSelection(CGLDocument* doc);

	void Execute();
	void UnExecute();

protected:
	CGLDocument*	m_doc;
	int	m_item;
};

//-----------------------------------------------------------------------------

class CCmdSelectElements : public CCommand
{
public:
	CCmdSelectElements(FSCoreMesh* pm, int* pe, int N, bool badd);
	CCmdSelectElements(FSCoreMesh* pm, const std::vector<int>& el, bool badd);
	~CCmdSelectElements() { delete[] m_ptag; delete[] m_pel; }

	void Execute();
	void UnExecute();

protected:
	FSCoreMesh*	m_pm;
	bool*	m_ptag;	// old selecion state of elements
	int*	m_pel;	// array of element indics we need to select
	bool	m_badd; // add to selection or not
	int		m_N;	// nr of elements to select
};

//-----------------------------------------------------------------------------

class CCmdUnselectElements : public CCommand
{
public:
	CCmdUnselectElements(FSMesh* mesh, int* pe, int N);
	CCmdUnselectElements(FSMesh* mesh, const std::vector<int>& elem);
	~CCmdUnselectElements() { delete[] m_ptag; delete[] m_pel; }

	void Execute();
	void UnExecute();

protected:
	FSMesh* m_mesh;
	bool*	m_ptag;	// old selecion state of elements
	int*	m_pel;	// array of element indics we need to select
	bool	m_badd; // add to selection or not
	int		m_N;	// nr of elements to select
};

//-----------------------------------------------------------------------------

class CCmdSelectFaces : public CCommand
{
public:
	CCmdSelectFaces(FSMeshBase* pm, int* pf, int N, bool badd);
	CCmdSelectFaces(FSMeshBase* pm, const std::vector<int>& fl, bool badd);
	~CCmdSelectFaces() { delete[] m_ptag; delete[] m_pface; }

	void Execute();
	void UnExecute();

protected:
	FSMeshBase*	m_pm;
	bool*	m_ptag;	// old selecion state of faces
	int*	m_pface;// array of face indics we need to select
	bool	m_badd; // add to selection or not
	int		m_N;	// nr of faces to select
};

//-----------------------------------------------------------------------------

class CCmdUnselectFaces : public CCommand
{
public:
	CCmdUnselectFaces(FSMeshBase* pm, int* pf, int N);
	CCmdUnselectFaces(FSMeshBase* pm, const std::vector<int>& face);
	~CCmdUnselectFaces() { delete[] m_ptag; delete[] m_pface; }

	void Execute();
	void UnExecute();

protected:
	FSMeshBase* m_pm;
	bool*	m_ptag;	// old selecion state of faces
	int*	m_pface;	// array of face indics we need to select
	bool	m_badd; // add to selection or not
	int		m_N;	// nr of faces to select
};

//-----------------------------------------------------------------------------

class CCmdSelectFEEdges : public CCommand
{
public:
	CCmdSelectFEEdges(FSLineMesh* pm, int* pe, int N, bool badd);
	CCmdSelectFEEdges(FSLineMesh* pm, const std::vector<int>& el, bool badd);
	~CCmdSelectFEEdges() { delete[] m_ptag; delete[] m_pedge; }

	void Execute();
	void UnExecute();

protected:
	FSLineMesh*	m_pm;
	bool*	m_ptag;	// old selecion state of edges
	int*	m_pedge;// array of edge indices we need to select
	bool	m_badd; // add to selection or not
	int		m_N;	// nr of edges to select
};

//-----------------------------------------------------------------------------

class CCmdUnselectFEEdges : public CCommand
{
public:
	CCmdUnselectFEEdges(FSLineMesh* pm, int* pe, int N);
	CCmdUnselectFEEdges(FSLineMesh* pm, const std::vector<int>& edge);
	~CCmdUnselectFEEdges() { delete[] m_ptag; delete[] m_pedge; }

	void Execute();
	void UnExecute();

protected:
	FSLineMesh*	m_pm;
	bool*	m_ptag;		// old selecion state of edges
	int*	m_pedge;	// array of edge indices we need to select
	bool	m_badd;		// add to selection or not
	int		m_N;		// nr of faces to select
};

//-----------------------------------------------------------------------------

class CCmdSelectFENodes : public CCommand
{
public:
	CCmdSelectFENodes(FSLineMesh* pm, int* pn, int N, bool badd);
	CCmdSelectFENodes(FSLineMesh* pm, const std::vector<int>& nl, bool badd);
	~CCmdSelectFENodes() { delete[] m_ptag; delete[] m_pn; }

	void Execute();
	void UnExecute();

protected:
	FSLineMesh*	m_pm;
	bool*	m_ptag;	// old selecion state of nodes
	int*	m_pn;	// array of node indices we need to select
	bool	m_badd; // add to selection or not
	int		m_N;	// nr of nodes to select
};

//-----------------------------------------------------------------------------

class CCmdUnselectNodes : public CCommand
{
public:
	CCmdUnselectNodes(FSLineMesh* pm, int* pn, int N);
	CCmdUnselectNodes(FSLineMesh* pm, const std::vector<int>& node);
	~CCmdUnselectNodes() { delete[] m_ptag; delete[] m_pn; }

	void Execute();
	void UnExecute();

protected:
	FSLineMesh* m_mesh;
	bool*	m_ptag;	// old selecion state of nodes
	int*	m_pn;	// array of nodes indices we need to select
	bool	m_badd; // add to selection or not
	int		m_N;	// nr of nodes to select
};

//-----------------------------------------------------------------------------
class CCmdDeleteFESelection : public CCommand
{
public:
	CCmdDeleteFESelection(GMeshObject* po, int nitem);
	~CCmdDeleteFESelection() { delete m_pnew; }

	void Execute();
	void UnExecute();

protected:
	GMeshObject*	m_pobj;
	FSMesh*			m_pold;
	FSMesh*			m_pnew;
	int		m_nitem;
};

//-----------------------------------------------------------------------------

class CCmdDeleteFESurfaceSelection : public CCommand
{
public:
	CCmdDeleteFESurfaceSelection(GSurfaceMeshObject* po, int nitem);
	~CCmdDeleteFESurfaceSelection() { delete m_pnew; }

	void Execute();
	void UnExecute();

protected:
	GSurfaceMeshObject*	m_pobj;
	FSSurfaceMesh*		m_pold;
	FSSurfaceMesh*		m_pnew;
	int				m_item;
};


//-----------------------------------------------------------------------------

class CCmdHideObject : public CCommand
{
public:
	CCmdHideObject(GObject* po, bool bhide);
	CCmdHideObject(std::vector<GObject*> po, bool bhide);

	void Execute();
	void UnExecute();

protected:
	std::vector<GObject*>	m_pobj;
	bool				m_bhide;
};

//-----------------------------------------------------------------------------
class CCmdHideParts : public CCommand
{
public:
	CCmdHideParts(GModel* model, GPart* part);
	CCmdHideParts(GModel* model, std::list<GPart*> partList);

	void Execute();
	void UnExecute();

protected:
	GModel*		m_model;
	std::list<GPart*>	m_partList;
};

//-----------------------------------------------------------------------------
class CCmdShowParts : public CCommand
{
public:
	CCmdShowParts(GModel* model, std::list<GPart*> partList);

	void Execute();
	void UnExecute();

protected:
	GModel*		m_model;
	std::list<GPart*>	m_partList;
};

//-----------------------------------------------------------------------------
class CCmdHideElements : public CCommand
{
public:
	CCmdHideElements(GObject* po, const std::vector<int>& elemList);
	CCmdHideElements(FSMesh* mesh, const std::vector<int>& elemList);

	void Execute();
	void UnExecute();

protected:
	GObject*		m_po;
	FSMesh*			m_mesh;
	std::vector<int>		m_elemList;
};

//-----------------------------------------------------------------------------
class CCmdHideFaces : public CCommand
{
public:
	CCmdHideFaces(FSSurfaceMesh* mesh, const std::vector<int>& faceList);

	void Execute();
	void UnExecute();

protected:
	FSSurfaceMesh*	m_mesh;
	std::vector<int>		m_faceList;
};

//-----------------------------------------------------------------------------

class CCmdHideSelection : public CCommand
{
public:
	CCmdHideSelection(CModelDocument* doc);

	void Execute();
	void UnExecute();

protected:
	CModelDocument*	m_doc;
	std::vector<int>	m_item;
	int		m_nitem;
	int		m_nselect;
};

//-----------------------------------------------------------------------------

class CCmdHideUnselected : public CCommand
{
public:
	CCmdHideUnselected(CModelDocument* doc);

	void Execute();
	void UnExecute();

protected:
	CModelDocument*	m_doc;
	int			m_nitem;
	int			m_nselect;
	std::vector<int>	m_item;
};

//-----------------------------------------------------------------------------

class CCmdUnhideAll : public CCommand
{
public:
	CCmdUnhideAll(CModelDocument* doc);

	void Execute();
	void UnExecute();

protected:
	CModelDocument*	m_doc;
	std::vector<int>	m_item;
	int			m_nitem;
	int			m_nselect;
	bool		m_bunhide;
};

//-----------------------------------------------------------------------------

class CCmdApplyFEModifier : public CCommand
{
public:
	CCmdApplyFEModifier(FEModifier* pmod, GObject* po, FSGroup* selection = 0);
	~CCmdApplyFEModifier() { delete m_pnew; delete m_pmod; }

	void Execute();
	void UnExecute();

protected:
	GObject*		m_pobj;
	FSMesh*			m_pold;	// old, unmodified mesh
	FSMesh*			m_pnew;	// new, modified mesh
	FEModifier*		m_pmod;
	FSGroup*		m_psel;
};

//-----------------------------------------------------------------------------

class CCmdApplySurfaceModifier : public CCommand
{
public:
	CCmdApplySurfaceModifier(FESurfaceModifier* pmod, GObject* po, FSGroup* selection);
	~CCmdApplySurfaceModifier();

	void Execute();
	void UnExecute();

protected:
	GObject*			m_pobj;
	FSSurfaceMesh*		m_pold;	// old, unmodified mesh
	FSSurfaceMesh*		m_pnew;	// new, modified mesh
	FESurfaceModifier*	m_pmod;
	FSGroup*			m_psel;
};

//-----------------------------------------------------------------------------

class CCmdChangeFEMesh : public CCommand
{
public:
	CCmdChangeFEMesh(GObject* po, FSMesh* pm);
	~CCmdChangeFEMesh() { delete m_pnew; }

	void Execute();
	void UnExecute();

protected:
	GObject*	m_po;
	FSMesh*		m_pnew;
};

class CCmdChangeFENodes : public CCommand
{
public:
	CCmdChangeFENodes(GObject* po, const std::vector<vec3d>& newPos);

	void Execute();
	void UnExecute();

protected:
	GObject* m_po;
	std::vector<vec3d> m_newPos;
};

//-----------------------------------------------------------------------------
class CCmdChangeFESurfaceMesh : public CCommand
{
public:
	CCmdChangeFESurfaceMesh(GSurfaceMeshObject* po, FSSurfaceMesh* pm, bool up = false);
	~CCmdChangeFESurfaceMesh();

	void Execute();
	void UnExecute();

protected:
	bool				m_update;
	GSurfaceMeshObject*	m_po;
	FSSurfaceMesh*		m_pnew;
};

//-----------------------------------------------------------------------------

class CCmdChangeView : public CCommand
{
public:
	CCmdChangeView(CGView* pview, GLCamera cam);
	~CCmdChangeView();

	void Execute();
	void UnExecute();

protected:
	CGView*		m_pview;
	GLCamera	m_cam;
};

//-----------------------------------------------------------------------------

class CCmdInvertElements : public CCommand
{
public:
	CCmdInvertElements(GMeshObject* po);

	void Execute();
	void UnExecute();

protected:
	GMeshObject*	m_po;
};

//-----------------------------------------------------------------------------
class CCmdChangeObjectParams : public CCommand
{
public:
	CCmdChangeObjectParams(GObject* pm);

	void Execute();
	void UnExecute();

protected:
	GObject*		m_po;
	ParamBlock		m_Param;
};

//-----------------------------------------------------------------------------
class CCmdChangeMesherParams : public CCommand
{
public:
	CCmdChangeMesherParams(GObject* pm);

	void Execute();
	void UnExecute();

protected:
	GObject*		m_po;
	ParamBlock		m_Param;
};

//-----------------------------------------------------------------------------
// swap two objects in the model
class CCmdSwapObjects : public CCommand
{
public:
	CCmdSwapObjects(GModel* model, GObject* pold, GObject* pnew);
	~CCmdSwapObjects();

	void Execute();
	void UnExecute();

protected:
	GModel*		m_model;
	GObject*	m_pold;	// the original object
	GObject*	m_pnew;	// the new mesh object
	ObjectMeshList*	m_oml;	// old object list
};

//-----------------------------------------------------------------------------
// Convert a primitive to a multi-block mesh
class CCmdConvertToMultiBlock : public CCommand
{
public:
	CCmdConvertToMultiBlock(GModel* model, GObject* po);
	~CCmdConvertToMultiBlock();

	void Execute();
	void UnExecute();

protected:
	GModel*		m_model;
	GObject*	m_pold;	// the original object
	GObject*	m_pnew;	// the new object
};

//----------------------------------------------------------------

class CCmdAddModifier : public CCommand
{
public:
	CCmdAddModifier(GModifiedObject* po, GModifier* pm);
	~CCmdAddModifier();

	void Execute();
	void UnExecute();

protected:
	GModifiedObject*	m_po;
	GModifier*			m_pm;
};

//----------------------------------------------------------------
class CCmdAddStep : public CCommand
{
public:
	CCmdAddStep(FSModel* fem, FSStep* ps, int insertAfter = -1);
	~CCmdAddStep();

	void Execute();
	void UnExecute();

protected:
	FSModel*	m_fem;
	FSStep*		m_pstep;
	int			m_pos;
};

//----------------------------------------------------------------
class CCmdSwapSteps : public CCommand
{
public:
	CCmdSwapSteps(FSModel* fem, FSStep* step0, FSStep* step1);

	void Execute();
	void UnExecute();

private:
	FSModel*	m_fem;
	FSStep*		m_step0;
	FSStep*		m_step1;
};

//----------------------------------------------------------------
class CCmdAddMaterial : public CCommand
{
public:
	CCmdAddMaterial(FSModel* fem, GMaterial* pm);
	~CCmdAddMaterial();

	void Execute();
	void UnExecute();

protected:
	FSModel*	m_fem;
	GMaterial*	m_pm;
};

class CCmdAssignMaterial : public CCommand
{
public:
	CCmdAssignMaterial(FSModel* fem, GPart* pg, GMaterial* mat);
	CCmdAssignMaterial(FSModel* fem, std::vector<GPart*>& partList, GMaterial* mat);
	void Execute();
	void UnExecute();

private:
	FSModel* m_fem;
	GMaterial* m_newMat;
	std::vector<GPart*> m_partList;
	std::vector<GMaterial*> m_oldMats;
};

class CCmdSetItemList : public CCommand
{
public:
	CCmdSetItemList(IHasItemLists* pbc, FSItemListBuilder* pl, int n);
	~CCmdSetItemList();

	void Execute();
	void UnExecute();

protected:
	IHasItemLists*		m_pbc;
	FSItemListBuilder*	m_pl;
	int					m_index;
};

//-----------------------------------------------------------------------------
class CCmdAddToItemListBuilder : public CCommand
{
public:
	CCmdAddToItemListBuilder(FSItemListBuilder* pold, std::vector<int>& lnew);

	void Execute();
	void UnExecute();

protected:
	FSItemListBuilder* m_pold;
	std::vector<int>	m_lnew;
	std::vector<int>	m_tmp;
};

//-----------------------------------------------------------------------------
class CCmdRemoveFromItemListBuilder : public CCommand
{
public:
	CCmdRemoveFromItemListBuilder(FSItemListBuilder* pold, std::vector<int>& lnew);

	void Execute();
	void UnExecute();

protected:
	FSItemListBuilder* m_pold;
	std::vector<int>	m_lnew;
	std::vector<int>	m_tmp;
};

//-----------------------------------------------------------------------------
class CCmdRemoveItemListBuilder : public CCommand
{
public:
	CCmdRemoveItemListBuilder(IHasItemLists* pmc, int n = 0);
	~CCmdRemoveItemListBuilder();

	void Execute();
	void UnExecute();

private:
	FSItemListBuilder*	m_pitem;
	IHasItemLists*		m_pmc;
	int	m_index;
};

//-----------------------------------------------------------------------------
class CCmdDeleteGObject : public CCommand
{
public:
	CCmdDeleteGObject(GModel* gm, GObject* po);
	~CCmdDeleteGObject();

	void Execute();
	void UnExecute();

protected:
	GModel*			m_gm;
	GObject*		m_po;
	ObjectMeshList*	m_poml;
};

//-----------------------------------------------------------------------------
class CCmdDeleteFSModelComponent: public CCmdGroup
{
public:
	CCmdDeleteFSModelComponent(FSModelComponent* po);
	~CCmdDeleteFSModelComponent();

	void Execute();
	void UnExecute();

protected:
	FSModelComponent* m_obj;
	std::vector<FSItemListBuilder*> m_sel;
};

//-----------------------------------------------------------------------------
class CCmdDeleteFSObject : public CCommand
{
public:
	CCmdDeleteFSObject(FSObject* po);
	~CCmdDeleteFSObject();

	void Execute();
	void UnExecute();

protected:
	FSObject*	m_obj;
	FSObject*	m_parent;
	bool		m_delObject;
	size_t		m_insertPos;
};

//-----------------------------------------------------------------------------
class CCmdSetActiveMeshLayer : public CCommand
{
public:
	CCmdSetActiveMeshLayer(GModel* mdl, int activeLayer);

	void Execute();
	void UnExecute();

protected:
	GModel*		m_gm;
	int			m_activeLayer;
};

//-----------------------------------------------------------------------------
class CCmdAddMeshLayer : public CCommand
{
public:
	CCmdAddMeshLayer(GModel* mdl, const std::string& layerName);

	void Execute();
	void UnExecute();

protected:
	GModel*		m_gm;
	std::string	m_layerName;
	int			m_layerIndex;
};

//-----------------------------------------------------------------------------
class CCmdDeleteMeshLayer : public CCommand
{
public:
	CCmdDeleteMeshLayer(GModel* mdl, int layerIndex);
	~CCmdDeleteMeshLayer();

	void Execute();
	void UnExecute();

protected:
	GModel*	m_gm;
	int		m_layerIndex;
	MeshLayer*	m_layer;
};

//-----------------------------------------------------------------------------
class CCmdRemoveMeshData : public CCommand
{
public:
	CCmdRemoveMeshData(FSMeshData* meshData);
	~CCmdRemoveMeshData();

	void Execute() override;
	void UnExecute() override;

private:
	FSMeshData*	m_data;
	int		m_index;
};

class CCmdToggleActiveParts : public CCommand
{
public:
	CCmdToggleActiveParts(const std::vector<GPart*>& partList);

	void Execute() override;
	void UnExecute() override;

private:
	std::vector<GPart*> m_partList;
};

//-----------------------------------------------------------------------------
class CCmdAddImageAnalysis : public CCommand
{
public:
	CCmdAddImageAnalysis(CImageAnalysis* analysis);
	~CCmdAddImageAnalysis();

	void Execute() override;
	void UnExecute() override;

private:
    bool m_del;
	CImageAnalysis*	m_analysis;
};

//-----------------------------------------------------------------------------
class CCmdDeleteImageAnalysis : public CCommand
{
public:
	CCmdDeleteImageAnalysis(CImageAnalysis* analysis);
	~CCmdDeleteImageAnalysis();

	void Execute() override;
	void UnExecute() override;

private:
    bool m_del;
	CImageAnalysis*	m_analysis;
};

// this command swaps the material properties of a GMaterial
class CCmdSwapMaterialProps : public CCommand
{
public:
	CCmdSwapMaterialProps(GMaterial* gmat, FSMaterial* props);

	~CCmdSwapMaterialProps();

	void Execute() override;

	void UnExecute() override;

private:
	GMaterial* m_gmat;
	FSMaterial* m_props;
};
