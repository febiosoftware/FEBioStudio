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
#include "FEStepComponent.h"

//=============================================================================
// Rigid loads
//=============================================================================
// Base class for all rigid body loads
class FSRigidLoad : public FSStepComponent
{
public:
	FSRigidLoad(int ntype, FSModel* ps, int nstep);

	virtual void SetMaterialID(int n) = 0;
	virtual int GetMaterialID() const = 0;

	int Type() const { return m_type; }

	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

private:
	int	m_type;
};

class FEBioRigidLoad : public FSRigidLoad
{
public:
	FEBioRigidLoad(FSModel* ps, int nstep = 0);

	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

	void SetMaterialID(int n) override;
	int GetMaterialID() const override;
};

//=============================================================================
// Rigid BCs
//=============================================================================
// Base class for all rigid body bc
class FSRigidBC : public FSStepComponent
{
public:
	FSRigidBC(int ntype, FSModel* ps, int nstep);

	virtual void SetMaterialID(int n) = 0;
	virtual int GetMaterialID() const = 0;

	int Type() const { return m_type; }

	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

private:
	int	m_type;
};

class FEBioRigidBC : public FSRigidBC
{
public:
	FEBioRigidBC(FSModel* ps, int nstep = 0);
	void Save(OArchive& ar);
	void Load(IArchive& ar);

	void SetMaterialID(int n) override;
	int GetMaterialID() const override;
};

//=============================================================================
// Rigid BCs
//=============================================================================
// Base class for all rigid body ic
class FSRigidIC : public FSStepComponent
{
public:
	FSRigidIC(int ntype, FSModel* ps, int nstep);

	virtual void SetMaterialID(int n) = 0;
	virtual int GetMaterialID() const = 0;

	int Type() const { return m_type; }

	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

private:
	int	m_type;
};

class FEBioRigidIC : public FSRigidIC
{
public:
	FEBioRigidIC(FSModel* ps, int nstep = 0);

	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

	void SetMaterialID(int n) override;
	int GetMaterialID() const override;
};
