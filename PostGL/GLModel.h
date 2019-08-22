#pragma once
#include "PostLib/GLObject.h"
#include "GLDisplacementMap.h"
#include "GLColorMap.h"
#include "PostLib/FEModel.h"
#include "PostLib/GDecoration.h"
#include "GLPlot.h"
#include <FSCore/FSObjectList.h>
#include <vector>

namespace Post {

//-----------------------------------------------------------------------------
typedef FSObjectList<Post::CGLPlot>	GPlotList;

//-----------------------------------------------------------------------------
// view conventions
enum View_Convention {
	CONV_FR_XZ,
	CONV_FR_XY,
	CONV_US_XY
};

// the selection modes
enum Selection_Mode {
	SELECT_NODES = 1,
	SELECT_EDGES = 2,
	SELECT_FACES = 4,
	SELECT_ELEMS = 8,
	SELECT_ADD = 16,
	SELECT_SUB = 32
};

// Selection Styles
enum Selection_Style {
	SELECT_RECT = 1,
	SELECT_CIRCLE = 2,
	SELECT_FREE = 3
};

class GLSurface
{
public:
	GLSurface(){}

	void add(const FEFace& f) { m_Face.push_back(f); }

	int Faces() const { return (int) m_Face.size(); }

	FEFace& Face(int i) { return m_Face[i]; }

private:
	vector<FEFace>	m_Face;
};

class GLEdge
{
public:
	struct EDGE
	{
		int n0, n1;
		int mat;
	};

	void AddEdge(const EDGE& e) { m_Edge.push_back(e); }
	int Edges() const { return (int) m_Edge.size(); }
	EDGE& Edge(int i) { return m_Edge[i]; }

	void Clear() { m_Edge.clear(); }

protected:
	vector<EDGE>	m_Edge;
};

class CGLModel : public CGLVisual
{
public:
	CGLModel(FEModel* ps);
	~CGLModel(void);

	CGLDisplacementMap* GetDisplacementMap() { return m_pdis; }
	CGLColorMap* GetColorMap() { return m_pcol; }
	FEModel* GetFEModel() { return m_ps; }

	bool Update(bool breset) override;
	void UpdateDisplacements(int nstate, bool breset = false);

	bool AddDisplacementMap(const char* szvectorField = 0);

	void RemoveDisplacementMap();

	bool HasDisplacementMap();

	void SetMaterialParams(FEMaterial* pm);

	//! set the smoothing angle
	void SetSmoothingAngle(double w);

	//! get the smoothing angle in degrees
	double GetSmoothingAngle() { return 180 * m_stol / PI; }

	//! get the smoothing angle in radians
	double GetSmoothingAngleRadians() { return m_stol; }

	//! get the active mesh
	FEMeshBase* GetActiveMesh();

	//! Reset all the states so any update will force the state to be evaluated
	void ResetAllStates();

	//! reset the mesh nodes
	void ResetMesh();

public:
	// return internal surfaces
	int InternalSurfaces() { return (int) m_innerSurface.size(); }
	GLSurface& InteralSurface(int i) { return *m_innerSurface[i]; }

public:
	bool ShowNormals() { return m_bnorm; }
	void ShowNormals(bool b) { m_bnorm = b; }

	bool RenderSmooth() { return m_bsmooth; }
	void RenderSmooth(bool b) { m_bsmooth = b; }

	void ShowShell2Hex(bool b) { m_bShell2Hex = b; }
	bool ShowShell2Hex() { return m_bShell2Hex; }

	int ShellReferenceSurface() { return m_nshellref; }
	void ShellReferenceSurface(int n) { m_nshellref = n; }

	void SetSubDivisions(int ndivs) { m_nDivs = ndivs; }
	int GetSubDivisions();

	int GetRenderMode() { return m_nrender; }
	void SetRenderMode(int nmode) { m_nrender = nmode; }

    int GetViewConvention() { return m_nconv; }
    void SetViewConvention(int nmode) { m_nconv = nmode; }
    
	bool RenderInteriorNodes() const { return m_brenderInteriorNodes; }
	void RenderInteriorNodes(bool b) { m_brenderInteriorNodes = b; }

public:
	// call this to render the model
	void Render(CGLContext& rc);

	void RenderPlots(CGLContext& rc);

public:
	void RenderNodes(FEModel* ps, CGLContext& rc);
	void RenderEdges(FEModel* ps, CGLContext& rc);
	void RenderFaces(FEModel* ps, CGLContext& rc);
	void RenderElems(FEModel* ps, CGLContext& rc);
	void RenderSurface(FEModel* ps, CGLContext& rc);

public:
	void RenderOutline(CGLContext& rc, int nmat = -1);
	void RenderNormals(CGLContext& rc);
	void RenderGhost  (CGLContext& rc);
	void RenderDiscrete(CGLContext& rc);

