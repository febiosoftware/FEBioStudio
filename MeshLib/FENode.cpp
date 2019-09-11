#include "FENode.h"

FENode::FENode()
{
	m_tex = 0.f;
	m_bext = false; 
}

FENode::FENode(const FENode& n) : FEItem(n)
{
	r = n.r;
	m_bext = n.m_bext;
	m_tex = n.m_tex;
}

void FENode::operator = (const FENode& n)
{
	r = n.r;
	m_bext = n.m_bext;
	m_tex = n.m_tex;
	FEItem::operator=(n);
}
