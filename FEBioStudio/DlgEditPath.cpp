#include "DlgEditPath.h"

class StackedWidget : public QStackedWidget
{
	QSize sizeHint() const override
	{
		return currentWidget()->sizeHint();
	}

	QSize minimumSizeHint() const override
	{
		return currentWidget()->minimumSizeHint();
	}

};

class Ui::CDlgEditPath
{
public:
	StackedWidget* stack;

	QWidget* localPage;
	QWidget* remotePage;
	QWidget* PBSPage;
	QWidget* SlurmPage;
	//	QWidget* customPage;

	QFormLayout* baseForm;
	QFormLayout* remoteForm;
	QFormLayout* PBSForm;
	QFormLayout* SlurmForm;
	//	QFormLayout* customForm;

	// Appear on every config type
	QComboBox*	launchType;
	QLineEdit*	name;
	QLineEdit*	path;

	// Remote config widgets
	QLineEdit*	remoteServer;
	QSpinBox*	remotePort;
	QLineEdit*	remoteUserName;
	QLineEdit*	remoteRemoteDir;

	// PBS config widgets
	QLineEdit*	PBSServer;
	QSpinBox*	PBSPort;
	QLineEdit*	PBSUserName;
	QLineEdit*	PBSRemoteDir;
	QLineEdit*	PBSJobName;
	QLineEdit*	PBSWalltime;
	QSpinBox*	PBSProcNum;
	QSpinBox*	PBSRam;

	// Slurm config widgets
	QLineEdit*	SlurmServer;
	QSpinBox*	SlurmPort;
	QLineEdit*	SlurmUserName;
	QLineEdit*	SlurmRemoteDir;
	QLineEdit*	SlurmJobName;
	QLineEdit*	SlurmWalltime;
	QSpinBox*	SlurmProcNum;
	QSpinBox*	SlurmRam;

