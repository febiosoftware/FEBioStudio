#pragma once
#include <string>

class CGLView;

namespace Post {
	class FEModel;
	class CGLModel;
}

class CPostDoc
{
	class Imp;

public:	
	CPostDoc();
	~CPostDoc();

	bool Load(const std::string& fileName);

	void Render(CGLView* view);

public:
	int GetStates();

	void SetActiveState(int n);

	void SetDataField(int n);

	Post::FEModel* GetFEModel();

	Post::CGLModel* GetGLModel();

private:
	Imp*	imp;
};
