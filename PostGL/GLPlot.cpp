#include "stdafx.h"
#include "GLPlot.h"
#include <GLWLib/GLWidgetManager.h>
using namespace Post;

CGLPlot::CGLPlot(CGLModel* po) : CGLVisual(po)
{
	m_renderOrder = 0;
}

CGLPlot::~CGLPlot()
{
}

void CGLPlot::UpdateTexture() 
{

}

void CGLPlot::SetRenderOrder(int renderOrder)
{
	m_renderOrder = renderOrder;
}

int CGLPlot::GetRenderOrder() const
{
	return m_renderOrder;
}

CGLLegendPlot::CGLLegendPlot(CGLModel* po) : CGLPlot(po)
{
	m_pbar = nullptr;
}

CGLLegendPlot::~CGLLegendPlot()
{
	SetLegendBar(nullptr);
}

void CGLLegendPlot::SetLegendBar(GLLegendBar* bar)
{
	if (m_pbar)
	{
		CGLWidgetManager::GetInstance()->RemoveWidget(m_pbar);
		delete m_pbar;
	}

	m_pbar = bar;
	if (m_pbar) CGLWidgetManager::GetInstance()->AddWidget(m_pbar);
}

GLLegendBar* CGLLegendPlot::GetLegendBar()
{
	return m_pbar;
}

void CGLLegendPlot::ChangeName(const std::string& name)
{
	CGLObject::SetName(name);
	if (m_pbar) m_pbar->copy_label(name.c_str());
}

bool CGLLegendPlot::ShowLegend() const
{ 
	return (m_pbar ? m_pbar->visible() : false);
}

void CGLLegendPlot::ShowLegend(bool b) 
{ 
	if (m_pbar)
	{
		if (b) m_pbar->show(); else m_pbar->hide();
	}
}
