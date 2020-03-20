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
class xpltFileReader;

//-----------------------------------------------------------------------------
class CPostDoc : public FSObject
{
	class Imp;

public:	
	CPostDoc();
	~CPostDoc();

	bool LoadPlotfile(const std::string& fileName, xpltFileReader* xplt);

	bool ReloadPlotfile();

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
