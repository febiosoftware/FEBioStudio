#pragma once
#include "Tool.h"

//-----------------------------------------------------------------------------
class CReadCurveTool : public CBasicTool
{
public:
	// constructor
	CReadCurveTool();

	bool OnApply();

private:
	bool	m_bcheck;
	QString	m_file;
};
