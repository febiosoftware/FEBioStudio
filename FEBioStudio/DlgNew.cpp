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

class Ui::CDlgNew
{
public:
	::CMainWindow*	m_wnd;

	int		m_ntemplate;
	QListWidget*	list;

public:
	void setup(::CMainWindow* wnd, QDialog* dlg)
	{
		m_wnd = wnd;

		m_ntemplate = 0;

		list = new QListWidget;
		QStackedWidget* s = new QStackedWidget;

		int ntemp = TemplateManager::Templates();
		for (int i = 0; i<ntemp; ++i)
		{
			const DocTemplate& doc = TemplateManager::GetTemplate(i);
			QLabel* label = new QLabel;
			label->setWordWrap(true);
			label->setText(QString("<h3>%1</h3><p>%2</p>").arg(doc.title.c_str()).arg(doc.description.c_str()));
			label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
			list->addItem(doc.title.c_str());
			s->addWidget(label);
		}

		list->setCurrentRow(0);

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(list);
		h->addWidget(s);

		QVBoxLayout* v = new QVBoxLayout;
		v->addLayout(h);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		v->addWidget(bb);

		dlg->setLayout(v);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(list, SIGNAL(currentRowChanged(int)), s, SLOT(setCurrentIndex(int)));
		QObject::connect(list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), dlg, SLOT(accept()));
	}
};

CDlgNew::CDlgNew(CMainWindow* parent ) : QDialog(parent), ui(new Ui::CDlgNew)
{
	setWindowTitle("New Model");
	ui->setup(parent, this);
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
	ui->m_ntemplate = ui->list->currentRow();
	QDialog::accept();
}

int CDlgNew::getTemplate()
{
	return ui->m_ntemplate;
}
