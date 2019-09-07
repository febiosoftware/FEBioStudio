#include "stdafx.h"
#include "DlgRun.h"
#include <QAction>
#include <QFileDialog>
#include <QToolButton>
#include <QBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <FSCore/FSDir.h>
#include "DlgEditPath.h"
#include <string.h>


class Ui::CDlgRun
{
public:
	QLineEdit*	cwd;
	QLineEdit*	jobName;
	QComboBox*	launchConfig;
	QCheckBox*	debug;
	QCheckBox*	writeNotes;

	QComboBox*	febioFile;
	
	QCheckBox* editCmd;
	QLineEdit*	cmd;

	std::vector<CLaunchConfig>* m_launch_configs;

	int m_last_index = -1;

public:
	void setup(QDialog* dlg)
	{
		jobName = new QLineEdit;
		launchConfig = new QComboBox;
		
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
		form->addRow("FEBio version:", launchConfig);
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
		QObject::connect(launchConfig, SIGNAL(currentIndexChanged(int)), dlg, SLOT(onPathChanged(int)));

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

void CDlgRun::SetLaunchConfig(std::vector<CLaunchConfig>& launchConfigs, int ndefault)
{
	ui->m_launch_configs = &launchConfigs;

	// Get Launch Config names
	QStringList launchConfigNames;
	for(CLaunchConfig conf : launchConfigs)
	{
		launchConfigNames.append(QString::fromStdString(conf.name));
	}


	ui->launchConfig->addItems(launchConfigNames);
	ui->launchConfig->setCurrentIndex(ndefault);
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

int CDlgRun::GetLaunchConfig()
{
	return ui->launchConfig->currentIndex();
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
	ui->launchConfig->addItem("<Edit...>");
	ui->launchConfig->addItem("<New...>");
	ui->launchConfig->insertSeparator(ui->launchConfig->count() - 2);
}

void CDlgRun::onPathChanged(int n)
{
	if (n == -1) return;
	int N = ui->launchConfig->count();
	if ((N>3) && (n == N - 1))
	{
		// Add new config
		runEditPathDlg(false);
	}
	else if ((N > 3) && (n == N - 2))
	{
		// Edit current config
		runEditPathDlg(true);
	}
	else if (n < N - 3)
	{
		ui->launchConfig->setToolTip(QString::fromStdString(ui->m_launch_configs->at(n).path));
	}

	ui->m_last_index = ui->launchConfig->currentIndex();
}

void CDlgRun::runEditPathDlg(bool edit)
{
	int N = ui->launchConfig->count();

	CDlgEditPath dlg(this);

	if(edit)
	{
		dlg.launchType->setCurrentIndex(ui->m_launch_configs->at(ui->m_last_index).type);
		dlg.name->setText(QString::fromStdString(ui->m_launch_configs->at(ui->m_last_index).name));
		dlg.path->setText(QString::fromStdString(ui->m_launch_configs->at(ui->m_last_index).path));
		if (dlg.server) dlg.server->setText(QString::fromStdString(ui->m_launch_configs->at(ui->m_last_index).server));
		if (dlg.port) dlg.port->setValue(ui->m_launch_configs->at(ui->m_last_index).port);
		if (dlg.userName) dlg.userName->setText(QString::fromStdString(ui->m_launch_configs->at(ui->m_last_index).userName));
		if (dlg.remoteDir) dlg.remoteDir->setText(QString::fromStdString(ui->m_launch_configs->at(ui->m_last_index).remoteDir));
		if (dlg.jobName) dlg.jobName->setText(QString::fromStdString(ui->m_launch_configs->at(ui->m_last_index).jobName));
		if (dlg.walltime) dlg.walltime->setText(QString::fromStdString(ui->m_launch_configs->at(ui->m_last_index).walltime));
		if (dlg.procNum) dlg.procNum->setValue(ui->m_launch_configs->at(ui->m_last_index).procNum);
		if (dlg.ram) dlg.ram->setValue(ui->m_launch_configs->at(ui->m_last_index).ram);
	}

	if (dlg.exec())
	{
		QString path = dlg.path->text();
		if (path.isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Cannot add empty path.");
			ui->launchConfig->setCurrentIndex(ui->m_last_index);
		}
		else
		{
			QString name = dlg.name->text();
			if (name.isEmpty()) name = path;

			int configIndex;
			// If we're editing, then get the index at m_last_index
			if(edit)
			{
				configIndex = ui->m_last_index;
			}
			// Otherwise create a new config, and get its index
			else
			{
				ui->m_launch_configs->push_back(CLaunchConfig());
				configIndex = ui->m_launch_configs->size() - 1;
			}

			// Get the launch config parameters based on the config type
			int type = dlg.launchType->currentIndex();
			ui->m_launch_configs->at(configIndex).type = type;
			ui->m_launch_configs->at(configIndex).name = dlg.name->text().toStdString();
			ui->m_launch_configs->at(configIndex).path = dlg.path->text().toStdString();

			if(type >= REMOTE)
			{
				ui->m_launch_configs->at(configIndex).server = dlg.server->text().toStdString();
				ui->m_launch_configs->at(configIndex).port = dlg.port->value();
				ui->m_launch_configs->at(configIndex).userName = dlg.userName->text().toStdString();
				ui->m_launch_configs->at(configIndex).remoteDir = dlg.remoteDir->text().toStdString();
			}

			if(type == PBS || type == SLURM)
			{
				ui->m_launch_configs->at(configIndex).jobName = dlg.jobName->text().toStdString();
				ui->m_launch_configs->at(configIndex).walltime = dlg.walltime->text().toStdString();
				ui->m_launch_configs->at(configIndex).procNum = dlg.procNum->value();
				ui->m_launch_configs->at(configIndex).ram = dlg.ram->value();
			}


			// If we're editing, change the config's name at m_last_index and set to the combobox to m_last_index
			if(edit)
			{
				ui->launchConfig->setItemText(ui->m_last_index, name);

				ui->launchConfig->setCurrentIndex(ui->m_last_index);
			}
			// Otherwise, add a new config to the combobox
			else
			{

				ui->launchConfig->blockSignals(true);
				ui->launchConfig->insertItem(N - 3, name);
				ui->launchConfig->blockSignals(false);
				ui->launchConfig->setCurrentIndex(N - 3);
			}

		}
	}
	else
		ui->launchConfig->setCurrentIndex(ui->m_last_index);
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
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(this, "FEBio Studio", "The directory you specified does not exist. Create it?",
					QMessageBox::Yes|QMessageBox::No);

			if(!(reply == QMessageBox::Yes))
			{
				return;
			}

		}

	// see if the job name is defined
	if (ui->jobName->text().isEmpty())
	{
		QMessageBox::critical(this, "FEBio Studio", "You must enter valid job name.");
		return;
	}

	QDialog::accept();
}
