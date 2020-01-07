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
class FEDataMap;
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
