#pragma once
#include <string>
#include <GeomLib/GObject.h>
#include <GeomLib/GMeshObject.h>

class CGLView;

namespace Post {
	class FEModel;
	class CGLModel;
	class FEDataField;
	class CPalette;
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

class CPostDoc : public FSObject
{
	class Imp;

public:	
	CPostDoc();
	~CPostDoc();

	bool LoadPlotfile(const std::string& fileName);

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

	void UpdateAllStates();

	void UpdateFEModel(bool breset = false);

	int GetEvalField();

	std::string GetTitle();

	void ActivateColormap(bool bchecked);

private:
	void ApplyPalette(const Post::CPalette& pal);

private:
	Imp*	imp;
};

class CPostObject : public GMeshObject
{
public:
	CPostObject(Post::CGLModel* glm);

	// build the FEMesh
	FEMesh* BuildMesh() override;

	FEMeshBase* GetEditableMesh() override;

	FELineMesh* GetEditableLineMesh() override;

	// is called whenever the selection has changed
	void UpdateSelection() override;

	void UpdateMesh() override;

	BOX GetBoundingBox();

private:
	Post::CGLModel* m_glm;
};
