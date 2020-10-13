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
#include "ObjectProps.h"

class GObject;
class FEModel;
class GMaterial;
class FEFixedDOF;
class FEPrescribedDOF;
class FESoluteMaterial;
class FEAnalysisStep;
class FERigidInterface;
class FERigidConstraint;
class FERigidConnector;
class FEProject;
class Param;
class FEReactionMaterial;
class FEMaterial;

class FEObjectProps : public CObjectProps
{
public:
	FEObjectProps(FSObject* po, FEModel* fem = nullptr);

protected:
	QStringList GetEnumValues(const char* ch) override;

private:
	FEModel*	m_fem;
};

class CFixedDOFProps : public CPropertyList
{
public:
	CFixedDOFProps(FEFixedDOF* pbc);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	FEFixedDOF*	m_bc;
};

class CAnalysisTimeSettings : public CObjectProps
{
public:
	CAnalysisTimeSettings(FEAnalysisStep* step);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	FEAnalysisStep*	m_step;
};

class CRigidInterfaceSettings : public CPropertyList
{
public:
	CRigidInterfaceSettings(FEModel&fem, FERigidInterface* pi);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	FERigidInterface*	m_ri;
	vector<GMaterial*>	m_mat;
	int					m_sel;
};

class CRigidConstraintSettings : public CObjectProps
{
public:
	CRigidConstraintSettings(FEModel& fem, FERigidConstraint* rc);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	FERigidConstraint*	m_rc;
	vector<GMaterial*>	m_mat;
	int					m_sel;
};

class CRigidConnectorSettings : public CObjectProps
{
public:
	CRigidConnectorSettings(FEModel& fem, FERigidConnector* rc);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	FERigidConnector*	m_rc;
	vector<GMaterial*>	m_mat;
	int					m_rbA;
	int					m_rbB;
};


class CMaterialProps : public FEObjectProps
{
public:
	CMaterialProps(FEModel& fem, FEMaterial* mat);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	void BuildPropertyList();

private:
	FEMaterial*	m_mat;
};

class CLogfileProperties : public CObjectProps
{
public:
	CLogfileProperties(FEProject& prj);
	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);
};


class CReactionReactantProperties : public CObjectProps
{
public:
	CReactionReactantProperties(FEReactionMaterial* mat, FEModel& fem);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	FEReactionMaterial*	m_mat;
	int					m_nsols;
};

class CReactionProductProperties : public CObjectProps
{
public:
	CReactionProductProperties(FEReactionMaterial* mat, FEModel& fem);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	FEReactionMaterial*	m_mat;
	int					m_nsols;
};