	void RenderSelection(CGLContext& rc);

	void RenderDecorations();

//	void RenderFeatureEdges(FEModel* ps);
	void RenderMeshLines(FEModel* ps, int nmat);
	void RenderThickShell(FEFace& face, FEMeshBase* pm);
	void RenderThickQuad(FEFace& face, FEMeshBase* pm);
	void RenderThickTri(FEFace& face, FEMeshBase* pm);
	void RenderThickShellOutline(FEFace& face, FEMeshBase* pm);
	void RenderShadows(FEModel* ps, const vec3d& lp, float inf);

	void AddDecoration(GDecoration* pd);
	void RemoveDecoration(GDecoration* pd);

protected:
	void RenderFace(FEFace& face, FEMeshBase* pm, int ndivs, bool bnode);
	void RenderFace(FEFace& face, FEMeshBase* pm, GLColor c[4], int ndivs, bool bnode);
	void RenderTexFace(FEFace& el, FEMeshBase* pm);
	void RenderElementOutline(FEElement& el, FEMeshBase* pm);
	void RenderFaceOutline(FEFace& face, FEMeshBase* pm, int ndivs);
	void RenderSolidPart(FEModel* ps, CGLContext& rc, int mat);
	void RenderSolidMaterial(FEModel* ps, int m);
	void RenderTransparentMaterial(CGLContext& rc, FEModel* ps, int m);
	void RenderFaceEdge(FEFace& face, int j, FEMeshBase* pm, int ndivs);
	void RenderSolidDomain(FEDomain& dom, bool btex, bool benable);

	void RenderInnerSurface(int m);
	void RenderInnerSurfaceOutline(int m, int ndivs);

	void RenderSmoothQUAD4(FEFace& f, FEMeshBase* pm, int ndivs, bool bnode);
	void RenderSmoothQUAD4(vec3f r[4], vec3f n[4], float t[4], int ndivs);

	void RenderSmoothQUAD8(FEFace& face, FEMeshBase* pm, int ndivs, bool bnode);
	void RenderSmoothQUAD8(vec3f r[8], vec3f n[8], float t[8], int ndivs);
	void RenderSmoothQUAD9(FEFace& face, FEMeshBase* pm, int ndivs, bool bnode);
	void RenderSmoothQUAD9(vec3f r[9], vec3f n[9], float t[9], int ndivs);
	void RenderSmoothTRI6(FEFace& face, FEMeshBase* pm, int ndivs, bool bnode);
	void RenderSmoothTRI6(vec3f r[6], vec3f n[8], float t[6], int ndivs);
	void RenderSmoothTRI7(FEFace& face, FEMeshBase* pm, int ndivs, bool bnode);
	void RenderSmoothTRI7(vec3f r[7], vec3f n[7], float t[7], int ndivs);
	void RenderSmoothTRI10(FEFace& face, FEMeshBase* pm, int ndivs, bool bnode);
	void RenderSmoothTRI10(vec3f r[10], vec3f n[10], float t[10], int ndivs);

	void RenderQUAD4(FEFace& f, bool bsmooth, bool bnode);
	void RenderQUAD8(FEFace& f, bool bsmooth, bool bnode);
	void RenderQUAD9(FEFace& f, bool bsmooth, bool bnode);
	void RenderTRI3 (FEFace& f, bool bsmooth, bool bnode);
	void RenderTRI6 (FEFace& f, bool bsmooth, bool bnode);
	void RenderTRI7 (FEFace& f, bool bsmooth, bool bnode);
	void RenderTRI10(FEFace& f, bool bsmooth, bool bnode);

	void RenderTexQUAD4(FEFace& face, FEMeshBase* pm);
	void RenderTexQUAD8(FEFace& face, FEMeshBase* pm);
	void RenderTexTRI3 (FEFace& face, FEMeshBase* pm);
	void RenderTexTRI6 (FEFace& face, FEMeshBase* pm);
	void RenderTexTRI7 (FEFace& face, FEMeshBase* pm);

	void RenderFace1Outline(FEFace& face, FEMeshBase* pm);
	void RenderFace2Outline(FEFace& face, FEMeshBase* pm, int ndivs);
	void RenderFace3Outline(FEFace& face, FEMeshBase* pm, int ndivs);

public:
	float currentTime() const { return m_fTime; }
	int currentTimeIndex() const { return m_nTime; }

	void setCurrentTimeIndex(int n);

	// set the active state closest to time t
	void SetTimeValue(float ftime);

	// get the time value of state n
	float GetTimeValue(int ntime);

	// get the state closest to time t
	int GetClosestTime(double t);

