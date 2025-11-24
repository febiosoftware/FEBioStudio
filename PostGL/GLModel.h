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
#include <PostLib/GLObject.h>
#include "GLDisplacementMap.h"
#include "GLColorMap.h"
#include <PostLib/FEPostModel.h>
#include "PostObject.h"
#include <GLLib/GDecoration.h>
#include <PostGL/GLPlotGroup.h>
#include <FSCore/FSObjectList.h>
#include <MeshLib/Intersect.h>
#include <MeshTools/FESelection.h>
#include <vector>
#include <memory>

namespace Post {

typedef FSObjectList<Post::CGLPlot>	GPlotList;

// view conventions
enum View_Convention {
	CONV_FR_XZ,
	CONV_FR_XY,
	CONV_US_XY
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

	void add(const FSFace& f) { m_Face.push_back(f); }

	int Faces() const { return (int) m_Face.size(); }

	FSFace& Face(int i) { return m_Face[i]; }

	const vector<FSFace>& FaceList() const { return m_Face; }

private:
	vector<FSFace>	m_Face;
};

class GLEdge
{
public:
	struct EDGE
	{
		int n0, n1;
		int mat;
		int	elem;
		float tex[2];
	};

	void AddEdge(const EDGE& e) { m_Edge.push_back(e); }
	int Edges() const { return (int) m_Edge.size(); }
	EDGE& Edge(int i) { return m_Edge[i]; }

	void Clear() { m_Edge.clear(); }

protected:
	vector<EDGE>	m_Edge;
};

class CGLModel : public FSObject
{
public:
	CGLModel(FEPostModel* ps);
	~CGLModel(void);

	void Clear();

	bool IsValid() const;

	void SetFEModel(FEPostModel* ps);

	CPostObject* GetPostObject();

	CGLDisplacementMap* GetDisplacementMap() { return m_pdis; }
	CGLColorMap* GetColorMap() { return m_pcol; }
	FEPostModel* GetFSModel() { return m_ps; }

	bool Update(bool breset) override;
	void UpdateDisplacements(int nstate, bool breset = false);

	bool AddDisplacementMap(const char* szvectorField = 0);

	void RemoveDisplacementMap();

	bool HasDisplacementMap();

	//! set the smoothing angle
	void SetSmoothingAngle(double w);

	//! get the smoothing angle in degrees
	double GetSmoothingAngle() { return m_stol; }

	//! get the smoothing angle in radians
	double GetSmoothingAngleRadians() { return PI*m_stol/180.0; }

	//! get the active mesh
	FSMesh* GetActiveMesh();

	//! get the active state
	Post::FEState* GetActiveState();

	//! reset the mesh nodes
	void ResetMesh();

public:
	bool ShowNormals() { return m_bnorm; }
	void ShowNormals(bool b) { m_bnorm = b; }

	void ShowShell2Solid(bool b);
	bool ShowShell2Solid() const;

	int ShellReferenceSurface() const;
	void ShellReferenceSurface(int n);

	void ShowBeam2Solid(bool b);
	bool ShowBeam2Solid() const;

	void SolidBeamRadius(float f);
	float SolidBeamRadius() const;

	void SetSubDivisions(int ndivs);
	int GetSubDivisions();

	int GetRenderMode() { return m_nrender; }
	void SetRenderMode(int nmode) { m_nrender = nmode; }

    int GetViewConvention() { return m_nconv; }
    void SetViewConvention(int nmode) { m_nconv = nmode; }
    
	bool RenderInteriorNodes() const { return m_brenderInteriorNodes; }
	void RenderInteriorNodes(bool b) { m_brenderInteriorNodes = b; }

	GLColor GetGhostColor() const { return m_ghost_color; }
	void SetGhostColor(GLColor c) { m_ghost_color = c; }

public:
	bool RenderInnerSurfaces();
	void RenderInnerSurfaces(bool b);

public:
	float CurrentTime() const;
	int CurrentTimeIndex() const;

	void SetCurrentTimeIndex(int n);

