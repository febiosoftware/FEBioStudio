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
#include "DocTemplate.h"
#include "MainWindow.h"

class Ui::CDlgNew
{
public:
	int	m_nchoice;
	QListWidget*	list;
	QTabWidget*		tab;
	QListWidget*	recentFilesList;
	QLineEdit*		projectFolder;
	QLineEdit*		projectName;

public:
	void setup(::CMainWindow* wnd, QDialog* dlg)
	{
		m_nchoice = 0;

		tab = new QTabWidget;

		QWidget* newProject = new QWidget;

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

		projectFolder = new QLineEdit;

		QToolButton* tb = new QToolButton;
		tb->setIcon(QIcon(":/icons/open.png"));
		tb->setToolTip(QString("Select the project folder."));

		QHBoxLayout* selectFolder = new QHBoxLayout;
		selectFolder->addWidget(projectFolder);
		selectFolder->addWidget(tb);
		selectFolder->setMargin(0);
		selectFolder->setSpacing(1);

		QFormLayout* f = new QFormLayout;
		f->addRow("Project name:"  , projectName   = new QLineEdit);
		f->addRow("Project folder:", selectFolder);

		projectName->setText("MyProject");

		QVBoxLayout* v = new QVBoxLayout;
		v->addLayout(h);
		v->addLayout(f);

		newProject->setLayout(v);

		tab->addTab(newProject, "New Project");

		recentFilesList = new QListWidget;
		QStringList recentFiles = wnd->GetRecentFileList();
		recentFilesList->addItems(recentFiles);
		tab->addTab(recentFilesList, "Recent Project");

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addWidget(tab);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		mainLayout->addWidget(bb);

		dlg->setLayout(mainLayout);

		// Make the project name the focus
		projectName->setFocus();

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(list, SIGNAL(currentRowChanged(int)), s, SLOT(setCurrentIndex(int)));
		QObject::connect(list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), dlg, SLOT(accept()));
		QObject::connect(recentFilesList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), dlg, SLOT(accept()));
		QObject::connect(tb, SIGNAL(clicked()), dlg, SLOT(onProjectFolder()));
	}
};

CDlgNew::CDlgNew(CMainWindow* parent ) : QDialog(parent), ui(new Ui::CDlgNew)
{
	setWindowTitle("Project Manager");
	ui->setup(parent, this);
}

void CDlgNew::accept()
{
	ui->m_nchoice = ui->list->currentIndex().row();

	if (ui->projectName->text().isEmpty())
	{
		QMessageBox::critical(this, "FEBio Studio", "You must enter a project name.");
		return;
	}

	if (ui->projectFolder->text().isEmpty())
	{
		QMessageBox::critical(this, "FEBio Studio", "You must select a valid project folder.");
		return;
	}

	QDialog::accept();
}

int CDlgNew::getTemplate()
{ 
	return ui->m_nchoice;
}

bool CDlgNew::createNew()
{
	return (ui->tab->currentIndex() == 0);
}

QString CDlgNew::getProjectName()
{
	return ui->projectName->text();
}

QString CDlgNew::getProjectFolder()
{
	return ui->projectFolder->text();
}

void CDlgNew::setProjectFolder(const QString& projectFolder)
{
	ui->projectFolder->setText(projectFolder);
}

QString CDlgNew::getRecentFileName()
{
	QListWidgetItem* item = ui->recentFilesList->currentItem();
	if (item) return item->text();
	else return QString();
}

void CDlgNew::onProjectFolder()
{
	QFileDialog dlg(this, "Project Folder");
	dlg.setFileMode(QFileDialog::Directory);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);

	QString path = ui->projectFolder->text();
	if (path.isEmpty() == false) dlg.setDirectory(path);
	if (dlg.exec())
	{
		// get the file name
		QStringList files = dlg.selectedFiles();
		QString path = files.first();

		ui->projectFolder->setText(path);
	}
}
