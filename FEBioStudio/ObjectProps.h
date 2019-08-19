#pragma once
#include "PropertyList.h"
#include <QtCore/QString>
#include <MathLib/math3d.h>
#include <vector>

class FSObject;

class CObjectProps : public CPropertyList
{
public:
	CObjectProps(FSObject* po);

	QVariant GetPropertyValue(int i);

	void SetPropertyValue(int i, const QVariant& v);

	virtual FSObject* GetFEObject() { return m_po; }

	int Params() const { return (int) m_params.size(); }

protected:
	void BuildParamList(FSObject* po);

	void AddParameter(Param& p);
	QVariant GetPropertyValue(Param& p);
	void SetPropertyValue(Param& p, const QVariant& v);

	virtual QStringList GetEnumValues(const char* ch);

protected:
	FSObject*			m_po;
	std::vector<Param*>	m_params;
};
