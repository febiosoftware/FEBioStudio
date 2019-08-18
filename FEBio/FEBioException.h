#pragma once

class FSObject;

//-----------------------------------------------------------------------------
//! general purpose FEBio error
class FEBioExportError {};

//-----------------------------------------------------------------------------
//! Thrown when a constraint is assigned to a non-rigid material
class InvalidMaterialReference {};

//-----------------------------------------------------------------------------
//! Thrown when the item list is undefined or invalid 
class InvalidItemListBuilder
{
public:
	InvalidItemListBuilder(FSObject* po) : m_po(po) {}
	FSObject*	m_po;
};

//-----------------------------------------------------------------------------
//! Thrown when a constraint has not been assigned to a rigid body
class MissingRigidBody
{
public:
	MissingRigidBody(const string& rb = "") { m_rbName = rb; }
	string m_rbName;
};

//-----------------------------------------------------------------------------
//! Thrown when a constraint has not been assigned to a rigid body
class RigidContactException
{
public:
	RigidContactException() {}
};
