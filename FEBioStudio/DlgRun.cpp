#include "stdafx.h"
#include "DlgRun.h"
#include <QAction>
#include <QFileDialog>
#include <QToolButton>
#include <QBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include "MainWindow.h"
#include "Document.h"

class CDlgEditPath : public QDialog
{
public:
	QLineEdit*	path;
	QLineEdit*	info;

public:
	CDlgEditPath(QWidget* parent) : QDialog(parent)
	{
		QFormLayout* form = new QFormLayout;
		form->setLabelAlignment(Qt::AlignRight);
		form->addRow("FEBio executable:", path = new QLineEdit);
		form->addRow("description:", info = new QLineEdit);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(form);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addWidget(bb);

		setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
	}
};

class Ui::CDlgRun
{
public:
	::CMainWindow*	m_wnd;

	QLineEdit*	cwd;
	QLineEdit*	jobName;
	QComboBox*	febio;
	QCheckBox*	debug;
	
	QCheckBox* editCmd;
	QLineEdit*	cmd;

	QStringList*	m_pathList;
	QStringList*	m_infoList;

	int m_last_index = -1;

public:
	void setup(QDialog* dlg)
	{
		jobName = new QLineEdit;
		febio = new QComboBox;
		
		cwd = new QLineEdit;
		QAction* setCWD = new QAction;
		setCWD->setIcon(QIcon(":/icons/open.png"));
		QToolButton* setCWDBtn = new QToolButton;
		setCWDBtn->setDefaultAction(setCWD);
		QBoxLayout* cwdLayout = new QBoxLayout(QBoxLayout::LeftToRight);
		cwdLayout->addWidget(cwd);
		cwdLayout->addWidget(setCWDBtn);
		
		QFormLayout* form = new QFormLayout;
		form->setLabelAlignment(Qt::AlignRight);
		form->addRow("FEBio version:", febio);
		form->addRow("Working directory:", cwdLayout);
		form->addRow("Job name:", jobName);
		form->addRow("Debug mode:", debug = new QCheckBox(""));

		editCmd = new QCheckBox("override command");
		cmd = new QLineEdit; cmd->setEnabled(false);
		cmd->setText("-i $(Filename)");

		QPushButton* run = new QPushButton("Run");
		QPushButton* cnl = new QPushButton("Cancel");

		QHBoxLayout* h = new QHBoxLayout;
		h->addStretch();
		h->addWidget(run);
		h->addWidget(cnl);
		
		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(form);
		l->addStretch();
		l->addWidget(editCmd);
		l->addWidget(cmd);
		l->addLayout(h);
		dlg->setLayout(l);
		
		QObject::connect(setCWDBtn, SIGNAL(clicked()), dlg, SLOT(on_setCWDBtn_Clicked()));
		QObject::connect(run, SIGNAL(clicked()), dlg, SLOT(accept()));
		QObject::connect(cnl, SIGNAL(clicked()), dlg, SLOT(reject()));
		QObject::connect(febio, SIGNAL(currentIndexChanged(int)), dlg, SLOT(onPathChanged(int)));

		QObject::connect(debug, SIGNAL(toggled(bool)), dlg, SLOT(updateDefaultCommand()));
		QObject::connect(editCmd, SIGNAL(toggled(bool)), cmd, SLOT(setEnabled(bool)));
	}
};

void CDlgRun::updateDefaultCommand()
{
	bool bg = ui->debug->isChecked();

	if (ui->cmd->isEnabled() == false)
	{
		QString t("-i $(Filename)");
		if (bg) t += " -g";
		ui->cmd->setText(t);
	}
}

void CDlgRun::on_setCWDBtn_Clicked()
{
	QFileDialog dlg(this, "Working Directory",GetWorkingDirectory());
	dlg.setFileMode(QFileDialog::Directory);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	if (dlg.exec())
	{
		dlg.setDirectory(GetWorkingDirectory());

		// get the file name
		QStringList files = dlg.selectedFiles();
		QString fileName = files.first();

		SetWorkingDirectory(fileName);
	}
}

void CDlgRun::SetWorkingDirectory(const QString& wd)
{
	QString workingDir(wd);
#ifdef WIN32
	workingDir.replace("/", "\\");
#endif
	ui->cwd->setText(workingDir);
}

