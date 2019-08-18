#pragma once
#include "PropertyList.h"
#include <vector>
using namespace std;

class FEModel;
class CObject;
class GObject;
class GMaterial;
class FEFixedDOF;
class FEPrescribedDOF;
class FESoluteMaterial;
class FEAnalysisStep;
class FERigidInterface;
class FERigidConstraint;
class FEConnector;
class FEProject;
class Param;
class FEReactionMaterial;
class FEDataMap;

class CObjectProps : public CPropertyList
{
public:
	CObjectProps(CObject* po, FEModel* fem = 0);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

	virtual CObject* GetFEObject() { return m_po; }

	int Params() const { return (int) m_params.size(); }

protected:
	void BuildParamList(CObject* po);

	void AddParameter(Param& p);
	QVariant GetPropertyValue(Param& p);
	void SetPropertyValue(Param& p, const QVariant& v);

protected:
	FEModel*		m_fem;
	CObject*		m_po;
	vector<Param*>	m_params;
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
	CRigidConnectorSettings(FEModel& fem, FEConnector* rc);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	FEConnector*		m_rc;
	vector<GMaterial*>	m_mat;
	int					m_rbA;
	int					m_rbB;
};


class CMaterialProps : public CObjectProps
{
public:	
	CMaterialProps(FEModel& fem, GMaterial* mat);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	void BuildPropertyList();

private:
	GMaterial*	m_mat;
};

class CPlotfileProperties : public CObjectProps
{
public:
	CPlotfileProperties(FEProject& prj);
	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);
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

class CDataMapProps : public CObjectProps
{
public:
	CDataMapProps(FEDataMap* map);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

private:
	void BuildPropertyList();

private:
	FEDataMap*	m_map;
};
