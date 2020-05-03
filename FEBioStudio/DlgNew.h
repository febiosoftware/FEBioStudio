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

	QString GetModelName();

	void SetModelFolder(const QString& modelPath);
	QString GetModelFolder();

private slots:
	void OnFolderName();

public:
	Ui::CDlgNew*	ui;
};
