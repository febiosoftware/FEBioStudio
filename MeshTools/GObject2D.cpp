#include "stdafx.h"
#include "GObject2D.h"
#include "FEAdvancingFrontMesher2D.h"

GObject2D::GObject2D() : GObject(GOBJECT2D)
{
	m_pMesher = new FEAdvancingFrontMesher2D(this);
}
