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
	QLineEdit*	configFile;

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

		configFile = new QLineEdit;
		configFile->setWhatsThis("Specify the location of an FEBio configuration file. (Optional)");
		QToolButton* selectConfigFile = new QToolButton;
		selectConfigFile->setIcon(QIcon(":/icons/open.png"));
		QHBoxLayout* configLayout = new QHBoxLayout;
		configLayout->addWidget(configFile);
		configLayout->addWidget(selectConfigFile);

		QFormLayout* form = new QFormLayout;
		form->setLabelAlignment(Qt::AlignRight);
		form->addRow("Job name:", jobName);
		form->addRow("FEBio version:", launchConfig);
		form->addRow("Working directory:", cwdLayout);
		form->addRow("FEBio file format:", febioFile);
		form->addRow("Write notes:", writeNotes = new QCheckBox);
		form->addRow("Debug mode:", debug = new QCheckBox(""));
		form->addRow("Config file:", configLayout);

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
		QObject::connect(selectConfigFile, SIGNAL(clicked()), dlg, SLOT(on_selectConfigFile()));
		QObject::connect(configFile, SIGNAL(textChanged(const QString&)), dlg, SLOT(updateDefaultCommand()));
	}
};

void CDlgRun::on_selectConfigFile()
{
	QString s = QFileDialog::getOpenFileName(this, "Open");
	if (s.isEmpty() == false)
	{ 
#ifdef WIN32
		s.replace('/', '\\');
#endif
		ui->configFile->setText(s);
		updateDefaultCommand();
	}
}

void CDlgRun::updateDefaultCommand()
{
	bool bg = ui->debug->isChecked();

	if (ui->cmd->isEnabled() == false)
	{
		QString t("-i $(Filename)");
		if (bg) t += " -g";

		QString configFile = ui->configFile->text();
		if (configFile.isEmpty() == false) t += " -config " + configFile;
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
		dlg.SetLaunchConfig(ui->m_launch_configs->at(ui->m_last_index));
	}

	if (dlg.exec())
	{
		// Get the new launchConfig from the dialog
		CLaunchConfig launchConfig = dlg.GetLaunchConfig();
		QString name = QString::fromStdString(launchConfig.name);

		// If we're editing
		if(edit)
		{
			ui->m_launch_configs->at(ui->m_last_index) = launchConfig;

			// change the config's name at m_last_index and set to the combobox to m_last_index
			ui->launchConfig->setItemText(ui->m_last_index, name);
			ui->launchConfig->setCurrentIndex(ui->m_last_index);
		}
		// Otherwise create a new config
		else
		{
			ui->m_launch_configs->push_back(launchConfig);

			// Add a new config to the combobox
			ui->launchConfig->blockSignals(true);
			ui->launchConfig->insertItem(N - 3, name);
			ui->launchConfig->blockSignals(false);
			ui->launchConfig->setCurrentIndex(N - 3);
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
