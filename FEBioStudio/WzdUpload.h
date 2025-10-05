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
#include <QStringList>
#include <QWizard>
#include <QTreeWidget>
#include <vector>

namespace Ui {
	class CWzdUpload;
}

//class QStringList;
class QTreeWidgetItem;
class CPublicationWidget;
class CModelDatabaseHandler;
class CModelRepoConnectionHandler;
class FEBioStudioProject;
//class CExportProjectWidget;

//class ClickableLabel : public QLabel {
//    Q_OBJECT
//
//public:
//    explicit ClickableLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
//    ~ClickableLabel();
//
//signals:
//    void clicked();
//
//protected:
//    void mousePressEvent(QMouseEvent* event);
//
//    void enterEvent(QEvent *ev) override
//    {
////    	setCursor(Qt::PointingHandCursor);
//    	setStyleSheet("QLabel { color : red; }");
//    }
//
//    void leaveEvent(QEvent *ev) override
//    {
////    	setCursor(Qt::ArrowCursor);
////    	setStyleSheet("QLabel { color : black; }");
//    	setStyleSheet("");
//    }
//
//};

//class TagLabel2 : public QFrame
//{
//	Q_OBJECT
//
//public:
//	QLabel* label;
//	ClickableLabel* remove;
//
//	TagLabel2(QString text, QWidget* parent = NULL);
//public slots:
//	void deleteThis();
//
//
//};

class CWzdUpload : public QWizard
{
	Q_OBJECT

public:
	CWzdUpload(QWidget* parent, int uploadPermissions, CModelDatabaseHandler* dbHandler, CModelRepoConnectionHandler* repoHandler, int modify = 0); //, FEBioStudioProject* project = nullptr);

	void setName(QString name);
	void setDescription(QString desc);
	void setCategories(QStringList& categories);
	void setCategory(QString category);
	void setOwner(QString owner);
	void setTags(QStringList& tags);
	void setPublications(const std::vector<CPublicationWidget*>& pub);

	void setFileInfo(QList<QList<QVariant>>& fileinfo);

	void setTagCompleter(QStringList& tags);

	QString getName();
	QString getDescription();
	QString getCategory();
	QString getOwner();
	QStringList getTags();
	QList<QVariant> getPublicationInfo();

	QStringList GetFilePaths();
	QStringList GetZipFilePaths();
	QList<QVariant> getFileInfo();

	void getProjectJson(QByteArray* json);
	void setProjectJson(QByteArray* json);

	void accept() override;
	void reject() override;

protected:
	void keyPressEvent(QKeyEvent* e) override;

public slots:
	void on_loadJson_triggered();
	void on_saveJson_triggered();
	void on_addFolder_triggered();
	void on_addFiles_triggered();
	void on_rename_triggered();
	void on_replaceFile_triggered();
	void on_fileTree_currentItemChanged(QTreeWidgetItem *current);
	void on_fileTree_itemChanged(QTreeWidgetItem *item, int column);
	void on_fileTree_itemDoubleClicked(QTreeWidgetItem * item, int column);
	void on_fileTree_itemClicked(QTreeWidgetItem * item, int column);
	void fileDescriptionChanged();

    void on_fileTags_TagAdded(QString& tag);
    void on_fileTags_TagDeleted(QString& tag);

private:
	QTreeWidgetItem* addFileFromJson(QJsonObject& file);
	void addFileToJson(QTreeWidgetItem* item, QVariantList& list);

private:
	Ui::CWzdUpload*	ui;
	CModelDatabaseHandler* dbHandler;
	CModelRepoConnectionHandler* repoHandler;
};