void CDlgRun::SetJobName(const QString& fn)
{
	ui->jobName->setText(fn);
}

void CDlgRun::SetFEBioPath(QStringList& path, QStringList& info, int ndefault)
{
	ui->m_pathList = &path;
	ui->m_infoList = &info;

//	assert(path.count() == info.count());

	ui->febio->addItems(info);
	ui->febio->setCurrentIndex(ndefault);
	ui->m_last_index = ndefault;
}

QString CDlgRun::GetWorkingDirectory()
{
	return ui->cwd->text();
}

QString CDlgRun::GetJobName()
{
	return ui->jobName->text();
}

int CDlgRun::GetFEBioPath()
{
	return ui->febio->currentIndex();
}

QString CDlgRun::CommandLine()
{
	return ui->cmd->text();
}

CDlgRun::CDlgRun(CMainWindow* parent) : QDialog(parent), ui(new Ui::CDlgRun)
{
	ui->m_wnd = parent;
	ui->setup(this);
	ui->jobName->setFocus();
	setWindowTitle("Run FEBio");
	resize(500, 300);
}

// Call this before exec!
void CDlgRun::Init()
{
	ui->febio->addItem("<Edit...>");
	ui->febio->addItem("<New...>");
	ui->febio->insertSeparator(ui->febio->count() - 2);
}

void CDlgRun::onPathChanged(int n)
{
	if (n == -1) return;
	int N = ui->febio->count();
	if ((N>3) && (n == N - 1))
	{
		// add new path
		CDlgEditPath dlg(this);
		if (dlg.exec())
		{
			QString path = dlg.path->text();
			if (path.isEmpty())
			{
				QMessageBox::critical(this, "PreView2", "Cannot add empty path.");
				ui->febio->setCurrentIndex(ui->m_last_index);
			}
			else
			{
				QString info = dlg.info->text();
				if (info.isEmpty()) info = path;

				ui->m_pathList->append(path);
				ui->m_infoList->append(info);

				ui->febio->blockSignals(true);
				ui->febio->insertItem(N - 3, info);
				ui->febio->blockSignals(false);
				ui->febio->setCurrentIndex(N - 3);
			}
		}
		else
			ui->febio->setCurrentIndex(ui->m_last_index);
	}
	else if ((N > 3) && (n == N - 2))
	{
		// edit existing path
		CDlgEditPath dlg(this);
		dlg.path->setText(ui->m_pathList->at(ui->m_last_index));
		dlg.info->setText(ui->m_infoList->at(ui->m_last_index));
		if (dlg.exec())
		{
			QString path = dlg.path->text();
			if (path.isEmpty())
			{
				QMessageBox::critical(this, "PreView2", "Cannot set empty path");
			}
			else
			{
				QString info = dlg.info->text();
				if (info.isEmpty()) info = path;

				ui->m_pathList->replace(ui->m_last_index, path);
				ui->m_infoList->replace(ui->m_last_index, info);

				ui->febio->setItemText(ui->m_last_index, info);
			}

			ui->febio->setCurrentIndex(ui->m_last_index);
		}
		else
			ui->febio->setCurrentIndex(ui->m_last_index);
	}
	else if (n < N - 3)
	{
		ui->febio->setToolTip(ui->m_pathList->at(n));
	}

	ui->m_last_index = ui->febio->currentIndex();
}

void CDlgRun::accept()
{
	// see if the path is valid
	QString path = ui->cwd->text();
	if (path.isEmpty())
	{
		QMessageBox::critical(this, "FEBio Studio", "You must enter a valid working directory.");
		return;
	}

	CDocument* doc = ui->m_wnd->GetDocument();
	std::string projectFolder = doc->GetDocFolder();
	path.replace("$(ProjectFolder)", QString::fromStdString(projectFolder));

	QDir dir(path);
	if (path.isEmpty() || (dir.exists() == false))
	{
		QMessageBox::critical(this, "FEBio Studio", "You must enter a valid working directory.");
		return;
	}

	// see if the job name is defined
	if (ui->jobName->text().isEmpty())
	{
		QMessageBox::critical(this, "FEBio Studio", "You must enter valid job name.");
		return;
	}

	QDialog::accept();
}
