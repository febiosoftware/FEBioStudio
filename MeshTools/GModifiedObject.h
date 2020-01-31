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
