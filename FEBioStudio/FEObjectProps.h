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
class CMainWindow;
class CFEBioJob;
class CImageModel;
class GDiscreteObject;

namespace Post {
	class CGLModel;
}

class FEObjectProps : public CObjectProps
{
public:
	FEObjectProps(FSObject* po, FSModel* fem = nullptr);

protected:
	QStringList GetEnumValues(const char* ch) override;

protected:
	FSModel*	m_fem;
};

class CFSObjectProps : public FEObjectProps
{
public:
	CFSObjectProps(FSModel* fem = nullptr) : FEObjectProps(nullptr, fem) {}
	virtual void SetFSObject(FSObject* po);
};

template <class T>
class CFSObjectProps_T : public CFSObjectProps
{
public:
	CFSObjectProps_T(FSModel* fem = nullptr) : CFSObjectProps(fem), m_pobj(nullptr) {}

	void SetFSObject(FSObject* po) override
	{
		Clear();
		m_pobj = dynamic_cast<T*>(po);
		m_po = m_pobj;
		if (m_pobj != nullptr) BuildProperties();
	}

	virtual void BuildProperties()
	{
		BuildParamList(m_pobj);
	}

protected:
	T* m_pobj;
};

class CStepSettings : public CFSObjectProps_T<FSStep>
{
public:
	CStepSettings(FSProject& prj);

	void BuildProperties() override;

	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);

private:
	int		m_moduleId;
};

class CRigidConnectorSettings : public CFSObjectProps_T<FSRigidConnector>
{
public:
	CRigidConnectorSettings(FSModel* fem);

	void BuildProperties() override;

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	std::vector<GMaterial*>	m_mat;
	int					m_rbA;
	int					m_rbB;
};

class CMaterialProps : public CFSObjectProps_T<GMaterial>
{
public:
	CMaterialProps(FSModel* fem);

	void BuildProperties() override;

	QVariant GetPropertyValue(int i) override;

	void SetPropertyValue(int i, const QVariant& v) override;

private:
	void BuildPropertyList();
	FSMaterial* m_mat;
};

class CLogfileProperties : public CFSObjectProps
{
public:
	CLogfileProperties(CModelViewer* wnd, FSProject& prj);

	void SetFSObject(FSObject* po) override
	{
		Update();
	}

	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);
	void Update() override;

private:
	int	m_actionIndex;
	CModelViewer* m_wnd;
	FSProject* m_prj;
};


class CReactionReactantProperties : public CFSObjectProps_T<FSReactionMaterial>
{
public:
	CReactionReactantProperties(FSModel* fem);

	void BuildProperties() override;

	QVariant GetPropertyValue(int i) override;

	void SetPropertyValue(int i, const QVariant& v) override;

private:
	int					m_nsols;
};

class CReactionProductProperties : public CFSObjectProps_T<FSReactionMaterial>
{
public:
	CReactionProductProperties(FSModel* fem);

	void BuildProperties() override;

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	int m_nsols;
};

class CPartProperties : public CFSObjectProps_T<GPart>
{
public:
	CPartProperties(FSModel& fem) : CFSObjectProps_T(&fem) {}
	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);

	QStringList GetEnumValues(const char* ch) override;

	void BuildProperties() override;
};

class CImageModelProperties : public CFSObjectProps_T<CImageModel>
{
public:
	CImageModelProperties();

	void BuildProperties() override;

	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);

private:
	enum PropOrder {PIXELTYPE, PXLDIM, SHOWBOX, X0, Y0, Z0, X1, Y1, Z1};
};

class CFEBioJobProps : public CFSObjectProps_T<CFEBioJob>
{
public:
	CFEBioJobProps(CMainWindow* wnd, CModelViewer* tree);

	void BuildProperties() override;

	QVariant GetPropertyValue(int i) override;

	void SetPropertyValue(int i, const QVariant& v) override;

private:
	CModelViewer*	m_tree;
	CMainWindow*	m_wnd;
};

class CPlotfileProperties : public CFSObjectProps
{
public:
	CPlotfileProperties(CModelViewer* wnd, FSProject& prj);

	void SetFSObject(FSObject* po) override;

	void Update() override;

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	CModelViewer* m_wnd;
	FSProject& m_prj;
	int	m_actionIndex;
};

class CPostModelProps : public CFSObjectProps_T<Post::CGLModel>
{
public:
	CPostModelProps() {}

	void BuildProperties() override;

	QVariant GetPropertyValue(int i) override;

	void SetPropertyValue(int i, const QVariant& v) override;
};

class CDiscreteObjectProps : public CFSObjectProps_T<GDiscreteObject>
{
public:
	CDiscreteObjectProps() {}

	void BuildProperties() override;
};

class FSGlobalsProps : public CFSObjectProps
{
public:
	FSGlobalsProps(FSModel* fem) : CFSObjectProps(fem) {}

	void SetFSObject(FSObject* po) override;
};
