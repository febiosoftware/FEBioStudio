#pragma once
#include <MeshTools/FEModifier.h>
#include <FEMLib/FEInterface.h>
#include <GeomLib/GObject.h>
#include <FEMLib/FEBoundaryCondition.h>
#include <FEMLib/FEConnector.h>
#include <FSCore/ParamBlock.h>
#include <MeshTools/GGroup.h>
#include <MeshTools/GDiscreteObject.h>
#include <FEMLib/FERigidConstraint.h>
#include <GeomLib/GMeshObject.h>
#include <MeshTools/GModifiedObject.h>
#include <MeshTools/FESurfaceModifier.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <GLLib/GLCamera.h>
#include "ViewSettings.h"

class CDocument;
class CCommandManager;
class CGLView;
class FEAnalysisStep;

//----------------------------------------------------------------
class CCommand;

//----------------------------------------------------------------
// exception thrown a command is executed
class CCmdFailed
{
public:
	CCmdFailed(CCommand* pcmd, const std::string& err) : m_pcmd(pcmd), m_err(err) {}
	CCommand* GetCommand() { return m_pcmd; }
	std::string GetErrorString() { return m_err; }

private:
	CCommand* m_pcmd;
	std::string	m_err;
};

//----------------------------------------------------------------
// CCommand
// Base class for other commands
//
class CCommand  
{
public:
	CCommand(const string& name);
	virtual ~CCommand() {}

	virtual void Execute  () = 0;
	virtual void UnExecute() = 0;

	const char* GetName() const { return m_name.c_str(); }
	void SetName(const string& name) { m_name = name; }

	VIEW_STATE GetViewState() { return m_state; }

protected:
	static CDocument* m_pDoc;

	// doc/view state variables
	VIEW_STATE	m_state;

	string m_name;	// command name

	friend class CCommandManager;
};

typedef vector<CCommand*> CCmdPtrArray;

//----------------------------------------------------------------
// CCmdGroup
// Command that groups other commands
//
class CCmdGroup : public CCommand
{
public:
	CCmdGroup(const char* sz) : CCommand((sz?sz:"Group")) {}
	virtual ~CCmdGroup() { for (int i=0; i<(int) m_Cmd.size(); ++i) delete m_Cmd[i]; }

	void AddCommand(CCommand* pcmd) { m_Cmd.push_back(pcmd); }

	virtual void Execute()
	{
		int N = (int) m_Cmd.size();
		for (int i=0; i<N; i++) m_Cmd[i]->Execute();
	}

	virtual void UnExecute()
	{
		int N = (int)m_Cmd.size();
		for (int i=N-1; i>=0; i--) m_Cmd[i]->UnExecute();
	}

	int GetCount() { return (int)m_Cmd.size(); }

protected:
	CCmdPtrArray	m_Cmd;	// array of pointer to commands
};

//-----------------------------------------------------------------------------

class CCmdAddObject : public CCommand
{
public:
	CCmdAddObject(GObject* po) : CCommand("Add object") { m_pobj = po; m_bdel = true; }
	~CCmdAddObject() { if (m_bdel) delete m_pobj; }

	void Execute();
	void UnExecute();

protected:
	GObject*	m_pobj;
	bool		m_bdel;
};

//-----------------------------------------------------------------------------

class CCmdAddDiscreteObject : public CCommand
{
public:
	CCmdAddDiscreteObject(GDiscreteObject* po) : CCommand("Add discrete object") { m_pobj = po; m_bdel = true; }
	~CCmdAddDiscreteObject() { if (m_bdel) delete m_pobj; }

	void Execute();
	void UnExecute();

protected:
	GDiscreteObject*	m_pobj;
	bool				m_bdel;
};

//-----------------------------------------------------------------------------

class CCmdAddInterface : public CCommand
{
public:
	CCmdAddInterface(FEStep* ps, FEInterface* pi) : CCommand("Add interface") { m_ps = ps; m_pint = pi; m_bdel = true; }
	~CCmdAddInterface() { if (m_bdel) delete m_pint; }

