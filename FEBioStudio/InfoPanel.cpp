#include "stdafx.h"
#include "InfoPanel.h"
#include <QBoxLayout>
#include <QPlainTextEdit>
#include <QLabel>
#include <FSCore/FSObject.h>

class Ui::CInfoPanel
{
public:
	FSObject*	po;
	QLabel*		name;
	QPlainTextEdit*	edit;

public:
	void setup(QWidget* w)
	{
		name = new QLabel;
		name->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(new QLabel("name:"));
		h->addWidget(name);
		h->addStretch();

		edit = new QPlainTextEdit;
		edit->setDisabled(true);
		edit->setObjectName("edit");
		edit->setFont(QFont("Courier", 12));

		QVBoxLayout* v = new QVBoxLayout;
		v->addLayout(h);
		v->addWidget(edit);
		w->setLayout(v);
		v->setMargin(0);

		QMetaObject::connectSlotsByName(w);
	}
};

CInfoPanel::CInfoPanel(QWidget* parent) : QWidget(parent), ui(new Ui::CInfoPanel)
{
	ui->setup(this);
}

void CInfoPanel::SetObject(FSObject* po)
{
	if (po == nullptr)
	{
		ui->po = po;
		ui->edit->clear();
		ui->edit->setDisabled(true);
		ui->name->setText("(no item selected)");
	}
	else
	{
		if (ui->edit->isEnabled() == false) ui->edit->setEnabled(true);
		ui->po = po;
		ui->name->setText(QString::fromStdString(po->GetName()));
		ui->edit->setPlainText(QString::fromStdString(po->GetInfo()));
	}
}

void CInfoPanel::on_edit_textChanged()
{
	if (ui->po)
	{
		QString txt = ui->edit->toPlainText();
		std::string s = txt.toStdString();
		ui->po->SetInfo(s);
	}
}
