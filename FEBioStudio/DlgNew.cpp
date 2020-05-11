#include "stdafx.h"
#include "DlgNew.h"
#include <QListWidget>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QStackedWidget>
#include <QTabWidget>
#include <QLineEdit>
#include <QFormLayout>
#include <QToolButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QCheckBox>
#include <QGuiApplication>
#include <QPushButton>
#include <QScreen>
#include <QDesktopWidget>
#include "DocTemplate.h"
#include "MainWindow.h"
#include "ModelDocument.h"

class Ui::CDlgNew
{
public:
	::CMainWindow*	m_wnd;

	QListWidget*	m_list;
	QLineEdit*		m_modelName;
	QCheckBox*		m_showDialog;

public:
	void setup(::CMainWindow* wnd, QDialog* dlg)
	{
		m_wnd = wnd;

		m_list = new QListWidget;
		QStackedWidget* s = new QStackedWidget;

		int ntemp = TemplateManager::Templates();
		for (int i = 0; i<ntemp; ++i)
		{
			const DocTemplate& doc = TemplateManager::GetTemplate(i);
			QLabel* label = new QLabel;
			label->setWordWrap(true);
			label->setText(QString("<h3>%1</h3><p>%2</p>").arg(doc.title.c_str()).arg(doc.description.c_str()));
			label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
			m_list->addItem(doc.title.c_str());
			s->addWidget(label);
		}

		m_list->setCurrentRow(0);

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(m_list);
		h->addWidget(s);

		QVBoxLayout* v = new QVBoxLayout;
		v->addLayout(h);

		QFormLayout* f = new QFormLayout;

		QToolButton* tb = new QToolButton;
		tb->setObjectName("folder");
		tb->setIcon(QIcon(":/icons/folder.png"));

		f->addRow("Model name:", m_modelName = new QLineEdit);
		m_modelName->setText("MyModel");
		
		v->addLayout(f);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QHBoxLayout* hb = new  QHBoxLayout;
		hb->setMargin(0);
		m_showDialog = new QCheckBox("Don't show this dialog box again");
		hb->addWidget(m_showDialog);
		hb->addWidget(bb);
		v->addLayout(hb);

		dlg->setLayout(v);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(m_list, SIGNAL(currentRowChanged(int)), s, SLOT(setCurrentIndex(int)));
		QObject::connect(m_list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), dlg, SLOT(accept()));
		QObject::connect(tb, SIGNAL(clicked(bool)), dlg, SLOT(OnFolderName()));

		m_list->setWhatsThis("Select the model template. This will adjust the UI to show only relevant features.");
		m_modelName->setWhatsThis("This is the model's name and the base of the model's filename.");
		tb->setWhatsThis("Change the model folder.");
	}
};

CDlgNew::CDlgNew(CMainWindow* parent ) : QDialog(parent), ui(new Ui::CDlgNew)
{
	setWindowTitle("New Model");
	ui->setup(parent, this);
}

void CDlgNew::setShowDialogOption(bool b)
{
	ui->m_showDialog->setChecked(b);
}

bool CDlgNew::showDialogOption()
{
	return ui->m_showDialog->isChecked();
}

void CDlgNew::SetModelName(const QString& name)
{
	ui->m_modelName->setText(name);
}

QString CDlgNew::GetModelName()
{
	return ui->m_modelName->text();
}

void CDlgNew::showEvent(QShowEvent* ev)
{
	QList<QScreen*> screenList = QGuiApplication::screens();
	QRect screenGeometry = screenList.at(0)->geometry();
	int x = (screenGeometry.width() - width()) / 2;
	int y = (screenGeometry.height() - height()) / 2;
	move(x, y);
}

void CDlgNew::accept()
{
	int ntemplate = ui->m_list->currentRow();
	QString modelName   = ui->m_modelName->text();

	if (ntemplate < 0)
	{
		QMessageBox::critical(this, "New Model", "Please choose a model template.");
		return;
	}

	// check the model's name
	if (modelName.isEmpty())
	{
		QMessageBox::critical(this, "New Model", "Please enter new name for the model.");
		return;
	}

	QDialog::accept();
}

int CDlgNew::getTemplate()
{
	return ui->m_list->currentRow();
}
