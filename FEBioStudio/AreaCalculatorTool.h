//
//  AreaCalculatorTool.h
//  FEBioStudio
//
#pragma once
#include "Tool.h"

//-----------------------------------------------------------------------------
class CAreaCalculatorTool : public CBasicTool
{
public:
    // constructor
    CAreaCalculatorTool(CMainWindow* wnd);

    // method called when user presses Apply button (optional)
    bool OnApply();

private:
    double    m_Ax, m_Ay, m_Az;
    double    m_A;
};
