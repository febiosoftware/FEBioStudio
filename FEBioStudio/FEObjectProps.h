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
#include "ObjectProps.h"
#include <vector>

class GObject;
class FSModel;
class GMaterial;
class FSFixedDOF;
class FSPrescribedDOF;
class FSSoluteMaterial;
class FSAnalysisStep;
class FSRigidInterface;
class FSRigidConstraint;
class FSRigidConnector;
class FSProject;
class Param;
class FSReactionMaterial;
class FSMaterial;
class GPart;
class FSStep;
class CModelViewer;

class CImageModel;

class FEObjectProps : public CObjectProps
{
public:
	FEObjectProps(FSObject* po, FSModel* fem = nullptr);

protected:
	QStringList GetEnumValues(const char* ch) override;

private:
	FSModel*	m_fem;
};

class CFixedDOFProps : public CPropertyList
{
public:
	CFixedDOFProps(FSFixedDOF* pbc);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	FSFixedDOF*	m_bc;
};

class CAnalysisTimeSettings : public CObjectProps
{
public:
	CAnalysisTimeSettings(FSAnalysisStep* step);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	FSAnalysisStep*	m_step;
};

class CStepSettings : public CObjectProps
{
public:
	CStepSettings(FSProject& prj, FSStep* step);
	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);

private:
	void BuildStepProperties();

private:
	FSStep* m_step;
	int		m_moduleId;
};

class CRigidInterfaceSettings : public CPropertyList
{
public:
	CRigidInterfaceSettings(FSModel&fem, FSRigidInterface* pi);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	FSRigidInterface*	m_ri;
	std::vector<GMaterial*>	m_mat;
	int					m_sel;
};

class CRigidConstraintSettings : public CObjectProps
{
public:
	CRigidConstraintSettings(FSModel& fem, FSRigidConstraint* rc);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	FSRigidConstraint*	m_rc;
	std::vector<GMaterial*>	m_mat;
	int					m_sel;
};

class CRigidConnectorSettings : public CObjectProps
{
public:
	CRigidConnectorSettings(FSModel& fem, FSRigidConnector* rc);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	FSRigidConnector*	m_rc;
	std::vector<GMaterial*>	m_mat;
	int					m_rbA;
	int					m_rbB;
};


class CMaterialProps : public FEObjectProps
{
public:
	CMaterialProps(FSModel& fem, FSMaterial* mat);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	void BuildPropertyList();

private:
	FSMaterial*	m_mat;
};

class CLogfileProperties : public CObjectProps
{
public:
	CLogfileProperties(CModelViewer* wnd, FSProject& prj);
	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);
	void Update() override;

private:
	int	m_actionIndex;
	CModelViewer* m_wnd;
	FSProject* m_prj;
};


class CReactionReactantProperties : public CObjectProps
{
public:
	CReactionReactantProperties(FSReactionMaterial* mat, FSModel& fem);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	FSReactionMaterial*	m_mat;
	int					m_nsols;
};

class CReactionProductProperties : public CObjectProps
{
public:
	CReactionProductProperties(FSReactionMaterial* mat, FSModel& fem);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	FSReactionMaterial*	m_mat;
	int					m_nsols;
};

class CPartProperties : public FEObjectProps
{
public:
	CPartProperties(GPart* pg, FSModel& fem);
	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);

	QStringList GetEnumValues(const char* ch) override;

private:
	GPart*	m_pg;
	FSModel*	m_fem;
};

class CImageModelProperties : public CObjectProps
{
public:
    CImageModelProperties(CImageModel* model);

    QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);

private:
    enum PropOrder {PIXELTYPE, PXLDIM, SHOWBOX, X0, Y0, Z0, X1, Y1, Z1};

private:
    CImageModel* m_model;

};