	void setup(QDialog* dlg)
	{
		stack = new StackedWidget;

		baseForm = new QFormLayout;
		baseForm->setLabelAlignment(Qt::AlignRight);
		baseForm->addRow("Launch Type:", launchType = new QComboBox);
		launchType->addItem("Local");

		// Appear on every config type
		baseForm->addRow("Configuration name:", name = new QLineEdit);
		baseForm->addRow("FEBio executable:", path = new QLineEdit);

		localPage = new QWidget;
		baseForm->setContentsMargins(0,0,0,0);

		stack->addWidget(localPage);

#ifdef HAS_SSH
		launchType->addItem("Remote");
		launchType->addItem("PBS Queue");
		launchType->addItem("Slurm Queue");
		launchType->addItem("Custom Remote");

		remotePage = new QWidget;
		PBSPage = new QWidget;
		SlurmPage = new QWidget;
		//	customPage = new QWidget;

		remoteForm = new QFormLayout;
		remoteForm->setLabelAlignment(Qt::AlignRight);
		remoteForm->setContentsMargins(0,0,0,0);

		PBSForm = new QFormLayout;
		PBSForm->setLabelAlignment(Qt::AlignRight);
		PBSForm->setContentsMargins(0,0,0,0);

		SlurmForm = new QFormLayout;
		SlurmForm->setLabelAlignment(Qt::AlignRight);
		SlurmForm->setContentsMargins(0,0,0,0);

		// Remote config widgets
		remoteForm->addRow("Server:", remoteServer = new QLineEdit);
		remoteForm->addRow("Port:", remotePort = new QSpinBox);
		remotePort->setValue(22);
		remotePort->setMaximum(65535);
		remoteForm->addRow("Username:", remoteUserName = new QLineEdit);
		remoteForm->addRow("Remote Directory:", remoteRemoteDir = new QLineEdit);

		remotePage->setLayout(remoteForm);
		stack->addWidget(remotePage);


		// PBS config widgets
		PBSForm->addRow("Server:", PBSServer = new QLineEdit);
		PBSForm->addRow("Port:", PBSPort = new QSpinBox);
		PBSPort->setValue(22);
		PBSPort->setMaximum(65535);
		PBSForm->addRow("Username:", PBSUserName = new QLineEdit);
		PBSForm->addRow("Remote Directory:", PBSRemoteDir = new QLineEdit);
		PBSForm->addRow("Job Name:", PBSJobName = new QLineEdit);
		PBSJobName->setPlaceholderText("(optional)");
		PBSForm->addRow("Walltime:", PBSWalltime = new QLineEdit);
		PBSWalltime->setPlaceholderText("HH:MM:SS");
		PBSWalltime->setText("1:00:00");
		PBSForm->addRow("Processors:", PBSProcNum = new QSpinBox);
		PBSProcNum->setValue(1);
		PBSForm->addRow("Ram:", PBSRam = new QSpinBox);
		PBSRam->setMaximum(9999999);
		PBSRam->setSingleStep(1024);
		PBSRam->setValue(0);

		PBSPage->setLayout(PBSForm);
		stack->addWidget(PBSPage);


		// Slurm config widgets
		SlurmForm->addRow("Server:", SlurmServer = new QLineEdit);
		SlurmForm->addRow("Port:", SlurmPort = new QSpinBox);
		SlurmPort->setValue(22);
		SlurmPort->setMaximum(65535);
		SlurmForm->addRow("Username:", SlurmUserName = new QLineEdit);
		SlurmForm->addRow("Remote Directory:", SlurmRemoteDir = new QLineEdit);
		SlurmForm->addRow("Job Name:", SlurmJobName = new QLineEdit);
		SlurmJobName->setPlaceholderText("(optional)");
		SlurmForm->addRow("Walltime:", SlurmWalltime = new QLineEdit);
		SlurmWalltime->setPlaceholderText("HH:MM:SS");
		SlurmWalltime->setText("1:00:00");
		SlurmForm->addRow("Processors:", SlurmProcNum = new QSpinBox);
		SlurmProcNum->setValue(1);
		SlurmForm->addRow("Ram:", SlurmRam = new QSpinBox);
		SlurmRam->setMaximum(9999999);
		SlurmRam->setSingleStep(1024);
		SlurmRam->setValue(0);

		SlurmPage->setLayout(SlurmForm);
		stack->addWidget(SlurmPage);
#endif

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(baseForm);
		l->addWidget(stack);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addWidget(bb);

		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(launchType, QOverload<int>::of(&QComboBox::activated), stack, &QStackedWidget::setCurrentIndex);
	}
};





CDlgEditPath::CDlgEditPath(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgEditPath)
{
	ui->setup(this);
}

void CDlgEditPath::SetLaunchConfig(CLaunchConfig launchConfig)
{
	ui->launchType->setCurrentIndex(launchConfig.type);
	ui->name->setText(QString::fromStdString(launchConfig.name));
	ui->path->setText(QString::fromStdString(launchConfig.path));

	switch(launchConfig.type)
	{
	case REMOTE:
		ui->remoteServer->setText(QString::fromStdString(launchConfig.server));
		ui->remotePort->setValue(launchConfig.port);
		ui->remoteUserName->setText(QString::fromStdString(launchConfig.userName));
		ui->remoteRemoteDir->setText(QString::fromStdString(launchConfig.remoteDir));
		break;
	case PBS:
		ui->PBSServer->setText(QString::fromStdString(launchConfig.server));
		ui->PBSPort->setValue(launchConfig.port);
		ui->PBSUserName->setText(QString::fromStdString(launchConfig.userName));
		ui->PBSRemoteDir->setText(QString::fromStdString(launchConfig.remoteDir));
		ui->PBSJobName->setText(QString::fromStdString(launchConfig.jobName));
		ui->PBSWalltime->setText(QString::fromStdString(launchConfig.walltime));
		ui->PBSProcNum->setValue(launchConfig.procNum);
		ui->PBSRam->setValue(launchConfig.ram);
		break;
	case SLURM:
		ui->SlurmServer->setText(QString::fromStdString(launchConfig.server));
		ui->SlurmPort->setValue(launchConfig.port);
		ui->SlurmUserName->setText(QString::fromStdString(launchConfig.userName));
		ui->SlurmRemoteDir->setText(QString::fromStdString(launchConfig.remoteDir));
		ui->SlurmJobName->setText(QString::fromStdString(launchConfig.jobName));
		ui->SlurmWalltime->setText(QString::fromStdString(launchConfig.walltime));
		ui->SlurmProcNum->setValue(launchConfig.procNum);
		ui->SlurmRam->setValue(launchConfig.ram);
		break;
	}

	ui->stack->setCurrentIndex(launchConfig.type);
}