	// set the active state closest to time t
	void SetTimeValue(float ftime);

public: // Selection
	void SetSelection(FESelection* sel);

	void SelectElements(vector<int>& items, bool bclear);

	//! unhide all items
	void UnhideAll();

	// selection
	void SelectNodesInRange(float fmin, float fmax, bool bsel);
	void SelectEdgesInRange(float fmin, float fmax, bool bsel);
	void SelectFacesInRange(float fmin, float fmax, bool bsel);
	void SelectElemsInRange(float fmin, float fmax, bool bsel);

	// --- V I S I B I L I T Y ---
	//! hide elements by material ID
	void HideMaterial(int nmat);

	//! show elements by material ID
	void ShowMaterial(int nmat);

	//! update visibility of all materials
	void UpdateMeshVisibility();

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
	int GetSelectionType() const { return m_selectType; }

	// set selection type
	void SetSelectionType(SelectionType mode) { m_selectType = mode; }

	// get a list of selected items
	void GetSelectionList(vector<int>& L, int mode);

	// get selection style
	int GetSelectionStyle() const { return m_selectStyle; }

	// set selection style
	void SetSelectionStyle(int n) { m_selectStyle = n; }

public:
	int DiscreteEdges();
	GLEdge::EDGE& DiscreteEdge(int i);

public:
	// edits plots
	void AddPlot(Post::CGLPlot* pplot, bool update = true);
	void RemovePlot(Post::CGLPlot* pplot);

	GPlotList& GetPlotList() { return m_pPlot; }
	void ClearPlots();

	int Plots() { return (int)m_pPlot.Size(); }
	CGLPlot* Plot(int i) { return m_pPlot[i]; }

	void MovePlotUp(Post::CGLPlot* plot);
	void MovePlotDown(Post::CGLPlot* plot);

	void UpdateColorMaps();

	void UpdateEdge();

protected:
	void UpdateInternalSurfaces(bool eval = true);
	void UpdateSelectionMesh();

public:
	bool		m_bnorm;		//!< calculate normals or not
	double		m_scaleNormals;	//!< normal scale factor
	bool		m_bghost;		//!< render the ghost (undeformed outline)
	bool		m_brenderInteriorNodes;	//!< render interior nodes or not
	int			m_nDivs;		//!< nr of element subdivisions
	int			m_nrender;		//!< render mode
    int         m_nconv;        //!< multiview projection convention
	GLColor		m_line_col;		//!< line color
	GLColor		m_node_col;		//!< color for rendering (unselected) nodes
	GLColor		m_sel_col;		//!< selection color
	GLColor		m_col_inactive;	//!< color for inactive parts
	GLColor		m_ghost_color;	//!< color for the "ghost"
	double		m_stol;			//!< smoothing threshold
	bool		m_renderInnerSurface;	//!< render the inner surfaces

	float		m_solidBeamRadius;
	bool		m_bShell2Solid;
	bool		m_bBeam2Solid;

	int		m_nshellref;

	bool		m_bshowMesh;
	bool		m_doZSorting;

public:
	FEPostModel*	m_ps;

	CPostObject* m_postObj;

	GLEdge					m_edge;	// all line elements from springs

	CGLDisplacementMap*		m_pdis;
	CGLColorMap*			m_pcol;

	FSMesh*	m_lastMesh;	// mesh of last evaluated state

	// selected items
	FESelection* m_selection;
	std::unique_ptr<GLMesh> m_selectionMesh;

	GPlotList			m_pPlot;	// list of plots

	// TODO: move to document?
	std::list<GDecoration*>	m_decor;

	// --- Selection ---
	SelectionType	m_selectType;		//!< current selection type (node, edge, face, elem)
	int				m_selectStyle;		//!< selection style (box, circle, rect)
};

// This class provides a convenient way to loop over all the plots in a model, traversing
// plot-groups recursively. 
class GLPlotIterator
{
public:
	GLPlotIterator(CGLModel* mdl);

	void operator ++ ();

	operator CGLPlot* ();

private:
	int	m_n;	// index in plot list
	std::vector<Post::CGLPlot*>	m_plt;
};

}
