#include "stdafx.h"
#include "Sketcher.h"
#include <MeshTools/GPLCObject.h>

//-----------------------------------------------------------------------------
GSketcher* GSketcher::m_pThis = 0;

//-----------------------------------------------------------------------------
GSketcher* GSketcher::GetInstance()
{
	if (m_pThis == 0) m_pThis = new GSketcher;
	return m_pThis;
}

//-----------------------------------------------------------------------------
GSketch& GSketcher::GetCurrentSketch()
{
	return m_sketch;
}

//-----------------------------------------------------------------------------
FEObject* GSketcher::FinalizeSketch()
{
	// create a new PLC object
	GPLCObject* po = new GPLCObject();

	// build the PLC from the sketch
	po->Create(m_sketch);

	// prepare for the next curve
	m_sketch.Clear();

	return po;
}