CLaunchConfig CDlgEditPath::GetLaunchConfig()
{
	CLaunchConfig launchConfig;

	int type = ui->launchType->currentIndex();
	launchConfig.type = type;
	launchConfig.name = ui->name->text().toStdString();
	launchConfig.path = ui->path->text().toStdString();

	switch(type)
	{
	case REMOTE:
		launchConfig.server = ui->remoteServer->text().toStdString();
		launchConfig.port = ui->remotePort->value();
		launchConfig.userName = ui->remoteUserName->text().toStdString();
		launchConfig.remoteDir = ui->remoteRemoteDir->text().toStdString();
		break;
	case PBS:
		launchConfig.server = ui->PBSServer->text().toStdString();
		launchConfig.port = ui->PBSPort->value();
		launchConfig.userName = ui->PBSUserName->text().toStdString();
		launchConfig.remoteDir = ui->PBSRemoteDir->text().toStdString();
		launchConfig.jobName = ui->PBSJobName->text().toStdString();
		launchConfig.walltime = ui->PBSWalltime->text().toStdString();
		launchConfig.procNum = ui->PBSProcNum->value();
		launchConfig.ram = ui->PBSRam->value();
		break;
	case SLURM:
		launchConfig.server = ui->SlurmServer->text().toStdString();
		launchConfig.port = ui->SlurmPort->value();
		launchConfig.userName = ui->SlurmUserName->text().toStdString();
		launchConfig.remoteDir = ui->SlurmRemoteDir->text().toStdString();
		launchConfig.jobName = ui->SlurmJobName->text().toStdString();
		launchConfig.walltime = ui->SlurmWalltime->text().toStdString();
		launchConfig.procNum = ui->SlurmProcNum->value();
		launchConfig.ram = ui->SlurmRam->value();
		break;
	}

	return launchConfig;
}


void CDlgEditPath::accept()
{
	int type = ui->launchType->currentIndex();

	if(ui->name->text().isEmpty())
	{
		QMessageBox::critical(this, "FEBio Studio", "Please enter a launch configuration name.");
		return;
	}

	if(ui->path->text().isEmpty())
	{
		QMessageBox::critical(this, "FEBio Studio", "Please enter a path to the FEBio executable.");
		return;
	}

	QFileInfo info(ui->path->text());

	switch(type)
	{
		// NOTE: Commenting this out since we don't need a path to run FEBio. Usually an executable name is enough. 
		// This check would prevent that.
/*	case LOCAL:
		if(!info.exists())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a valid path to the FEBio executable.");
					return;
		}
		break;
*/
	case REMOTE:
		if(ui->remoteServer->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a server address.");
			return;
		}

		if(ui->remoteUserName->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a username.");
			return;
		}

		if(ui->remoteRemoteDir->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a remote directory.");
			return;
		}
		break;

	case PBS:
		if(ui->PBSServer->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a server address.");
			return;
		}

		if(ui->PBSUserName->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a username.");
			return;
		}

		if(ui->PBSRemoteDir->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a remote directory.");
			return;
		}

		if(ui->PBSWalltime->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a walltime.");
			return;
		}
		break;

	case SLURM:
		if(ui->SlurmServer->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a server address.");
			return;
		}

		if(ui->SlurmUserName->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a username.");
			return;
		}

		if(ui->SlurmRemoteDir->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a remote directory.");
			return;
		}

		if(ui->SlurmWalltime->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a walltime.");
			return;
		}
		break;
	}


	QDialog::accept();
}














