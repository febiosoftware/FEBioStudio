#pragma once
#include <QDialog>
#include <vector>
#include "LaunchConfig.h"

namespace Ui {
	class CDlgRun;
}

class CDlgRun : public QDialog
{
	Q_OBJECT

public:
	CDlgRun(QWidget* parent);

	// Call this before exec!
	void Init();

	void SetWorkingDirectory(const QString& wd);
	void SetJobName(const QString& fn);
	void SetLaunchConfig(std::vector<CLaunchConfig>& launchConfigs, int ndefault = 0);

	QString GetWorkingDirectory();
	QString GetJobName();
	int GetLaunchConfig();
	int GetFEBioFileVersion();
	bool WriteNodes();
	bool UseSubDir();

	QString CommandLine();

	void accept() override;

protected slots:
	void updateDefaultCommand();
	void on_setCWDBtn_Clicked();
	void onPathChanged(int n);
	void on_selectConfigFile();

private:
	Ui::CDlgRun*	ui;

	void runEditPathDlg(bool edit);
};
