#pragma once
#include <string>

class CGLView;

namespace Post {
	class FEModel;
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

private:
	Imp*	imp;
};
