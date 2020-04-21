#pragma once
#include <list>
#include <stdio.h>
namespace Post {

	class FEPostModel;

class VRMLExporter
{
public:
	VRMLExporter(void);
	~VRMLExporter(void);

	bool Save(FEPostModel* pscene, const char* szfile);

protected:
	void inctab();
	void dectab();

	void WriteNode(const char* szname);
	void CloseNode();

	void Write(const char* sz);

	void write_mesh();
	void write_states();
	void write_timer();

protected:
	FEPostModel*	m_pscene;
	FILE*	m_fp;
	char	m_sztab[256];
};

}
