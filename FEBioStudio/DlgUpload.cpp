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

#include "stdafx.h"
#include <QWidget>
#include <QMessageBox>
#include <QFrame>
#include <QAction>
#include <QLineEdit>
#include <QComboBox>
#include <QCompleter>
#include <QPlainTextEdit>
#include <QFormLayout>
#include <QBoxLayout>
#include <QListWidget>
#include <QToolButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFrame>
#include <QFileInfo>
#include <QLocale>
#include "DlgUpload.h"
#include "PublicationWidgetView.h"
#include "ExportProjectWidget.h"
#include "LocalDatabaseHandler.h"
#include "RepoConnectionHandler.h"

//ClickableLabel::ClickableLabel(QWidget* parent, Qt::WindowFlags f)
//    : QLabel(parent) {
//
//}
//
//ClickableLabel::~ClickableLabel() {}
//
//void ClickableLabel::mousePressEvent(QMouseEvent* event) {
//    emit clicked();
//}
//
//
//TagLabel::TagLabel(QString text, QWidget* parent)
//	: QFrame(parent)
//{
//	QHBoxLayout* layout = new QHBoxLayout;
//	layout->setContentsMargins(3, 0, 3, 0);
//	layout->setAlignment(Qt::AlignLeft);
//
//	layout->addWidget(label = new QLabel(text));
//
//	remove = new ClickableLabel;
//	remove->setText("x");
//
//	layout->addWidget(remove);
//	layout->setSizeConstraint(QLayout::SetFixedSize);
//
//	setLayout(layout);
//	setFrameStyle(QFrame::Box);
//
////
////	setStyleSheet("background-color : white; border: black;");
//
//
//	QObject::connect(remove, SIGNAL(clicked()), this, SLOT(deleteThis()));
//}
//
//void TagLabel::deleteThis()
//{
//	delete this;
//}

class Ui::CDlgUpload
{
public:
	QLineEdit* name;
	QPlainTextEdit* description;
	QLabel* owner;
	QLabel* version;
	
	QLabel* categoryLabel;
	QComboBox* categoryBox;

	QLineEdit* newTag;
	QCompleter* completer;
	QListWidget* tags;
	::CPublicationWidgetView* pubs;
	::CExportProjectWidget* files;

public:
	void setup(QDialog* dlg, int uploadPermissions, FEBioStudioProject* project)
	{
		QHBoxLayout* outerLayout = new QHBoxLayout;

		QFormLayout* form = new QFormLayout;
		form->addRow("Name: ", name = new QLineEdit);
		form->addRow("Description: ", description = new QPlainTextEdit);

		if(uploadPermissions == 1)
		{
			form->addRow("Category: ", categoryLabel = new QLabel);
			categoryBox = NULL;
		}
		else
		{
			form->addRow("Category: ", categoryBox = new QComboBox);
			categoryLabel = NULL;
		}

		form->addRow("Owner: ", owner = new QLabel);
		form->addRow("Version: ", version = new QLabel);

		QHBoxLayout* tagLayout = new QHBoxLayout;
		QVBoxLayout* v1 = new QVBoxLayout;
		QVBoxLayout* v2 = new QVBoxLayout;
		v2->setAlignment(Qt::AlignTop);

		newTag = new QLineEdit;
		completer = new QCompleter;
		completer->setCaseSensitivity(Qt::CaseInsensitive);
		newTag->setCompleter(completer);
		v1->addWidget(newTag);

		tags = new QListWidget;
		tags->setSelectionMode(QAbstractItemView::ExtendedSelection);
		v1->addWidget(tags);

		QAction* addTag = new QAction;
		addTag->setIcon(QIcon(":/icons/selectAdd.png"));
		QToolButton* addTagBtn = new QToolButton;
		addTagBtn->setDefaultAction(addTag);
		addTagBtn->setObjectName("addTagBtn");
		v2->addWidget(addTagBtn);

		QAction* delTag= new QAction;
		delTag->setIcon(QIcon(":/icons/selectSub.png"));
		QToolButton* delTagBtn = new QToolButton;
		delTagBtn->setDefaultAction(delTag);
		delTagBtn->setObjectName("delTagBtn");
		v2->addWidget(delTagBtn);

		tagLayout->addLayout(v1);
		tagLayout->addLayout(v2);

		QVBoxLayout* layout = new QVBoxLayout;

		layout->addLayout(form);
		layout->addWidget(new QLabel("Tags:"));
		layout->addLayout(tagLayout);

		layout->addWidget(new QLabel("Publications:"));

		pubs = new ::CPublicationWidgetView(::CPublicationWidgetView::EDITABLE);
		layout->addWidget(pubs);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		layout->addWidget(bb);

		outerLayout->addLayout(layout);

		if(project)
		{
			files = new ::CExportProjectWidget(project, true);
			outerLayout->addWidget(files);
		}

		dlg->setLayout(outerLayout);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));

	}
};

CDlgUpload::CDlgUpload(QWidget* parent, int uploadPermissions, CLocalDatabaseHandler* dbHandler, CRepoConnectionHandler* repoHandler, FEBioStudioProject* project)
	: QDialog(parent), ui(new Ui::CDlgUpload), dbHandler(dbHandler), repoHandler(repoHandler)
{
	ui->setup(this, uploadPermissions, project);
	setWindowTitle("Upload Project");

	QMetaObject::connectSlotsByName(this);
}

