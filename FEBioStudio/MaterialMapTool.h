#pragma once
#include "Tool.h"

class CMaterialMapTool : public CBasicTool
{
public:
	CMaterialMapTool();

	bool OnApply();

private:
	QString	m_file;
	int		m_nmat;
};
