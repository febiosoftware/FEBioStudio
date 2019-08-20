#pragma once
#include <QDialog>

namespace Ui {
	class CDlgRun;
}

class CMainWindow;

class CDlgRun : public QDialog
{
	Q_OBJECT

public:
	CDlgRun(CMainWindow* parent);

	// Call this before exec!
	void Init();

	void SetWorkingDirectory(const QString& wd);
	void SetJobName(const QString& fn);
	void SetFEBioPath(QStringList& path, QStringList& info, int ndefault = 0);

	QString GetWorkingDirectory();
	QString GetJobName();
	int GetFEBioPath();

	QString CommandLine();

	void accept() override;

protected slots:
	void updateDefaultCommand();
	void on_setCWDBtn_Clicked();
	void onPathChanged(int n);

private:
	Ui::CDlgRun*	ui;
};
