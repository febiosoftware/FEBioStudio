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
#include <QWidget>
#include <vector>
#include <QProcess>

class QTreeWidgetItem;
class CMainWindow;

namespace Ui {
	class CProjectViewer;
}

class CProjectViewer : public QWidget
{
	Q_OBJECT

public:
	CProjectViewer(CMainWindow* pwnd, QWidget* parent = 0);

	void Update();

	void contextMenuEvent(QContextMenuEvent* ev) override;

private:
	QTreeWidgetItem* currentItem();
	void SelectItem(int itemId);

private slots:
	void on_fileList_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
	void on_fileList_itemDoubleClicked(QTreeWidgetItem* item, int column);
	void on_info_textChanged();
	void onCreateGroup();
	void onMoveToGroup(int i);
	void onRemoveGroup();
	void onRenameGroup();
	void onShowInExplorer();
	void onRemoveFromProject();
	void onAddFile();
	void onImportFolder();
	void onBuildPlugin();
	void onLoadPlugin();
	void onAddFEBioFeature();
	// plugin process slots
	void onConfigureFinished(int, QProcess::ExitStatus);
	void onBuildFinished(int, QProcess::ExitStatus);
	void onReadyRead();
	void onErrorOccurred(QProcess::ProcessError);

private:
	Ui::CProjectViewer*	ui;
};

class CPluginProcess : public QProcess
{
public:
	CPluginProcess(QObject* parent) : QProcess(parent) {}

	virtual void run() = 0;
};

class CConfigurePluginProcess : public CPluginProcess
{
public:
	CConfigurePluginProcess(QObject* parent);
	void run() override;
};

class CBuildPluginProcess : public CPluginProcess
{
public:
	CBuildPluginProcess(QObject* parent);
	void run() override;
};
