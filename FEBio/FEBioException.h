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

#pragma once
#include <FSCore/FSObject.h>
#include <string>

using std::string;

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
	InvalidItemListBuilder(FSObject* po) { if (po) m_name = po->GetName(); }
	InvalidItemListBuilder(const std::string& s) { m_name = s; }
	std::string	m_name;
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

// Thrown when export is cancelled
class CancelExport
{
public:
	CancelExport() {}
};
