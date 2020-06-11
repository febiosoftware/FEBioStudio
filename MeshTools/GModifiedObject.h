/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

#pragma once
#include <GeomLib/GObject.h>
#include "GModifier.h"

//-----------------------------------------------------------------------------
// This class describes an object that has modifiers applied to it.
// It needs another object that it references to which it applies the modifiers.
// The reference object is managed by this object. 
class GModifiedObject :	public GObject
{
public:
	// con/de-structor
	GModifiedObject(GObject* po);
	~GModifiedObject(void);

	// build FE mesh
	FEMesh* BuildMesh();

	// Build the render mesh
	void BuildGMesh();

	// Modifier operations
	GModifierStack* GetModifierStack() { return m_pStack; }
	void AddModifier(GModifier* pmod);
	void DeleteModifier(GModifier* pmod);

	// update object data
	bool Update(bool b = true);

	// return the child object
	GObject* GetChildObject() { return m_po; }
	void SetChildObject(GObject* po, bool bclone = true);

	// serialization
	void Save(OArchive& ar);
	void Load(IArchive& ar);

	// cloning
	GObject* Clone();

	// get the mesher object
	FEMesher* GetFEMesher();

protected:
	void CloneChild();

protected:
	GObject*			m_po;		// reference object
	GModifierStack*		m_pStack;	// modifier stack
};
