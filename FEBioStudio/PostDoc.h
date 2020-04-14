#pragma once
#include <string>
#include <GeomLib/GObject.h>
#include <GeomLib/GMeshObject.h>
#include <FSCore/FSObject.h>

class CGLView;

namespace Post {
	class FEModel;
	class CGLModel;
	class FEDataField;
	class CPalette;
	class CGView;
	class CGLObject;
	class FEFileReader;
	class FEMaterial;
}

class CPostObject;

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
		bool	m_bnorm;	// calculate normals or not
		bool	m_boutline;	// render as outline
		bool	m_bghost;	// render ghost
		bool	m_bShell2Hex; // render shells as hexes
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
		int		m_nRangeType;	// range type
		int		m_nField;
	};

	struct DISPLACEMENTMAP
	{
		int		m_nfield;	// vector field defining the displacement
		float	m_scale;	// displacement scale factor
	};

public:
	ModelData(Post::CGLModel* po);
	void SetData(Post::CGLModel* po);

protected:
	MODEL						m_mdl;	// CGLModel data
	COLORMAP					m_cmap;	// CColorMap data
	DISPLACEMENTMAP				m_dmap;	// DisplacementMap data
	std::vector<Post::FEMaterial>		m_mat;	// material list
	std::vector<std::string>	m_data;	// data field strings
};

//-----------------------------------------------------------------------------
class xpltFileReader;

//-----------------------------------------------------------------------------
class CPostDoc : public FSObject
{
	class Imp;

public:	
	CPostDoc();
	~CPostDoc();

	bool LoadPlotfile(const std::string& fileName, xpltFileReader* xplt);

	bool ReloadPlotfile(xpltFileReader* xplt);

	bool LoadFEModel(Post::FEFileReader* fileReader, const char* szfile);

	void Render(CGLView* view);

	bool IsValid();

public:
	int GetStates();

	void SetActiveState(int n);

	int GetActiveState();

	void SetDataField(int n);

	Post::FEModel* GetFEModel();

	Post::CGLModel* GetGLModel();

	CPostObject* GetPostObject();

	TIMESETTINGS& GetTimeSettings();

	std::string GetFieldString();

	float GetTimeValue();

	float GetTimeValue(int n);

	void SetCurrentTimeValue(float ftime);

	void UpdateAllStates();

	void UpdateFEModel(bool breset = false);

	int GetEvalField();

	std::string GetTitle();

	void ActivateColormap(bool bchecked);

	void DeleteObject(Post::CGLObject* po);

	Post::CGView* GetView();

	std::string GetFileName();

	// get the model's bounding box
	BOX GetBoundingBox();

	// get the selection bounding box
	BOX GetSelectionBox();

private:
	void ApplyPalette(const Post::CPalette& pal);

private:
	Imp*	imp;
};

class CPostObject : public GMeshObject
{
public:
	CPostObject(Post::CGLModel* glm);
	~CPostObject();

	// is called whenever the selection has changed
	void UpdateSelection() override;

	void UpdateMesh() override;

	BOX GetBoundingBox();

private:
	Post::CGLModel* m_glm;
};
