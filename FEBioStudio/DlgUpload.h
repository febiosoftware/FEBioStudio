/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include "LaunchConfig.h"

namespace Ui {
	class CDlgUpload;
}

class QStringList;
class CPublicationWidget;
class CLocalDatabaseHandler;
class CRepoConnectionHandler;
class FEBioStudioProject;
class CExportProjectWidget;

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
//    	setStyleSheet("QLabel { color : black; }");
//    }
//
//};
//
//class TagLabel : public QFrame
//{
//	Q_OBJECT
//
//public:
//	QLabel* label;
//	ClickableLabel* remove;
//
//	TagLabel(QString text, QWidget* parent = NULL);
//public slots:
//	void deleteThis();
//
//
//};

class CDlgUpload : public QDialog
{
	Q_OBJECT

public:
	CDlgUpload(QWidget* parent, int uploadPermissions, CLocalDatabaseHandler* dbHandler, CRepoConnectionHandler* repoHandler, FEBioStudioProject* project = nullptr);

	void setName(QString name);
	void setDescription(QString desc);
	void setCategories(QStringList& categories);
	void setOwner(QString owner);
	void setVersion(QString version);
	void setTags(QStringList& tags);
	void setPublications(const std::vector<CPublicationWidget*>& pub);

	void setTagList(QStringList& tags);

	QString getName();
	QString getDescription();
	QString getCategory();
	QString getOwner();
	QString getVersion();
	QStringList getTags();
	QList<QVariant> getPublicationInfo();

	CExportProjectWidget* exportProjectWidget();

	void accept() override;


public slots:
	void on_addTagBtn_clicked();
	void on_delTagBtn_clicked();

private:
	Ui::CDlgUpload*	ui;
	CLocalDatabaseHandler* dbHandler;
	CRepoConnectionHandler* repoHandler;
	void UpdateLaunchConfigBox(int index);
};
