/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2025 University of Utah, The Trustees of Columbia University in
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

#include <QBoxLayout>
#include <QAction>
#include <QToolButton>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>
#include <QCompleter>
#include "TagWidget.h"
#include "IconProvider.h"

class Ui::TagWidget
{
public:
    QLabel* tagLabel;
    QLineEdit* newTag;
	QListWidget* tags;
    QToolButton* addTagBtn;
    QToolButton* delTagBtn;

public:
    void SetupUi(::TagWidget* parent)
    {
        QHBoxLayout* layout = new QHBoxLayout;
        layout->setContentsMargins(0,0,0,0);

		QVBoxLayout* tagsV1 = new QVBoxLayout;
		QVBoxLayout* tagsV2 = new QVBoxLayout;
		tagsV2->setAlignment(Qt::AlignTop);

		QHBoxLayout* tagsH1 = new QHBoxLayout;
		tagsH1->addWidget(tagLabel = new QLabel("Tags: "));
		tagsH1->addWidget(newTag = new QLineEdit);
		tagsV1->addLayout(tagsH1);

		tags = new QListWidget;
		tags->setSelectionMode(QAbstractItemView::ExtendedSelection);
		tagsV1->addWidget(tags);

		QAction* addTag = new QAction;
		addTag->setIcon(CIconProvider::GetIcon("selectAdd"));
		addTagBtn = new QToolButton;
		addTagBtn->setDefaultAction(addTag);
		tagsV2->addWidget(addTagBtn);

		QAction* delTag= new QAction;
		delTag->setIcon(CIconProvider::GetIcon("selectSub"));
		delTagBtn = new QToolButton;
		delTagBtn->setDefaultAction(delTag);
		tagsV2->addWidget(delTagBtn);

		layout->addLayout(tagsV1);
		layout->addLayout(tagsV2);

        parent->setLayout(layout);
    }

};

TagWidget::TagWidget() : ui(new Ui::TagWidget)
{
    ui->SetupUi(this);

    connect(ui->addTagBtn, &QToolButton::clicked, this, &TagWidget::on_addTagBtn_clicked);
    connect(ui->delTagBtn, &QToolButton::clicked, this, &TagWidget::on_delTagBtn_clicked);
}

size_t TagWidget::Count()
{
    return ui->tags->count();
}

void TagWidget::Clear()
{
    ui->tags->clear();
    ui->newTag->clear();
}

QStringList TagWidget::GetTags()
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

void TagWidget::SetTags(QStringList& tags)
{
	for(auto tag : tags)
	{
		ui->tags->addItem(tag);
	}
}

void TagWidget::SetTagCompleter(QStringList& tags)
{
	if(ui->newTag->completer()) delete ui->newTag->completer();

	ui->newTag->setCompleter(new QCompleter(tags));
	ui->newTag->completer()->setCaseSensitivity(Qt::CaseInsensitive);
}

void TagWidget::on_addTagBtn_clicked()
{
    if(!ui->newTag->text().isEmpty())
	{
        QString tag = ui->newTag->text();
		ui->tags->addItem(tag);
        emit TagAdded(tag);
	}
	ui->newTag->clear();
}

void TagWidget::on_delTagBtn_clicked()
{
    QList<QListWidgetItem*> items = ui->tags->selectedItems();

	for(QListWidgetItem* item : items)
	{
        QString tag = item->text();
		delete item;
        emit TagDeleted(tag);
	}
}