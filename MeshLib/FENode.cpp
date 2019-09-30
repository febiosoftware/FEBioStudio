#include "FENode.h"

FENode::FENode()
{
}

FENode::FENode(const FENode& n) : FEItem(n)
{
	r = n.r;
}

void FENode::operator = (const FENode& n)
{
	r = n.r;
	FEItem::operator=(n);
}
