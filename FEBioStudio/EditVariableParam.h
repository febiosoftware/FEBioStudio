#pragma once
#include <QComboBox>

class Param;

class CEditVariableParam : public QComboBox
{
	Q_OBJECT

public:
	CEditVariableParam(QWidget* parent = nullptr);

	void setParam(Param* p);

	QString text() const;

public slots:
	void onCurrentIndexChanged(int index);
	void onTextChanged(const QString& txt);

signals:
	void typeChanged();
	void requestClose();

private:
	Param* m_param;
};
