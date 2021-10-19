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

#include <vector>
#include <QWidget>
#include <QVBoxLayout>
#include <QString>
#include <QStringList>
#include <QLabel>
#include <QToolButton>
#include <QCheckBox>
#include <QAction>
#include "PublicationWidget.h"
#include "WrapLabel.h"
#include "IconProvider.h"

#include <iostream>



class Ui::CPublicationWidget
{
public:
	QWidget* shortWidget;
	QAction* expand;
	QLabel* shortLabel;
	QCheckBox* shortCheckBox;

	QWidget* fullWidget;
	QAction* shrink;
	WrapLabel* fullLabel;
	QLabel* DOILabel;
	QCheckBox* fullCheckBox;

	QAction* select;

public:

	void setup(::CPublicationWidget* parent)
	{
		QVBoxLayout* layout = new QVBoxLayout;
		layout->setContentsMargins(3, 0, 3, 0);
		layout->setSizeConstraint(QLayout::SetMinimumSize);

		QAction* select = new QAction(parent);
		select->setIcon(CIconProvider::GetIcon("check"));
		select->setObjectName("select");

		if(parent->isExpandable())
		{
			shortWidget = new QWidget;
			QHBoxLayout* shortLayout = new QHBoxLayout;

			shortLabel = new QLabel(parent->ShortText());
			shortLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
			shortLayout->addWidget(shortLabel);

			expand = new QAction(parent);
			expand->setIcon(CIconProvider::GetIcon("expand"));
			expand->setObjectName("expand");
			QToolButton* expandBtn = new QToolButton;
			expandBtn->setDefaultAction(expand);
			shortLayout->addWidget(expandBtn);

			if(parent->getSelection() == ::CPublicationWidget::CHECKBOX)
			{
				shortCheckBox = new QCheckBox(parent);
				shortCheckBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

				QObject::connect(shortCheckBox, &QCheckBox::stateChanged, parent, &::CPublicationWidget::checkBox_stateChanged);

				shortLayout->addWidget(shortCheckBox);
			}
			else if(parent->getSelection() == ::CPublicationWidget::BUTTON)
			{
				QToolButton* shortSelectBtn = new QToolButton;
				shortSelectBtn->setDefaultAction(select);

				shortLayout->addWidget(shortSelectBtn);
			}

			shortWidget->setLayout(shortLayout);
			layout->addWidget(shortWidget);
		}

		fullWidget = new QWidget;
		if(parent->isExpandable())
		{
			fullWidget->setHidden(true);
		}
		QVBoxLayout* fullLayout = new QVBoxLayout;
		fullLayout->setSpacing(0);
		QHBoxLayout* hlayout = new QHBoxLayout;

		fullLabel = new WrapLabel(parent->FullText());

		hlayout->addWidget(fullLabel);

		QVBoxLayout* fullVBLayout = new QVBoxLayout;
		fullVBLayout->setAlignment(Qt::AlignTop);
		hlayout->addLayout(fullVBLayout);

		if(parent->isExpandable())
		{
			shrink = new QAction(parent);
			shrink->setIcon(CIconProvider::GetIcon("collapse"));
			shrink->setObjectName("shrink");
			QToolButton* shrinkBtn = new QToolButton;
			shrinkBtn->setDefaultAction(shrink);

			fullVBLayout->addWidget(shrinkBtn);
		}

		if(parent->getSelection() == ::CPublicationWidget::CHECKBOX)
		{
			fullCheckBox = new QCheckBox(parent);
			fullCheckBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

			if(parent->isExpandable())
			{
				QObject::connect(fullCheckBox, &QCheckBox::stateChanged, parent, &::CPublicationWidget::checkBox_stateChanged);
			}

			fullVBLayout->addWidget(fullCheckBox);

		}
		else if(parent->getSelection() == ::CPublicationWidget::BUTTON)
		{
			QToolButton* fullSelectBtn = new QToolButton;
			fullSelectBtn->setDefaultAction(select);

			fullVBLayout->addWidget(fullSelectBtn);
		}

		fullLayout->addLayout(hlayout);

		DOILabel = new QLabel(QString("DOI: <a href=\"https://doi.org/%1\">%1</a>").arg(parent->getDOI()));
		DOILabel->setOpenExternalLinks(true);
		fullLayout->addWidget(DOILabel);

		fullWidget->setLayout(fullLayout);
		layout->addWidget(fullWidget);

		parent->setLayout(layout);
		parent->setFrameStyle(QFrame::Box);
		parent->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	}
};

CPublicationWidget::CPublicationWidget(Selection selection, bool expandable) : QFrame(), selection(selection), expandable(expandable), ui(new Ui::CPublicationWidget)
{

}

CPublicationWidget::CPublicationWidget(QVariantMap& data, Selection selection, bool expandable)
	: QFrame(), title(data["title"].toString()), year(data["year"].toString()), journal(data["journal"].toString()), volume(data["volume"].toString()),
	  issue(data["issue"].toString()), pages(data["pages"].toString()), DOI(data["DOI"].toString()),
	  authorGiven(data["authorGiven"].toStringList()), authorFamily(data["authorFamily"].toStringList()), selection(selection), expandable(expandable), ui(new Ui::CPublicationWidget)
{
	ui->setup(this);

	QMetaObject::connectSlotsByName(this);
}

