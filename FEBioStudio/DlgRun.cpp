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
#include <FSCore/FSDir.h>

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
	QLineEdit*	cwd;
	QLineEdit*	jobName;
	QComboBox*	febioVersion;
	QCheckBox*	debug;
	QCheckBox*	writeNotes;

	QComboBox*	febioFile;
	
	QCheckBox* editCmd;
	QLineEdit*	cmd;

	QStringList*	m_pathList;
	QStringList*	m_infoList;

	int m_last_index = -1;

public:
	void setup(QDialog* dlg)
	{
		jobName = new QLineEdit;
		febioVersion = new QComboBox;
		
		cwd = new QLineEdit;
		QAction* setCWD = new QAction;
		setCWD->setIcon(QIcon(":/icons/open.png"));
		QToolButton* setCWDBtn = new QToolButton;
		setCWDBtn->setDefaultAction(setCWD);
		QBoxLayout* cwdLayout = new QBoxLayout(QBoxLayout::LeftToRight);
		cwdLayout->addWidget(cwd);
		cwdLayout->addWidget(setCWDBtn);

		febioFile = new QComboBox;
		febioFile->addItem("FEBio 2.5 format");
#ifdef _DEBUG
		febioFile->addItem("FEBio 3.0 format");
#endif
		
		QFormLayout* form = new QFormLayout;
		form->setLabelAlignment(Qt::AlignRight);
		form->addRow("FEBio version:", febioVersion);
		form->addRow("Working directory:", cwdLayout);
		form->addRow("Job name:", jobName);
		form->addRow("FEBio file format:", febioFile);
		form->addRow("Write notes:", writeNotes = new QCheckBox);
		form->addRow("Debug mode:", debug = new QCheckBox(""));

		writeNotes->setChecked(true);

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
		QObject::connect(febioVersion, SIGNAL(currentIndexChanged(int)), dlg, SLOT(onPathChanged(int)));

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

	ui->febioVersion->addItems(info);
	ui->febioVersion->setCurrentIndex(ndefault);
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
	return ui->febioVersion->currentIndex();
}

int CDlgRun::GetFEBioFileVersion()
{
	return ui->febioFile->currentIndex();
}

bool CDlgRun::WriteNodes()
{
	return ui->writeNotes->isChecked();
}

QString CDlgRun::CommandLine()
{
	return ui->cmd->text();
}

CDlgRun::CDlgRun(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgRun)
{
	ui->setup(this);
	ui->jobName->setFocus();
	setWindowTitle("Run FEBio");
	resize(500, 300);
}

// Call this before exec!
void CDlgRun::Init()
{
	ui->febioVersion->addItem("<Edit...>");
	ui->febioVersion->addItem("<New...>");
	ui->febioVersion->insertSeparator(ui->febioVersion->count() - 2);
}

void CDlgRun::onPathChanged(int n)
{
	if (n == -1) return;
	int N = ui->febioVersion->count();
	if ((N>3) && (n == N - 1))
	{
		// add new path
		CDlgEditPath dlg(this);
		if (dlg.exec())
		{
			QString path = dlg.path->text();
			if (path.isEmpty())
			{
				QMessageBox::critical(this, "FEBio Studio", "Cannot add empty path.");
				ui->febioVersion->setCurrentIndex(ui->m_last_index);
			}
			else
			{
				QString info = dlg.info->text();
				if (info.isEmpty()) info = path;

				ui->m_pathList->append(path);
				ui->m_infoList->append(info);

				ui->febioVersion->blockSignals(true);
				ui->febioVersion->insertItem(N - 3, info);
				ui->febioVersion->blockSignals(false);
				ui->febioVersion->setCurrentIndex(N - 3);
			}
		}
		else
			ui->febioVersion->setCurrentIndex(ui->m_last_index);
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
				QMessageBox::critical(this, "FEBio Studio", "Cannot set empty path");
			}
			else
			{
				QString info = dlg.info->text();
				if (info.isEmpty()) info = path;

				ui->m_pathList->replace(ui->m_last_index, path);
				ui->m_infoList->replace(ui->m_last_index, info);

				ui->febioVersion->setItemText(ui->m_last_index, info);
			}

			ui->febioVersion->setCurrentIndex(ui->m_last_index);
		}
		else
			ui->febioVersion->setCurrentIndex(ui->m_last_index);
	}
	else if (n < N - 3)
	{
		ui->febioVersion->setToolTip(ui->m_pathList->at(n));
	}

	ui->m_last_index = ui->febioVersion->currentIndex();
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

	// convert to an absolute path
	std::string spath = path.toStdString();
	FSDir dir(spath);
	spath = dir.toAbsolutePath();

	// check if it exists
	QDir qdir(QString::fromStdString(spath));
	if (path.isEmpty() || (qdir.exists() == false))
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