	void Execute();
	void UnExecute();

protected:
	FEStep*			m_ps;
	FEInterface*	m_pint;
	bool			m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddRigidConnector : public CCommand
{
public:
	CCmdAddRigidConnector(FEStep* ps, FERigidConnector* pi) : CCommand("Add rigid connector") { m_ps = ps; m_pint = pi; m_bdel = true; }
    ~CCmdAddRigidConnector() { if (m_bdel) delete m_pint; }
    
    void Execute();
    void UnExecute();
    
protected:
    FEStep*			m_ps;
	FERigidConnector*	m_pint;
    bool			m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddPart : public CCommand
{
public:
	CCmdAddPart(GObject* po, FEPart* pg) : CCommand("Add Part") { m_po = po; m_pg = pg; m_bdel = true; }
	~CCmdAddPart() { if (m_bdel) delete m_pg; }

	void Execute();
	void UnExecute();

protected:
	GObject*		m_po;
	FEPart*		m_pg;
	bool		m_bdel;
};

//-----------------------------------------------------------------------------

class CCmdAddSurface: public CCommand
{
public:
	CCmdAddSurface(GObject* po, FESurface* pg) : CCommand("Add Surface") { m_po = po; m_pg = pg; m_bdel = true; }
	~CCmdAddSurface() { if (m_bdel) delete m_pg; }

	void Execute();
	void UnExecute();

protected:
	GObject*	m_po;
	FESurface*	m_pg;
	bool		m_bdel;
};

//-----------------------------------------------------------------------------

class CCmdAddFEEdgeSet : public CCommand
{
public:
	CCmdAddFEEdgeSet(GObject* po, FEEdgeSet* pg) : CCommand("Add EdgeSet") { m_po = po; m_pg = pg; m_bdel = true; }
	~CCmdAddFEEdgeSet() { if (m_bdel) delete m_pg; }

	void Execute();
	void UnExecute();

protected:
	GObject*	m_po;
	FEEdgeSet*	m_pg;
	bool		m_bdel;
};

//-----------------------------------------------------------------------------

class CCmdAddNodeSet : public CCommand
{
public:
	CCmdAddNodeSet(GObject* po, FENodeSet* pg) : CCommand("Add Nodeset") { m_po = po; m_pg = pg; m_bdel = true; }
	~CCmdAddNodeSet() { if (m_bdel) delete m_pg; }

	void Execute();
	void UnExecute();

protected:
	GObject*	m_po;
	FENodeSet*	m_pg;
	bool		m_bdel;
};

//-----------------------------------------------------------------------------

class CCmdAddGPartGroup : public CCommand
{
public:
	CCmdAddGPartGroup(GPartList* pg) : CCommand("Add Part") { m_pg = pg; m_bdel = true; }
	~CCmdAddGPartGroup() { if (m_bdel) delete m_pg; }

	void Execute();
	void UnExecute();

protected:
	GPartList*		m_pg;
	bool			m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddGFaceGroup : public CCommand
{
public:
	CCmdAddGFaceGroup(GFaceList* pg) : CCommand("Add Surface") { m_pg = pg; m_bdel = true; }
	~CCmdAddGFaceGroup() { if (m_bdel) delete m_pg; }

	void Execute();
	void UnExecute();

protected:
	GFaceList*		m_pg;
	bool			m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddGEdgeGroup : public CCommand
{
public:
	CCmdAddGEdgeGroup(GEdgeList* pg) : CCommand("Add Edge") { m_pg = pg; m_bdel = true; }
	~CCmdAddGEdgeGroup() { if (m_bdel) delete m_pg; }

	void Execute();
	void UnExecute();

protected:
	GEdgeList*		m_pg;
	bool			m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddGNodeGroup : public CCommand
{
public:
	CCmdAddGNodeGroup(GNodeList* pg) : CCommand("Add Nodeset") { m_pg = pg; m_bdel = true; }
	~CCmdAddGNodeGroup() { if (m_bdel) delete m_pg; }

	void Execute();
	void UnExecute();

protected:
	GNodeList*		m_pg;
	bool			m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddBC : public CCommand
{
public:
	CCmdAddBC(FEStep* ps, FEBoundaryCondition* pbc) : CCommand("Add Boundary Condition") { m_ps = ps; m_pbc = pbc; m_bdel = true; }
	~CCmdAddBC() { if (m_bdel) delete m_pbc; }

	void Execute();
	void UnExecute();

protected:
	FEStep*					m_ps;
	FEBoundaryCondition*	m_pbc;
	bool					m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddIC : public CCommand
{
public:
	CCmdAddIC(FEStep* ps, FEInitialCondition* pic) : CCommand("Add Initial Condition") { m_ps = ps; m_pic = pic; m_bdel = true; }
	~CCmdAddIC() { if (m_bdel) delete m_pic; }

	void Execute();
	void UnExecute();

protected:
	FEStep*					m_ps;
	FEInitialCondition*		m_pic;
	bool					m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddLoad : public CCommand
{
public:
	CCmdAddLoad(FEStep* ps, FEBoundaryCondition* pfc) : CCommand("Add Load") { m_ps = ps; m_pfc = pfc; m_bdel = true; }
	~CCmdAddLoad() { if (m_bdel) delete m_pfc; }

	void Execute();
	void UnExecute();

protected:
	FEStep*					m_ps;
	FEBoundaryCondition*	m_pfc;
	bool					m_bdel;
};

//-----------------------------------------------------------------------------
class CCmdAddRC : public CCommand
{
public:
	CCmdAddRC(FEStep* ps, FERigidConstraint* prc) : CCommand("Add Rigid Constraint") { m_ps = ps; m_prc = prc; m_bdel = true; }
	~CCmdAddRC() { if (m_bdel) delete m_prc; }

	void Execute();
	void UnExecute();

protected:
	FEStep*				m_ps;
	FERigidConstraint*	m_prc;
	bool				m_bdel;
};

//----------------------------------------------------------------
class CCmdDeleteDiscreteObject : public CCommand
{
public:
	CCmdDeleteDiscreteObject(GDiscreteObject* po) : CCommand("Remove discrete object") { m_pobj = po; m_bdel = false; }
	~CCmdDeleteDiscreteObject() { if (m_bdel) delete m_pobj; }

	void Execute();
	void UnExecute();

protected:
	GDiscreteObject*	m_pobj;
	bool	m_bdel;
	int		m_npos;
};


//-----------------------------------------------------------------------------
class CCmdTranslateSelection : public CCommand
{
public:
	CCmdTranslateSelection(vec3d dr);

	void Execute();
	void UnExecute();

protected:
	vec3d	m_dr;
	int		m_item;	// item mode
};

//-----------------------------------------------------------------------------

class CCmdRotateSelection : public CCommand
{
public:
	CCmdRotateSelection(quatd q, vec3d rc);

	void Execute();
	void UnExecute();

protected:
	quatd	m_q;
	vec3d	m_rc;
	int		m_item;	// item mode
};

//-----------------------------------------------------------------------------

class CCmdScaleSelection : public CCommand
{
public:
	CCmdScaleSelection(double s, vec3d dr, vec3d rc);

	void Execute();
	void UnExecute();

protected:
	double	m_s;
	vec3d	m_dr;
	vec3d	m_rc;
	int		m_item;	// item mode
};

//-----------------------------------------------------------------------------
class CCmdToggleObjectVisibility : public CCommand
{
public:
	CCmdToggleObjectVisibility();
	
	void Execute();
	void UnExecute();
};

//-----------------------------------------------------------------------------
class CCmdTogglePartVisibility : public CCommand
{
public:
	CCmdTogglePartVisibility();

	void Execute();
	void UnExecute();
};

//-----------------------------------------------------------------------------
class CCmdToggleDiscreteVisibility : public CCommand
{
public:
	CCmdToggleDiscreteVisibility();

	void Execute();
	void UnExecute();
};

//-----------------------------------------------------------------------------
class CCmdToggleElementVisibility : public CCommand
{
public:
	CCmdToggleElementVisibility();

	void Execute();
	void UnExecute();
};

//-----------------------------------------------------------------------------
class CCmdToggleFEFaceVisibility : public CCommand
{
public:
	CCmdToggleFEFaceVisibility();

	void Execute();
	void UnExecute();
};

//-----------------------------------------------------------------------------

class CCmdSelectObject : public CCommand
{
public: 
	CCmdSelectObject(GObject* po, bool badd);
	CCmdSelectObject(GObject** ppo, int n, bool badd);
	CCmdSelectObject(const vector<GObject*>& po, bool badd);
	~CCmdSelectObject() { delete [] m_ptag; delete [] m_ppo; }

	void Execute();
	void UnExecute();

protected:
	bool*		m_ptag;	// old selection state of meshes
	GObject**	m_ppo;	// pointers to objects we need to select
	bool		m_badd; // add meshes to selection or not
	int			m_N;	// Nr of meshes to select
};

//-----------------------------------------------------------------------------

class CCmdUnselectObject : public CCommand
{
public: 
	CCmdUnselectObject(GObject* po);
	CCmdUnselectObject(GObject** ppo, int n);
	CCmdUnselectObject(const vector<GObject*>& po);
	~CCmdUnselectObject() { delete [] m_ptag; delete [] m_ppo; }

	void Execute();
	void UnExecute();

protected:
	bool*		m_ptag;	// old selection state of meshes
	GObject**	m_ppo;	// pointers to meshes we need to unselect
	int			m_N;	// Nr of meshes to select
};


//-----------------------------------------------------------------------------

class CCmdSelectPart : public CCommand
{
public:
	CCmdSelectPart(FEModel* ps, int* npart, int n, bool badd);
	CCmdSelectPart(FEModel* ps, const vector<int>& part, bool badd);

	void Execute();
	void UnExecute();

protected:
	FEModel*		m_ps;		// pointer to model
	vector<int>		m_npart;	// list of parts to select
	vector<bool>	m_bold;		// old selection state of parts
	bool			m_badd;		// add to current selection
};

//-----------------------------------------------------------------------------

class CCmdUnSelectPart : public CCommand
{
public:
	CCmdUnSelectPart(FEModel* ps, int* npart, int n);
	CCmdUnSelectPart(FEModel* ps, const vector<int>& part);

	void Execute();
	void UnExecute();

protected:
	FEModel*		m_ps;		// pointer to model
	vector<int>		m_npart;	// list of parts to select
	vector<bool>	m_bold;		// old selection state of parts
};

//-----------------------------------------------------------------------------

class CCmdSelectSurface : public CCommand
{
public:
	CCmdSelectSurface(FEModel* ps, int* nsurf, int n, bool badd);
	CCmdSelectSurface(FEModel* ps, const vector<int>& surf, bool badd);

	void Execute();
	void UnExecute();

protected:
	FEModel*		m_ps;		// pointer to model
	vector<int>		m_nsurf;	// list of surfaces to select
	vector<bool>	m_bold;		// old selection state of surfaces
	bool			m_badd;		// add to current selection
};

//-----------------------------------------------------------------------------

class CCmdUnSelectSurface : public CCommand
{
public:
	CCmdUnSelectSurface(FEModel* ps, int* nsurf, int n);
	CCmdUnSelectSurface(FEModel* ps, const vector<int>& surf);

	void Execute();
	void UnExecute();

protected:
	FEModel*		m_ps;		// pointer to model
	vector<int>		m_nsurf;	// list of surfaces to select
	vector<bool>	m_bold;		// old selection state of surfaces
	bool			m_badd;		// add to current selection
};

//-----------------------------------------------------------------------------

class CCmdSelectEdge : public CCommand
{
public:
	CCmdSelectEdge(FEModel* ps, int* nedge, int n, bool badd);
	CCmdSelectEdge(FEModel* ps, const vector<int>& edge, bool badd);

	void Execute();
	void UnExecute();

protected:
	FEModel*		m_ps;		// pointer to model
	vector<int>		m_nedge;	// list of edges to select
	vector<bool>	m_bold;		// old selection state of surfaces
	bool			m_badd;		// add to current selection
};

//-----------------------------------------------------------------------------

class CCmdUnSelectEdge : public CCommand
{
public:
	CCmdUnSelectEdge(FEModel* ps, int* nedge, int n);
	CCmdUnSelectEdge(FEModel* ps, const vector<int>& edge);

	void Execute();
	void UnExecute();

protected:
	FEModel*		m_ps;		// pointer to model
	vector<int>		m_nedge;	// list of edges to select
	vector<bool>	m_bold;		// old selection state of surfaces
	bool			m_badd;		// add to current selection
};

//-----------------------------------------------------------------------------

class CCmdSelectNode : public CCommand
{
public:
	CCmdSelectNode(FEModel* ps, int* node, int n, bool badd);
	CCmdSelectNode(FEModel* ps, const vector<int>& node, bool badd);

	void Execute();
	void UnExecute();

protected:
	FEModel*		m_ps;		// pointer to model
	vector<int>		m_node;		// list of edges to select
	vector<bool>	m_bold;		// old selection state of surfaces
	bool			m_badd;		// add to current selection
};

//-----------------------------------------------------------------------------

class CCmdUnSelectNode : public CCommand
{
public:
	CCmdUnSelectNode(FEModel* ps, int* node, int n);
	CCmdUnSelectNode(FEModel* ps, const vector<int>& node);

	void Execute();
	void UnExecute();

protected:
	FEModel*		m_ps;		// pointer to model
	vector<int>		m_node;		// list of edges to select
	vector<bool>	m_bold;		// old selection state of surfaces
	bool			m_badd;		// add to current selection
};

//-----------------------------------------------------------------------------

class CCmdSelectDiscrete : public CCommand
{
public:
	CCmdSelectDiscrete(FEModel* ps, int* pobj, int n, bool badd);
	CCmdSelectDiscrete(FEModel* ps, const vector<int>& obj, bool badd);

	void Execute();
	void UnExecute();

protected:
	FEModel*		m_ps;		// pointer to model
	vector<int>		m_obj;		// list of discrete objects to select
	vector<bool>	m_bold;		// old selection state of surfaces
	bool			m_badd;		// add to current selection
};

//-----------------------------------------------------------------------------

class CCmdUnSelectDiscrete : public CCommand
{
public:
	CCmdUnSelectDiscrete(FEModel* ps, int* pobj, int n);
	CCmdUnSelectDiscrete(FEModel* ps, const vector<int>& obj);

	void Execute();
	void UnExecute();

protected:
	FEModel*		m_ps;		// pointer to model
	vector<int>		m_obj;		// list of discrete objects to select
	vector<bool>	m_bold;		// old selection state of surfaces
	bool			m_badd;		// add to current selection
};


//-----------------------------------------------------------------------------

class CCmdAddAndSelectObject : public CCmdGroup
{
public:
	CCmdAddAndSelectObject(GObject* po) : CCmdGroup("Add object")
	{
		AddCommand(new CCmdAddObject(po));
		AddCommand(new CCmdSelectObject(po, false));
	}
};

//-----------------------------------------------------------------------------

class CCmdInvertSelection : public CCommand
{
public:
	CCmdInvertSelection();

	void Execute();
	void UnExecute();

protected:
	int	m_item;
};

//-----------------------------------------------------------------------------

class CCmdSelectElements : public CCommand
{
public:
	CCmdSelectElements(FEMesh* pm, int* pe, int N, bool badd);
	CCmdSelectElements(FEMesh* pm, vector<int>& el, bool badd);
	~CCmdSelectElements() { delete [] m_ptag; delete [] m_pel; }

	void Execute();
	void UnExecute();

protected:
	FEMesh*	m_pm;
	bool*	m_ptag;	// old selecion state of elements
	int*	m_pel;	// array of element indics we need to select
	bool	m_badd; // add to selection or not
	int		m_N;	// nr of elements to select
};

//-----------------------------------------------------------------------------

class CCmdUnselectElements : public CCommand
{
public:
	CCmdUnselectElements(int* pe, int N);
	CCmdUnselectElements(const vector<int>& elem);
	~CCmdUnselectElements() { delete [] m_ptag; delete [] m_pel; }

	void Execute();
	void UnExecute();

protected:
	bool*	m_ptag;	// old selecion state of elements
	int*	m_pel;	// array of element indics we need to select
	bool	m_badd; // add to selection or not
	int		m_N;	// nr of elements to select
};

//-----------------------------------------------------------------------------

class CCmdSelectFaces : public CCommand
{
public:
	CCmdSelectFaces(FEMeshBase* pm, int* pf, int N, bool badd);
	CCmdSelectFaces(FEMeshBase* pm, vector<int>& fl, bool badd);
	~CCmdSelectFaces() { delete [] m_ptag; delete [] m_pface; }

	void Execute();
	void UnExecute();

protected:
	FEMeshBase*	m_pm;
	bool*	m_ptag;	// old selecion state of faces
	int*	m_pface;// array of face indics we need to select
	bool	m_badd; // add to selection or not
	int		m_N;	// nr of faces to select
};

//-----------------------------------------------------------------------------

class CCmdUnselectFaces : public CCommand
{
public:
	CCmdUnselectFaces(FEMeshBase* pm, int* pf, int N);
	CCmdUnselectFaces(FEMeshBase* pm, const vector<int>& face);
	~CCmdUnselectFaces() { delete [] m_ptag; delete [] m_pface; }

	void Execute();
	void UnExecute();

protected:
	FEMeshBase* m_pm;
	bool*	m_ptag;	// old selecion state of faces
	int*	m_pface;	// array of face indics we need to select
	bool	m_badd; // add to selection or not
	int		m_N;	// nr of faces to select
};

//-----------------------------------------------------------------------------

class CCmdSelectFEEdges : public CCommand
{
public:
	CCmdSelectFEEdges(FELineMesh* pm, int* pe, int N, bool badd);
	CCmdSelectFEEdges(FELineMesh* pm, vector<int>& el, bool badd);
	~CCmdSelectFEEdges() { delete [] m_ptag; delete [] m_pedge; }

	void Execute();
	void UnExecute();

protected:
	FELineMesh*	m_pm;
	bool*	m_ptag;	// old selecion state of edges
	int*	m_pedge;// array of edge indices we need to select
	bool	m_badd; // add to selection or not
	int		m_N;	// nr of edges to select
};

//-----------------------------------------------------------------------------

class CCmdUnselectFEEdges : public CCommand
{
public:
	CCmdUnselectFEEdges(FELineMesh* pm, int* pe, int N);
	CCmdUnselectFEEdges(FELineMesh* pm, const vector<int>& edge);
	~CCmdUnselectFEEdges() { delete [] m_ptag; delete [] m_pedge; }

	void Execute();
	void UnExecute();

protected:
	FELineMesh*	m_pm;
	bool*	m_ptag;		// old selecion state of edges
	int*	m_pedge;	// array of edge indices we need to select
	bool	m_badd;		// add to selection or not
	int		m_N;		// nr of faces to select
};

//-----------------------------------------------------------------------------

class CCmdSelectFENodes : public CCommand
{
public:
	CCmdSelectFENodes(FELineMesh* pm, int* pn, int N, bool badd);
	CCmdSelectFENodes(FELineMesh* pm, vector<int>& nl, bool badd);
	~CCmdSelectFENodes() { delete [] m_ptag; delete [] m_pn; }

	void Execute();
	void UnExecute();

protected:
	FELineMesh*	m_pm;
	bool*	m_ptag;	// old selecion state of nodes
	int*	m_pn;	// array of node indices we need to select
	bool	m_badd; // add to selection or not
	int		m_N;	// nr of nodes to select
};

//-----------------------------------------------------------------------------

class CCmdUnselectNodes : public CCommand
{
public:
	CCmdUnselectNodes(int* pn, int N);
	CCmdUnselectNodes(const vector<int>& node);
	~CCmdUnselectNodes() { delete [] m_ptag; delete [] m_pn; }

	void Execute();
	void UnExecute();

protected:
	bool*	m_ptag;	// old selecion state of nodes
	int*	m_pn;	// array of nodes indices we need to select
	bool	m_badd; // add to selection or not
	int		m_N;	// nr of nodes to select
};

//-----------------------------------------------------------------------------

class CCmdAssignPartMaterial : public CCommand
{
public:
	CCmdAssignPartMaterial(FEModel* po, vector<int> npart, int nmat);

	void Execute();
	void UnExecute();

protected:
	FEModel*	m_ps;
	vector<int>	m_part;

	vector<int>	m_old;	// old materials
	int			m_mat;	// new material
};

//-----------------------------------------------------------------------------

class CCmdAssignObjectMaterial : public CCommand
{
public:
	CCmdAssignObjectMaterial(GObject* po, int mat);

	void Execute();
	void UnExecute();

protected:
	GObject*	m_po;
	int			m_mat;
	vector<int>	m_old;
};

//-----------------------------------------------------------------------------
class CCmdAssignObjectListMaterial : public CCmdGroup
{
public:
	CCmdAssignObjectListMaterial(vector<GObject*> o, int mat);
};

//-----------------------------------------------------------------------------
// Obsolete as of 1.4
/*
class CCmdAssignMaterial : public CCommand
{
public:
	CCmdAssignMaterial(GObject* po, FEMaterial* pmat, int* pel=0, int N=0);
	~CCmdAssignMaterial(){ delete [] m_ppmat; }

	void Execute();
	void UnExecute();

protected:
	FEMaterial*	m_pmat;
	FEMesh*	m_pm;
	GObject* m_po;

	int*	m_pel;
	int		m_N;

	FEMaterial**	m_ppmat;
};
*/

//-----------------------------------------------------------------------------

class CCmdDeleteFESelection : public CCommand
{
public:
	CCmdDeleteFESelection(GMeshObject* po);
	~CCmdDeleteFESelection() { delete m_pnew; }

	void Execute();
	void UnExecute();

protected:
	GMeshObject*	m_pobj;
	FEMesh*			m_pold;
	FEMesh*			m_pnew;
	int				m_item;
};

//-----------------------------------------------------------------------------

class CCmdDeleteFESurfaceSelection : public CCommand
{
public:
	CCmdDeleteFESurfaceSelection(GSurfaceMeshObject* po);
	~CCmdDeleteFESurfaceSelection() { delete m_pnew; }

	void Execute();
	void UnExecute();

protected:
	GSurfaceMeshObject*	m_pobj;
	FESurfaceMesh*		m_pold;
	FESurfaceMesh*		m_pnew;
	int				m_item;
};


//-----------------------------------------------------------------------------

class CCmdHideObject : public CCommand
{
public:
	CCmdHideObject(GObject* po, bool bhide);
	CCmdHideObject(vector<GObject*> po, bool bhide);

	void Execute();
	void UnExecute();

protected:
	vector<GObject*>	m_pobj;
	bool				m_bhide;
};

//-----------------------------------------------------------------------------
class CCmdHideParts : public CCommand
{
public:
	CCmdHideParts(std::list<GPart*> partList);

	void Execute();
	void UnExecute();

protected:
	std::list<GPart*>	m_partList;
};

//-----------------------------------------------------------------------------
class CCmdShowParts : public CCommand
{
public:
	CCmdShowParts(std::list<GPart*> partList);

	void Execute();
	void UnExecute();

protected:
	std::list<GPart*>	m_partList;
};

//-----------------------------------------------------------------------------
class CCmdHideElements : public CCommand
{
public:
	CCmdHideElements(FEMesh* mesh, const vector<int>& elemList);

	void Execute();
	void UnExecute();

protected:
	FEMesh*			m_mesh;
	vector<int>		m_elemList;
};

//-----------------------------------------------------------------------------
class CCmdHideFaces: public CCommand
{
public:
	CCmdHideFaces(FESurfaceMesh* mesh, const vector<int>& faceList);

	void Execute();
	void UnExecute();

protected:
	FESurfaceMesh*	m_mesh;
	vector<int>		m_faceList;
};

//-----------------------------------------------------------------------------

class CCmdHideSelection : public CCommand
{
public:
	CCmdHideSelection();

	void Execute();
	void UnExecute();

protected:
	vector<int>	m_item;
	int		m_nitem;
};

//-----------------------------------------------------------------------------

class CCmdHideUnselected : public CCommand
{
public:
	CCmdHideUnselected();

	void Execute();
	void UnExecute();

protected:
	int			m_nitem;
	vector<int>	m_item;
};

//-----------------------------------------------------------------------------

class CCmdUnhideAll : public CCommand
{
public:
	CCmdUnhideAll();

	void Execute();
	void UnExecute();

protected:
	vector<int>	m_item;
	int			m_nitem;
	bool		m_bunhide;
};

//-----------------------------------------------------------------------------

class CCmdApplyFEModifier : public CCommand
{
public:
	CCmdApplyFEModifier(FEModifier* pmod, GObject* po, FEGroup* selection = 0);
	~CCmdApplyFEModifier() { delete m_pnew; delete m_pmod; }

	void Execute();
	void UnExecute();

protected:
	GObject*		m_pobj;
	FEMesh*			m_pold;	// old, unmodified mesh
	FEMesh*			m_pnew;	// new, modified mesh
	FEModifier*		m_pmod;
	FEGroup*		m_psel;
};

//-----------------------------------------------------------------------------

class CCmdApplySurfaceModifier : public CCommand
{
public:
	CCmdApplySurfaceModifier(FESurfaceModifier* pmod, GObject* po, FEGroup* selection);
	~CCmdApplySurfaceModifier();

	void Execute();
	void UnExecute();

protected:
	GObject*			m_pobj;
	FESurfaceMesh*		m_pold;	// old, unmodified mesh
	FESurfaceMesh*		m_pnew;	// new, modified mesh
	FESurfaceModifier*	m_pmod;
	FEGroup*			m_psel;
};

//-----------------------------------------------------------------------------

class CCmdChangeFEMesh : public CCommand
{
public:
	CCmdChangeFEMesh(GObject* po, FEMesh* pm, bool up = false);
	~CCmdChangeFEMesh() { delete m_pnew; }

	void Execute();
	void UnExecute();

protected:
	bool		m_update;
	GObject*	m_po;
	FEMesh*		m_pnew;
};

//-----------------------------------------------------------------------------
class CCmdChangeFESurfaceMesh : public CCommand
{
public:
	CCmdChangeFESurfaceMesh(GSurfaceMeshObject* po, FESurfaceMesh* pm, bool up = false);
	~CCmdChangeFESurfaceMesh();

	void Execute();
	void UnExecute();

protected:
	bool				m_update;
	GSurfaceMeshObject*	m_po;
	FESurfaceMesh*		m_pnew;
};

//-----------------------------------------------------------------------------

class CCmdChangeView : public CCommand
{
public:
	CCmdChangeView(CGLView* pview, CGLCamera cam);
	~CCmdChangeView();

	void Execute();
	void UnExecute();

protected:
	CGLView*	m_pview;
	CGLCamera	m_cam;
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
// Convert an object to an editable mesh
class CCmdConvertToEditableMesh : public CCommand
{
public:
	CCmdConvertToEditableMesh(GObject* po);
	~CCmdConvertToEditableMesh();

	void Execute();
	void UnExecute();

protected:
	GObject*	m_pold;	// the original object
	GObject*	m_pnew;	// the new mesh object
};

//-----------------------------------------------------------------------------
// Convert a surface mesh to an editable mesh
class CCmdConvertSurfaceToEditableMesh : public CCommand
{
public:
	CCmdConvertSurfaceToEditableMesh(GObject* po);
	~CCmdConvertSurfaceToEditableMesh();

	void Execute();
	void UnExecute();

protected:
	GObject*	m_pold;	// the original object
	GObject*	m_pnew;	// the new mesh object
};


//-----------------------------------------------------------------------------
// Convert an object to an editable surface
class CCmdConvertToEditableSurface : public CCommand
{
public:
	CCmdConvertToEditableSurface(GObject* po);
	~CCmdConvertToEditableSurface();

	void Execute();
	void UnExecute();

protected:
	GObject*	m_pold;	// the original object
	GObject*	m_pnew;	// the new mesh object
};

//-----------------------------------------------------------------------------
// Convert a primitive to a multi-block mesh
class CCmdConvertToMultiBlock : public CCommand
{
public:
	CCmdConvertToMultiBlock(GObject* po);
	~CCmdConvertToMultiBlock();

	void Execute();
	void UnExecute();

protected:
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
	CCmdAddStep(FEStep* ps);
	~CCmdAddStep();

	void Execute();
	void UnExecute();

protected:
	FEStep*	m_pstep;
};

//----------------------------------------------------------------
class CCmdAddMaterial : public CCommand
{
public:
	CCmdAddMaterial(GMaterial* pm);
	~CCmdAddMaterial();

	void Execute();
	void UnExecute();

protected:
	GMaterial* m_pm;
};

//-----------------------------------------------------------------------------
class CCmdSetBCItemList : public CCommand
{
public:
	CCmdSetBCItemList(FEBoundaryCondition* pbc, FEItemListBuilder* pl);
	~CCmdSetBCItemList();

	void Execute();
	void UnExecute();

protected:
	FEBoundaryCondition*	m_pbc;
	FEItemListBuilder*		m_pl;
};

//-----------------------------------------------------------------------------
class CCmdUnassignBC : public CCommand
{
public:
	CCmdUnassignBC(FEBoundaryCondition* pbc);
	~CCmdUnassignBC();

	void Execute();
	void UnExecute();

protected:
	FEBoundaryCondition*	m_pbc;
	FEItemListBuilder*		m_pl;
};

//-----------------------------------------------------------------------------
class CCmdAddToItemListBuilder : public CCommand
{
public:
	CCmdAddToItemListBuilder(FEItemListBuilder* pold, list<int>& lnew);

	void Execute();
	void UnExecute();

protected:
	FEItemListBuilder* m_pold;
	list<int>	m_lnew;
	vector<int>	m_tmp;
};

//-----------------------------------------------------------------------------
class CCmdRemoveFromItemListBuilder : public CCommand
{
public:
	CCmdRemoveFromItemListBuilder(FEItemListBuilder* pold, list<int>& lnew);

	void Execute();
	void UnExecute();

protected:
	FEItemListBuilder* m_pold;
	list<int>	m_lnew;
	vector<int>	m_tmp;
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
