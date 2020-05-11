#pragma once
#include <QDialog>

namespace Ui {
	class CDlgNew;
}

class CMainWindow;

class CDlgNew : public QDialog
{
	Q_OBJECT

public:
	CDlgNew(CMainWindow* parent);

	void accept();

	int getTemplate();

	void showEvent(QShowEvent* ev);

	void SetModelName(const QString& name);
	QString GetModelName();

	void setShowDialogOption(bool b);
	bool showDialogOption();

public:
	Ui::CDlgNew*	ui;
};
