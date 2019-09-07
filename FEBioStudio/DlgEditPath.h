#include "stdafx.h"
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
#include "LaunchConfig.h"

class CDlgEditPath : public QDialog
{
	Q_OBJECT

public:
	QFormLayout* form;
	QComboBox*	launchType;
	QLineEdit*	name;
	QLineEdit*	path;
	QLineEdit*	server;
	QSpinBox*	port;
	QLineEdit*	userName;
	QLineEdit*	remoteDir;
	QLineEdit*	jobName;
	QLineEdit*	walltime;
	QSpinBox*	procNum;
	QSpinBox*	ram;

public:
	CDlgEditPath(QWidget* parent) : QDialog(parent)
	{
		form = new QFormLayout;
		form->setLabelAlignment(Qt::AlignRight);
		form->addRow("Launch Type:", launchType = new QComboBox);
		launchType->addItem("Local");
#ifdef HAS_SSH
		launchType->addItem("Remote");
		launchType->addItem("PBS Queue");
		launchType->addItem("Slurm Queue");
		launchType->addItem("Custom Remote");
#endif
		server = nullptr;
		port = nullptr;
		userName = nullptr;
		remoteDir = nullptr;
		jobName = nullptr;
		walltime = nullptr;
		procNum = nullptr;
		ram = nullptr;

		form->addRow("Configuration name:", name = new QLineEdit);
		form->addRow("FEBio executable:", path = new QLineEdit);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(form);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addWidget(bb);

		setLayout(l);

		onLaunchTypeChanged(0);

		QObject::connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
		QObject::connect(launchType, SIGNAL(currentIndexChanged(int)), this, SLOT(onLaunchTypeChanged(int)));
	}

	virtual ~CDlgEditPath() {}

private slots:
	void onLaunchTypeChanged(int n)
	{
		while(form->rowCount() > 3)
		{
			form->removeRow(form->rowCount() - 1 );
		}

		if(n >= REMOTE)
		{
			form->addRow("Server:", server = new QLineEdit);
			form->addRow("Port:", port = new QSpinBox);
			port->setValue(22);
			form->addRow("Username:", userName = new QLineEdit);
			form->addRow("Remote Directory:", remoteDir = new QLineEdit);
		}

		if(n == PBS || n == SLURM)
		{
			form->addRow("Job Name:", jobName = new QLineEdit);
			form->addRow("Walltime:", walltime = new QLineEdit);
			walltime->setPlaceholderText("HH:MM:SS");
			walltime->setText("1:00:00");
			form->addRow("Processors:", procNum = new QSpinBox);
			procNum->setValue(1);
			form->addRow("Ram:", ram = new QSpinBox);
			ram->setMaximum(9999999);
			ram->setSingleStep(1024);
			ram->setValue(0);
		}

	}
};