CPublicationWidget::CPublicationWidget(QString title, QString year, QString journal, QString volume,
		QString issue, QString pages, QString DOI, QStringList authorGiven, QStringList authorFamily,
		Selection selection, bool expandable) : QFrame(), title(title), year(year), journal(journal),
			volume(volume), issue(issue), pages(pages), DOI(DOI), authorGiven(authorGiven),
			authorFamily(authorFamily), selection(selection), expandable(expandable),
			ui(new Ui::CPublicationWidget)
{
	ui->setup(this);

	QMetaObject::connectSlotsByName(this);
}

CPublicationWidget::CPublicationWidget(const CPublicationWidget& obj)
{
	title = obj.title;
	year = obj.year;
	journal = obj.journal;
	volume = obj.volume;
	issue = obj.issue;
	pages = obj.pages;
	DOI = obj.DOI;
	authorGiven = obj.authorGiven;
	authorFamily = obj.authorFamily;

	expandable = obj.expandable;
	selection = obj.selection;

	ui = new Ui::CPublicationWidget;

	ui->setup(this);

	QMetaObject::connectSlotsByName(this);
}

void CPublicationWidget::init()
{
	ui->setup(this);

	QMetaObject::connectSlotsByName(this);
}


void CPublicationWidget::on_expand_triggered()
{
	ui->fullWidget->setHidden(false);
	ui->shortWidget->setHidden(true);
}


void CPublicationWidget::on_shrink_triggered()
{
	ui->fullWidget->setHidden(true);
	ui->shortWidget->setHidden(false);
}

bool CPublicationWidget::isExpandable() const
{
	return expandable;
}

void CPublicationWidget::on_select_triggered()
{
	emit chosen_publication(this);
}

void CPublicationWidget::checkBox_stateChanged(int state)
{
	ui->shortCheckBox->blockSignals(true);
	ui->fullCheckBox->blockSignals(true);

	ui->shortCheckBox->setChecked(state);
	ui->fullCheckBox->setChecked(state);

	ui->shortCheckBox->blockSignals(false);
	ui->fullCheckBox->blockSignals(false);
}

QString CPublicationWidget::ShortText()
{
	if(authorFamily.length() == 0)
	{
		authorFamily.push_back("");
		authorGiven.push_back("");
	}

	return QString("%2 (%3). %1").arg(title).arg(authorFamily[0]).arg(year);
}

QString CPublicationWidget::FullText()
{
	QString authorString;
	for(int index = 0; index < authorGiven.count(); index++)
	{
		authorString += authorFamily.at(index) + ", ";
		authorString += authorGiven.at(index);

		if(index == authorGiven.count() - 1)
		{

		}
		else if(index == authorGiven.count() - 2)
		{
			authorString += " & ";
		}
		else
		{
			authorString += ", ";
		}
	}

	return QString("%1 (%2). %3. %4, %5(%6), %7.").arg(authorString).arg(year).arg(title).arg(journal).arg(volume).arg(issue).arg(pages);
}

const QStringList& CPublicationWidget::getAuthorFamily() const {
	return authorFamily;
}

void CPublicationWidget::setAuthorFamily(const QStringList &authorFamily) {
	this->authorFamily = authorFamily;
}

const QStringList& CPublicationWidget::getAuthorGiven() const {
	return authorGiven;
}

void CPublicationWidget::setAuthorGiven(const QStringList &authorGiven) {
	this->authorGiven = authorGiven;
}

const QString& CPublicationWidget::getDOI() const {
	return DOI;
}

void CPublicationWidget::setDOI(const QString &doi) {
	DOI = doi;
}

const QString& CPublicationWidget::getIssue() const {
	return issue;
}

void CPublicationWidget::setIssue(const QString &issue) {
	this->issue = issue;
}

const QString& CPublicationWidget::getJournal() const {
	return journal;
}

void CPublicationWidget::setJournal(const QString &journal) {
	this->journal = journal;
}

const QString& CPublicationWidget::getPages() const {
	return pages;
}

void CPublicationWidget::setPages(const QString &pages) {
	this->pages = pages;
}

const QString& CPublicationWidget::getTitle() const {
	return title;
}

void CPublicationWidget::setTitle(const QString &title) {
	this->title = title;
}

const QString& CPublicationWidget::getVolume() const {
	return volume;
}

void CPublicationWidget::setVolume(const QString &volume) {
	this->volume = volume;
}

const QString& CPublicationWidget::getYear() const {
	return year;
}

void CPublicationWidget::setYear(const QString &year) {
	this->year = year;
}

int CPublicationWidget::getSelection() const
{
	return selection;
}

bool CPublicationWidget::isChecked() const
{
	if(ui->fullCheckBox)
	{
		return ui->fullCheckBox->isChecked();
	}

	return false;
}