	// get the currently active state
	FEState* currentState();

public: // Selection
	const vector<FENode*>&		GetNodeSelection   () const { return m_nodeSelection; }
	const vector<FEEdge*>&		GetEdgeSelection   () const { return m_edgeSelection; }
	const vector<FEFace*>&		GetFaceSelection   () const { return m_faceSelection; }
	const vector<FEElement*>&	GetElementSelection() const { return m_elemSelection; }
	void UpdateSelectionLists(int mode = -1);
	void ClearSelectionLists();

	void SelectNodes(vector<int>& items, bool bclear);
	void SelectEdges(vector<int>& items, bool bclear);
	void SelectFaces(vector<int>& items, bool bclear);
	void SelectElements(vector<int>& items, bool bclear);

	//! unhide all items
	void UnhideAll();

	//! clear selection
	void ClearSelection();

	//! select connected elements (connected via surface)
	void SelectConnectedSurfaceElements(FEElement& el);

	//! select connected elements (connected via volume)
	void SelectConnectedVolumeElements(FEElement& el);

	//! select connected faces
	void SelectConnectedFaces(FEFace& f, double angleTol);

	//! select connected edges
	void SelectConnectedEdges(FEEdge& e);

	//! select connected nodes on surface
	void SelectConnectedSurfaceNodes(int n);

	//! select connected nodes in volume
	void SelectConnectedVolumeNodes(int n);


	// --- V I S I B I L I T Y ---
	//! hide elements by material ID
	void HideMaterial(int nmat);

	//! show elements by material ID
	void ShowMaterial(int nmat);

	//! enable or disable mesh items based on material's state
	void UpdateMeshState();

	//! hide selected elements
	void HideSelectedElements();
	void HideUnselectedElements();

	//! hide selected faces
	void HideSelectedFaces();

	//! hide selected edges
	void HideSelectedEdges();

	//! hide selected nodes
	void HideSelectedNodes();

	// --- S E L E C T I O N ---

	// get selection mode
	int GetSelectionMode() const { return m_selectMode; }

	// set selection mode
	void SetSelectionMode(int mode) { m_selectMode = mode; }

	// get a list of selected items
	void GetSelectionList(vector<int>& L, int mode);

	// get selection style
	int GetSelectionStyle() const { return m_selectStyle; }

	// set selection style
	void SetSelectionStyle(int n) { m_selectStyle = n; }

	// convert between selections
	void ConvertSelection(int oldMode, int newMode);

	//! Invert selected items
	void InvertSelectedNodes();
	void InvertSelectedEdges();
	void InvertSelectedFaces();
	void InvertSelectedElements();

	// select items
	void SelectAllNodes();
	void SelectAllEdges();
	void SelectAllFaces();
	void SelectAllElements();

public:
	// edits plots
	void AddPlot(Post::CGLPlot* pplot);
	GPlotList& GetPlotList() { return m_pPlot; }
	void ClearPlots();

	int Plots() { return (int)m_pPlot.Size(); }
	CGLPlot* Plot(int i) { return m_pPlot[i]; }

	void UpdateColorMaps();

protected:
	void UpdateInternalSurfaces(bool eval = true);
	void ClearInternalSurfaces();
	void UpdateEdge();

public:
	bool		m_bnorm;		//!< calculate normals or not
	bool		m_bsmooth;		//!< render smooth or not
	bool		m_bghost;		//!< render the ghost (undeformed outline)
	bool		m_brenderInteriorNodes;	//!< render interior nodes or not
	bool		m_bShell2Hex;	//!< render shells as hexes
	int			m_nshellref;	//!< shell reference surface
	int			m_nDivs;		//!< nr of element subdivisions
	int			m_nrender;		//!< render mode
    int         m_nconv;        //!< multiview projection convention
	GLColor		m_line_col;		//!< line color
	GLColor		m_node_col;		//!< color for rendering (unselected) nodes
	GLColor		m_sel_col;		//!< selection color
	GLColor		m_col_inactive;	//!< color for inactive parts
	double		m_stol;			//!< smoothing threshold

	bool		m_bshowMesh;

	float	m_fTime;		// current time value
	int		m_nTime;		// active time step

	unsigned int	m_layer;

protected:
	FEModel*			m_ps;
	vector<GLSurface*>		m_innerSurface;
	GLEdge					m_edge;	// all line elements from springs

	CGLDisplacementMap*		m_pdis;
	CGLColorMap*			m_pcol;

	// selected items
	vector<FENode*>		m_nodeSelection;
	vector<FEEdge*>		m_edgeSelection;
	vector<FEFace*>		m_faceSelection;
	vector<FEElement*>	m_elemSelection;

	GPlotList			m_pPlot;	// list of plots

	// TODO: move to document?
	std::list<GDecoration*>	m_decor;

	// --- Selection ---
	int		m_selectMode;		//!< current selection mode (node, edge, face, elem)
	int		m_selectStyle;		//!< selection style (box, circle, rect)
};
}
