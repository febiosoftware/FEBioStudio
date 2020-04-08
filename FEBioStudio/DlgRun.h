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

	void SetWorkingDirectory(const QString& wd);
	void SetJobName(const QString& fn);
	void SetJobNames(QStringList& jobNames);
	void SetLaunchConfig(std::vector<CLaunchConfig>& launchConfigs, int ndefault = 0);
	void SetFEBioFileVersion(int fileVersion);

	QString GetWorkingDirectory();
	QString GetJobName();
	QString GetConfigFileName();
	int GetLaunchConfig();
	int GetFEBioFileVersion();
	bool WriteNotes();
	bool UseSubDir();
	void SetConfigFileName(const QString& configFile);

	QString CommandLine();

	void accept() override;

protected slots:
	void updateDefaultCommand();
	void on_setCWDBtn_Clicked();
	void on_editLCBtn_Clicked();
	void on_selectConfigFile();

private:
	Ui::CDlgRun*	ui;
	void UpdateLaunchConfigBox(int index);
};