void CDlgUpload::setName(QString name)
{
	ui->name->setText(name);
}

void CDlgUpload::setDescription(QString desc)
{
	ui->description->document()->setPlainText(desc);
}

void CDlgUpload::setCategories(QStringList& categories)
{
	if(ui->categoryLabel)
	{
		ui->categoryLabel->setText(categories[0]);
	}
	else
	{
		ui->categoryBox->addItems(categories);
	}
}

void CDlgUpload::setOwner(QString owner)
{
	ui->owner->setText(owner);
}

void CDlgUpload::setVersion(QString version)
{
	ui->version->setText(version);
}

void CDlgUpload::setTags(QStringList& tags)
{
	for(auto tag : tags)
	{
		ui->tags->addItem(tag);
	}
}

void CDlgUpload::setPublications(const std::vector<CPublicationWidget*>& pubs)
{
	for(auto pub : pubs)
	{
		ui->pubs->addPublicationCopy(*pub);
	}
}

void CDlgUpload::setTagList(QStringList& tags)
{
	delete ui->completer;
	ui->completer = new QCompleter(tags);
	ui->completer->setCaseSensitivity(Qt::CaseInsensitive);

	ui->newTag->setCompleter(ui->completer);
}

QString CDlgUpload::getName()
{
	return ui->name->text();
}

QString CDlgUpload::getDescription()
{
	return ui->description->document()->toPlainText();
}

QString CDlgUpload::getCategory()
{
	if(ui->categoryLabel)
	{
		return ui->categoryLabel->text();
	}
	else
	{
		return ui->categoryBox->currentText();
	}
}

QString CDlgUpload::getOwner()
{
	return ui->owner->text();
}

QString CDlgUpload::getVersion()
{
	return ui->version->text();
}


QStringList CDlgUpload::getTags()
{
	QStringList tagList;

	for(int tag = 0; tag < ui->tags->count(); ++tag)
	{
		QString tagText = ui->tags->item(tag)->text().trimmed();

		if(!tagText.isEmpty())
		{
			if(tagList.filter(tagText, Qt::CaseInsensitive).count() == 0)
			{
				tagList.append(tagText);
			}
		}
	}

	return tagList;
}

QList<QVariant> CDlgUpload::getPublicationInfo()
{
	return ui->pubs->getPublicationInfo();
}

CExportProjectWidget* CDlgUpload::exportProjectWidget()
{
	return ui->files;
}

void CDlgUpload::accept()
{
	if(ui->name->text().isEmpty())
	{
		QMessageBox::critical(this, "Upload", "Please enter a name for your project.");
		return;
	}

	QString username = getOwner();
	QString name = getName();
	QString category = getCategory();
	if(!dbHandler->isValidUpload(username, name, category))
	{
		QMessageBox::critical(this, "Upload", "You already have a project with that name in this category."
				"\n\nPlease choose a different project name.");
		return;
	}

	if(ui->description->toPlainText().isEmpty())
	{
		QMessageBox::critical(this, "Upload", "Please enter a description for your project.");
		return;
	}

	if(ui->tags->count() == 0)
	{
		QMessageBox::critical(this, "Upload", "Please add at least one tag to your project.");
		return;
	}

	QStringList filePaths = ui->files->GetFilePaths();
	if(filePaths.isEmpty())
	{
		QMessageBox::critical(this, "Upload", "Please select at least one file to upload.");
		return;
	}

	qint64 totalSize = 0;
	for(auto path : filePaths)
	{
		QFileInfo info(path);
		totalSize += info.size();
	}

	qint64 currentProjectsSize = dbHandler->currentProjectsSize(repoHandler->getUsername());
	qint64 sizeLimit = repoHandler->getSizeLimit();

	if(totalSize + currentProjectsSize > sizeLimit)
	{
		QLocale locale = this->locale();

		QString message = QString("This upload would exceed your limit of %1 on the repository. Please remove some files "
				"or delete some projects from the repository.\n\n"
				"Current Project Size: %2\n"
				"Total on Repository: %3\n").arg(locale.formattedDataSize(sizeLimit))
				.arg(locale.formattedDataSize(totalSize))
				.arg(locale.formattedDataSize(currentProjectsSize));

		QMessageBox::critical(this, "Upload", message);
		return;
	}

	QStringList descriptions = ui->files->GetFileDescriptions();
	for(auto desc : descriptions)
	{
		if(desc.isEmpty())
		{
			QMessageBox::StandardButton reply = QMessageBox::question(this, "Upload", "Some of your files are missing descriptions."
					"\n\nWould you like to upload without them?");

			if(reply == QMessageBox::Yes)
			{
				break;
			}
			else
			{
				return;
			}
		}
	}

	if(ui->pubs->count() == 0)
	{
		QMessageBox::StandardButton reply = QMessageBox::question(this, "Upload", "You have not associated any publications with your project."
								"\n\nWould you like to upload anyway?");

		if(reply != QMessageBox::Yes)
		{
			return;
		}
	}

	QDialog::accept();
}

void CDlgUpload::on_addTagBtn_clicked()
{
	if(!ui->newTag->text().isEmpty())
	{
		ui->tags->addItem(ui->newTag->text());
	}
	ui->newTag->clear();

}

void CDlgUpload::on_delTagBtn_clicked()
{
	QList<QListWidgetItem*> items = ui->tags->selectedItems();

	for(QListWidgetItem* item : items)
	{
		delete item;
	}
}









