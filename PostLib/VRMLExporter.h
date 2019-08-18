#pragma once

#include "FEModel.h"
#include <list>
#include <stdio.h>
namespace Post {
class VRMLExporter
{
public:
	VRMLExporter(void);
	~VRMLExporter(void);

	bool Save(FEModel* pscene, const char* szfile);

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
	FEModel*	m_pscene;
	FILE*	m_fp;
	char	m_sztab[256];
};

}
