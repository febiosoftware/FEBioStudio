#pragma once
#include <string>
#include <GeomLib/GObject.h>

class CGLView;

namespace Post {
	class FEModel;
	class CGLModel;
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

class CPostDoc
{
	class Imp;

public:	
	CPostDoc();
	~CPostDoc();

	bool Load(const std::string& fileName);

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

	void UpdateAllStates();

	int GetEvalField();

private:
	Imp*	imp;
};

class CPostObject : public GObject
{
	// make sure this number does not coincide with any in PreViewLib/enum.h
	enum { POST_OBJECT = 100 };

public:
	CPostObject(Post::CGLModel* glm);

	void BuildGMesh() override;

	// build the FEMesh
	FEMesh* BuildMesh() override;

	FEMeshBase* GetEditableMesh() override;

	FELineMesh* GetEditableLineMesh() override;

	// is called whenever the selection has changed
	void UpdateSelection() override;

	void UpdateMesh();

private:
	Post::CGLModel* m_glm;
};
