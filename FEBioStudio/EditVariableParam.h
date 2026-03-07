#pragma once
#include <QWidget>

class Param;

class UIEditVariableParam;

class CEditVariableParam : public QWidget
{
	Q_OBJECT

public:
	CEditVariableParam(QWidget* parent = nullptr);

	void setParam(Param* p);

	QString text() const;

public slots:
	void onCurrentIndexChanged(int index);
	void onTextChanged(const QString& txt);
	void onEditPressed();

signals:
	void typeChanged();
	void requestClose();

private:
	UIEditVariableParam* ui;
};

