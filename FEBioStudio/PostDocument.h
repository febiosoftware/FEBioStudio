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
#include "GLModelDocument.h"
#include <PostLib/Material.h>
#include <CUILib/GraphData.h>

class CModelDocument;
class CPostObject;
class CPalette;

namespace Post {
	class FEPostModel;
	class FEFileReader;
	class CGLModel;
	class CGLObject;
}

// Timer modes
#define MODE_FORWARD	0
#define MODE_REVERSE	1
#define MODE_CYLCE		2

//-----------------------------------------------------------------------------
struct TIMESETTINGS
{
	int		m_mode;		// play mode
	double	m_fps;		// frames per second
	int		m_start;	// start time
	int		m_end;		// end time
	int		m_inc;		// used when MODE_CYCLE
	bool	m_bloop;	// loop or not
	bool	m_bfix;		// use a fixed time step
	double	m_dt;		// fixed time step size

	void Defaults();
};

//-----------------------------------------------------------------------------
// model data which is used for file updates
class ModelData
{
private:
	struct MODEL
	{
		int		m_ntime;
		bool	m_bnorm;	// calculate normals or not
		bool	m_boutline;	// render as outline
		bool	m_bghost;	// render ghost
		bool	m_bShell2Solid; // render shells as solids
		bool	m_bBeam2Solid; // render beams as solids
		int		m_nshellref;	// shell reference surface
		int		m_nDivs;	// nr of element subdivisions
		int		m_nrender;	// render mode
		double	m_smooth;	// smoothing angle
	};

	struct COLORMAP
	{
		bool	m_bactive;
		int		m_ntype;
		int		m_ndivs;
		bool	m_bsmooth;	// smooth gradient or not
		float	m_min;
		float	m_max;
		float	m_user[2];	// user range
		bool	m_bDispNodeVals;	// render nodal values
		int		m_maxRangeType;	// range type
		int		m_minRangeType;	// range type
		int		m_nField;
	};

	struct DISPLACEMENTMAP
	{
		int		m_nfield;	// vector field defining the displacement
		float	m_scale;	// displacement scale factor
	};

public:
	ModelData();
	void ReadData(Post::CGLModel* po);
	void WriteData(Post::CGLModel* po);

	bool IsValid() const;

protected:
	bool		m_isValid;
	MODEL						m_mdl;	// CGLModel data
	COLORMAP					m_cmap;	// CColorMap data
	DISPLACEMENTMAP				m_dmap;	// DisplacementMap data
	std::vector<Post::Material>		m_mat;	// material list
	std::vector<std::string>	m_data;	// data field strings
};


class CPostDocument : public CGLModelDocument
{
	Q_OBJECT

public:
	CPostDocument(CMainWindow* wnd, CModelDocument* parent = nullptr);
	~CPostDocument();

	void Clear() override;

	bool Initialize() override;

	GObject* GetActiveObject() override;

	bool IsValid();

	void SetModifiedFlag(bool bset = true) override;

	void SetInitFlag(bool b);

	void UpdateSelection() override;

public: // overrides for CGLModelDocument

	Post::CGLModel* GetGLModel() override;

	Post::FEPostModel* GetFSModel() override;

public:
	int GetStates();

	void SetActiveState(int n);

	int GetActiveState();

	void SetDataField(int n);

	bool MergeFEModel(Post::FEPostModel* fem);

	CPostObject* GetPostObject();

	TIMESETTINGS& GetTimeSettings();

	float GetTimeValue(int n);

	void SetCurrentTimeValue(float ftime);

	void UpdateAllStates();

	void UpdateFEModel(bool breset = false);

	int GetEvalField();

	void ActivateColormap(bool bchecked);

	void DeleteObject(Post::CGLObject* po);

	void Activate() override;

	std::string GetFileName();

	// get the model's bounding box
	BOX GetBoundingBox();

	void SetGLModel(Post::CGLModel* glm);

public:
	//! save to session file
	bool SavePostSession(const std::string& fileName);

public:
	int Graphs() const;
	void AddGraph(const CGraphData& data);
	const CGraphData* GetGraphData(int i);
	int FindGraphData(const CGraphData* data);
	void ReplaceGraphData(int n, const CGraphData& data);
	void DeleteGraph(const CGraphData* data);

private:
	void ApplyPalette(const CPalette& pal);

signals:
	void selectionChanged();

private:
	CModelDocument*	m_doc;

private:
	Post::CGLModel*		m_glm;
	Post::FEPostModel*	m_fem;
	std::string			m_fileName;

	std::vector<CGraphData*>	m_graphs;

	ModelData	m_MD;

	TIMESETTINGS m_timeSettings;

	bool	m_binit;
};
