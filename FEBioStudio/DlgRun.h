/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#pragma once
#include <QDialog>
#include <vector>

class CLaunchConfig;

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
	void SetLaunchConfig(std::vector<CLaunchConfig*>& launchConfigs, int ndefault = 0);
	void SetFEBioFileVersion(int fileVersion);

	void ShowFEBioSaveOptions(bool b);
	void EnableJobSettings(bool b);

	QString GetWorkingDirectory();
	QString GetJobName();
	QString GetConfigFileName();
	int GetLaunchConfig();
	int GetFEBioFileVersion();
	bool WriteNotes();
	bool AllowMixedMesh();
	bool UseSubDir();
	void SetConfigFileName(const QString& configFile);
	bool DoAutoSave();

	void ShowAdvancedSettings(bool b);
	bool AdvancedSettingsShown();

	void SetDebugFlag(bool b);
	bool HasDebugFlag();

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
