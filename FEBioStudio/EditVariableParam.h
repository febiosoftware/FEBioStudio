#pragma once
#include <QComboBox>

class Param;

//-----------------------------------------------------------------------------
class CEditVariableParam : public QComboBox
{
	Q_OBJECT

public:
	CEditVariableParam(QWidget* parent = nullptr);

	void setParam(Param* p);

public slots:
	void onCurrentIndexChanged(int index);
	void onEditTextChanged(const QString& txt);

signals:
	void typeChanged();

private:
	Param* m_param;
};

