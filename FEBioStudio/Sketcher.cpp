/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

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
FSObject* GSketcher::FinalizeSketch()
{
	// create a new PLC object
	GPLCObject* po = new GPLCObject();

	// build the PLC from the sketch
	po->Create(m_sketch);

	// prepare for the next curve
	m_sketch.Clear();

	return po;
}
