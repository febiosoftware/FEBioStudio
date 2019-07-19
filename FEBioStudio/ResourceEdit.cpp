#include "stdafx.h"
#include "ResourceEdit.h"
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QFileDialog>

class Ui::CResourceEdit
{
public:
	QLineEdit*		m_name;
	QPushButton*	m_pick;

	QStringList		m_resFlt;

public:
	void setup(QWidget* w)
	{
		m_name = new QLineEdit;
		m_pick = new QPushButton("...");
		m_pick->setFixedWidth(20);

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(m_name);
		h->addWidget(m_pick);
		h->setSpacing(0);
		h->setMargin(0);
		w->setLayout(h);

		m_resFlt << "All files (*)";

		QObject::connect(m_name, SIGNAL(editingFinished()), w, SLOT(nameChanged()));
		QObject::connect(m_pick, SIGNAL(clicked(bool)), w, SLOT(buttonPressed()));
	}
};


//=================================================================================================

CResourceEdit::CResourceEdit(QWidget* parent) : QWidget(parent), ui(new Ui::CResourceEdit)
{
	ui->setup(this);
}

void CResourceEdit::setResourceFilter(const QStringList& flt)
{
	ui->m_resFlt = flt;
}

QString CResourceEdit::resourceName() const
{
	return ui->m_name->text();
}

void CResourceEdit::setResourceName(const QString& t)
{
	ui->m_name->setText(t);
}

void CResourceEdit::nameChanged()
{
	emit resourceChanged();
}

void CResourceEdit::buttonPressed()
{
	QFileDialog dlg(this);
	dlg.setFileMode(QFileDialog::ExistingFile);
	dlg.setNameFilters(ui->m_resFlt);
	if (dlg.exec())
	{
		QStringList files = dlg.selectedFiles();
		if (files.size() == 1)
		{
			QString fileName = files.at(0);
			ui->m_name->setText(fileName);
			emit resourceChanged();
		}
	}
}
