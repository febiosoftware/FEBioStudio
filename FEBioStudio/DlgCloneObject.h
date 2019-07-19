#pragma once
#include <QDialog>

namespace Ui {
	class CDlgCloneObject;
}

class CDlgCloneObject : public QDialog
{
	Q_OBJECT

public:
	CDlgCloneObject(QWidget* parent);

	void accept();

	QString GetNewObjectName();

protected:
	Ui::CDlgCloneObject*	ui;
};
