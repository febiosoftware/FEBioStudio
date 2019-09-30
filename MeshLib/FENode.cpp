#include "FENode.h"

FENode::FENode()
{
	m_bext = false; 
}

FENode::FENode(const FENode& n) : FEItem(n)
{
	r = n.r;
	m_bext = n.m_bext;
}

void FENode::operator = (const FENode& n)
{
	r = n.r;
	m_bext = n.m_bext;
	FEItem::operator=(n);
}
