#pragma once
#include <QDialog>

class CDlgNewProjectUI;
class CMainWindow;

class CDlgNewProject : public QDialog
{
	Q_OBJECT

public:
	CDlgNewProject(CMainWindow* wnd);

	void accept() override;

	void SetProjectFolder(const QString& folder);
	QString GetProjectFolder();

private slots:
	void OnSelectFolder();

private:
	CDlgNewProjectUI*	ui;
};
