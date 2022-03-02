/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "stdafx.h"
#include <QAbstractItemView>
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
#include <QStackedWidget>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QLabel>
#include <unordered_map>
#include "DlgEditLConfigs.h"
#include "LaunchConfig.h"
#include "DynamicStackedWidget.h"

class Ui::CDlgEditPath
{
public:
	QListWidget* launchConfigList;

	QWidget* defaultPage;

	QStackedWidget* rhsStack;
	DynamicStackedWidget* configStack;

	QToolButton* addConfigBtn;
	QToolButton* delConfigBtn;
	QToolButton* editLocalPath;

	QWidget* localPage;
	QWidget* remotePage;
	QWidget* PBSPage;
	QWidget* SlurmPage;
	QWidget* customPage;

	QComboBox*	launchType;

	// Local config widgets
	QLineEdit*	localPath;

	// Remote config widgets
	QLineEdit*	remotePath;
	QLineEdit*	remoteServer;
	QSpinBox*	remotePort;
	QLineEdit*	remoteUserName;
	QLineEdit*	remoteRemoteDir;

	// PBS config widgets
	QLineEdit*	PBSPath;
	QLineEdit*	PBSServer;
	QSpinBox*	PBSPort;
	QLineEdit*	PBSUserName;
	QLineEdit*	PBSRemoteDir;
	QPlainTextEdit* PBSText;
	QString DefaultPBSText = "#!/bin/bash\n\n"
			"#PBS -l nodes=1:ppn=1\n"
			"#PBS -l walltime=1:00:00\n"
			"#PBS -N ${JOB_NAME}\n"
			"#PBS -o ${REMOTE_DIR}/${JOB_NAME}_stdout.log\n"
			"#PBS -e ${REMOTE_DIR}/${JOB_NAME}_stderr.log\n\n"
			"${FEBIO_PATH} ${REMOTE_DIR}/${JOB_NAME}.feb";

	// Slurm config widgets
	QLineEdit*	SlurmPath;
	QLineEdit*	SlurmServer;
	QSpinBox*	SlurmPort;
	QLineEdit*	SlurmUserName;
	QLineEdit*	SlurmRemoteDir;
	QPlainTextEdit* SlurmText;
	QString DefaultSlurmText = "#!/bin/bash\n\n"
			"#SBATCH -N 1\n"
			"#SBATCH -n 1\n"
			"#SBATCH -t 1:00:00\n"
			"#SBATCH -J ${JOB_NAME}\n"
			"#SBATCH -o ${REMOTE_DIR}/${JOB_NAME}_stdout.log\n"
			"#SBATCH -e ${REMOTE_DIR}/${JOB_NAME}_stderr.log\n\n"
			"${FEBIO_PATH} ${REMOTE_DIR}/${JOB_NAME}.feb";

	// Custom config widgets
	QLineEdit*	customServer;
	QSpinBox*	customPort;
	QLineEdit*	customUserName;
	QLineEdit*	customRemoteDir;
	QPlainTextEdit* customText;
	QString DefaultCustomText = "#Use this feild to create a custom script.\n"
				"#Lines starting with '#' are comments and will be ignored.\n"
				"#Empty Lines will be ignored.\n"
				"#Each line will be run as a separate command.\n\n"
				"#You can use the following macros:\n"
				"#\t${REMOTE_DIR}: the remote directory that you specify.\n"
				"#\t${JOB_NAME}: your current job's name.\n";

	std::vector<CLaunchConfig>& launchConfigs;
	std::unordered_map<QListWidgetItem*, CLaunchConfig> tempLaunchConfigs;

	CDlgEditPath(std::vector<CLaunchConfig>& launchConfigs) : launchConfigs(launchConfigs){}

	void setup(QDialog* dlg)
	{
		// build the left, right panes
		QWidget* leftPane = buildLeftPane();
		QWidget* rightPane = buildRightPane();

		// build the splitter
		QSplitter* split = new QSplitter;
		split->setOrientation(Qt::Horizontal);
		split->addWidget(leftPane);
		split->addWidget(rightPane);

		// build the main layout
		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addWidget(split);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		mainLayout->addWidget(bb);

		dlg->setLayout(mainLayout);

		QObject::connect(addConfigBtn, SIGNAL(clicked()), dlg, SLOT(on_addConfigBtn_Clicked()));
		QObject::connect(delConfigBtn, SIGNAL(clicked()), dlg, SLOT(on_delConfigBtn_Clicked()));

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(launchType, QOverload<int>::of(&QComboBox::activated), configStack, &QStackedWidget::setCurrentIndex);
		QObject::connect(launchConfigList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), dlg, SLOT(on_selection_change(QListWidgetItem*, QListWidgetItem*)));
		QObject::connect(editLocalPath, SIGNAL(clicked()), dlg, SLOT(on_editLocalPath_clicked()));
	}

	QWidget* buildLeftPane()
	{
		// build the launch configuration widget
		launchConfigList = new QListWidget;
		launchConfigList->setEditTriggers(QAbstractItemView::DoubleClicked);
		launchConfigList->setDragDropMode(QAbstractItemView::InternalMove);
		launchConfigList->setDropIndicatorShown(true);
		for (CLaunchConfig& lc : launchConfigs)
		{
			QListWidgetItem* item = new QListWidgetItem(lc.name.c_str(), launchConfigList);
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			tempLaunchConfigs[item] = lc;
		}
//		launchConfigList->setFixedWidth(launchConfigList->sizeHintForColumn(0) + 10);

		// build the add config button
		QAction* addConfig = new QAction;
		addConfig->setIcon(QIcon(":/icons/selectAdd.png"));
		addConfigBtn = new QToolButton;
		addConfigBtn->setDefaultAction(addConfig);
		addConfigBtn->setToolTip("Add a new launch configuration");

		// build the delete config button
		QAction* delConfig = new QAction;
		delConfig->setIcon(QIcon(":/icons/selectSub.png"));
		delConfigBtn = new QToolButton;
		delConfigBtn->setDefaultAction(delConfig);
		delConfigBtn->setToolTip("Remove launch configuration");

		// build the layouts
		QHBoxLayout* h1 = new QHBoxLayout;
		h1->setContentsMargins(0, 0, 0, 0);
		h1->addWidget(addConfigBtn);
		h1->addWidget(delConfigBtn);
		h1->setAlignment(Qt::AlignLeft);

		QVBoxLayout* v2 = new QVBoxLayout;
		v2->setContentsMargins(0, 0, 0, 0);
		v2->addWidget(launchConfigList);
		v2->addLayout(h1);

		QWidget* leftPane = new QWidget;
		leftPane->setLayout(v2);

		return leftPane;
	}

	QWidget* buildRightPane()
	{
		// form for right-hand side pane
		QFormLayout* baseForm = new QFormLayout;
		baseForm->setLabelAlignment(Qt::AlignRight);
		baseForm->setContentsMargins(0, 0, 0, 0);
		baseForm->addRow("Launch Type:", launchType = new QComboBox);

		// the stack holds the widgets for the different launch configuration types selected via launchType
		configStack = new DynamicStackedWidget;

		// add the different config widgets
		launchType->addItem("Local");
		localPage = buildLocalConfigWidget();
		configStack->addWidget(localPage);

#ifdef HAS_SSH
		launchType->addItem("Remote");
		remotePage = buildRemoteConfigWidget();
		configStack->addWidget(remotePage);

		launchType->addItem("PBS Queue");
		PBSPage = buildPBSConfigWidget();
		configStack->addWidget(PBSPage);

		launchType->addItem("Slurm Queue");
		SlurmPage = buildSlurmConfigWidget();
		configStack->addWidget(SlurmPage);

		launchType->addItem("Custom Remote");
		customPage = buildCustomConfigWidget();
		configStack->addWidget(customPage);
#endif

		QVBoxLayout* v1 = new QVBoxLayout;
		v1->setContentsMargins(0, 0, 0, 0);
		v1->addLayout(baseForm);
		v1->addWidget(configStack);

		QWidget* rightPane = new QWidget;
		rightPane->setLayout(v1);

		rhsStack = new QStackedWidget;
		QLabel* lbl = new QLabel("(no properties)");
		lbl->setAlignment(Qt::AlignTop);
		rhsStack->addWidget(lbl);
		rhsStack->addWidget(rightPane);

		return rhsStack;
	}

	QWidget* buildLocalConfigWidget()
	{
		editLocalPath = new QToolButton; editLocalPath->setText("...");
		QHBoxLayout* localPathLayout = new QHBoxLayout;
		localPathLayout->addWidget(localPath = new QLineEdit);
		localPathLayout->addWidget(editLocalPath);

		QWidget* localPage = new QWidget;
		QFormLayout* localForm = new QFormLayout;
		localForm->setLabelAlignment(Qt::AlignRight);
		localForm->setContentsMargins(0, 0, 0, 0);
		localForm->addRow("FEBio Executable:", localPathLayout);
		localPage->setLayout(localForm);

		return localPage;
	}

	QWidget* buildRemoteConfigWidget()
	{
		QFormLayout* remoteForm = new QFormLayout;
		remoteForm->setLabelAlignment(Qt::AlignRight);
		remoteForm->setContentsMargins(0, 0, 0, 0);

		// Remote config widgets
		remoteForm->addRow("Remote Executable:", remotePath = new QLineEdit);
		remoteForm->addRow("Server:", remoteServer = new QLineEdit);
		remoteForm->addRow("Port:", remotePort = new QSpinBox);
		remotePort->setValue(22);
		remotePort->setMaximum(65535);
		remoteForm->addRow("Username:", remoteUserName = new QLineEdit);
		remoteForm->addRow("Remote Directory:", remoteRemoteDir = new QLineEdit);

		QWidget* remotePage = new QWidget;
		remotePage->setLayout(remoteForm);

		return remotePage;
	}

	QWidget* buildPBSConfigWidget()
	{
		QFormLayout* PBSForm = new QFormLayout;
		PBSForm->setLabelAlignment(Qt::AlignRight);
		PBSForm->setContentsMargins(0, 0, 0, 0);

		// PBS config widgets
		PBSForm->addRow("Remote Executable:", PBSPath = new QLineEdit);
		PBSForm->addRow("Server:", PBSServer = new QLineEdit);
		PBSForm->addRow("Port:", PBSPort = new QSpinBox);
		PBSPort->setValue(22);
		PBSPort->setMaximum(65535);
		PBSForm->addRow("Username:", PBSUserName = new QLineEdit);
		PBSForm->addRow("Remote Directory:", PBSRemoteDir = new QLineEdit);
		PBSText = new QPlainTextEdit(DefaultPBSText);
		PBSText->setPlaceholderText(DefaultPBSText);

		PBSText->setWordWrapMode(QTextOption::NoWrap);
		PBSText->setMinimumWidth(PBSText->document()->size().width() * 1.1);
		PBSText->setMinimumHeight(PBSText->fontMetrics().height() * (PBSText->document()->lineCount() + 2));

		QVBoxLayout* PBSV1 = new QVBoxLayout;
		PBSV1->addLayout(PBSForm);
		PBSV1->addWidget(PBSText);

		QWidget* PBSPage = new QWidget;
		PBSPage->setLayout(PBSV1);
		return PBSPage;
	}

	QWidget* buildSlurmConfigWidget()
	{
		QFormLayout* SlurmForm = new QFormLayout;
		SlurmForm->setLabelAlignment(Qt::AlignRight);
		SlurmForm->setContentsMargins(0, 0, 0, 0);

		// Slurm config widgets
		SlurmForm->addRow("Remote Executable:", SlurmPath = new QLineEdit);
		SlurmForm->addRow("Server:", SlurmServer = new QLineEdit);
		SlurmForm->addRow("Port:", SlurmPort = new QSpinBox);
		SlurmPort->setValue(22);
		SlurmPort->setMaximum(65535);
		SlurmForm->addRow("Username:", SlurmUserName = new QLineEdit);
		SlurmForm->addRow("Remote Directory:", SlurmRemoteDir = new QLineEdit);
		SlurmText = new QPlainTextEdit(DefaultSlurmText);
		SlurmText->setPlaceholderText(DefaultSlurmText);

		SlurmText->setWordWrapMode(QTextOption::NoWrap);
		SlurmText->setMinimumWidth(SlurmText->document()->size().width() * 1.1);
		SlurmText->setMinimumHeight(SlurmText->fontMetrics().height() * (SlurmText->document()->lineCount() + 2));

		QVBoxLayout* SlurmV1 = new QVBoxLayout;
		SlurmV1->addLayout(SlurmForm);
		SlurmV1->addWidget(SlurmText);

		QWidget* SlurmPage = new QWidget;
		SlurmPage->setLayout(SlurmV1);
		return SlurmPage;
	}

	QWidget* buildCustomConfigWidget()
	{
		QFormLayout* customForm = new QFormLayout;
		customForm->setLabelAlignment(Qt::AlignRight);
		customForm->setContentsMargins(0, 0, 0, 0);

		// Custom config widgets
//		customForm->addRow("Custom Script", customFile = new QLineEdit);
		customForm->addRow("Server:", customServer = new QLineEdit);
		customForm->addRow("Port:", customPort = new QSpinBox);
		customPort->setValue(22);
		customPort->setMaximum(65535);
		customForm->addRow("Username:", customUserName = new QLineEdit);
		customForm->addRow("Remote Directory:", customRemoteDir = new QLineEdit);

		customText = new QPlainTextEdit(DefaultCustomText);
		customText->setPlaceholderText(DefaultCustomText);

		customText->setWordWrapMode(QTextOption::NoWrap);
		customText->setMinimumWidth(customText->document()->size().width() * 1.3);
		customText->setMinimumHeight(customText->fontMetrics().height() * (customText->document()->lineCount() + 2));

		QVBoxLayout* customV1 = new QVBoxLayout;
		customV1->addLayout(customForm);
		customV1->addWidget(customText);

		QWidget* customPage = new QWidget;
		customPage->setLayout(customV1);
		return customPage;
	}
};

CDlgEditPath::CDlgEditPath(QWidget* parent, std::vector<CLaunchConfig>* launchConfigs)
	: QDialog(parent), ui(new Ui::CDlgEditPath(*launchConfigs))
{
	ui->setup(this);
}

void CDlgEditPath::UpdateConfig(QListWidgetItem* item)
{
	if(!item) return;

	CLaunchConfig& launchConfig = ui->tempLaunchConfigs[item];
	if (launchConfig.type == launchTypes::DEFAULT) return;

	int type = ui->launchType->currentIndex();
	launchConfig.type = type;
	launchConfig.name = item->text().toStdString();

	switch(type)
	{
	case LOCAL:
		launchConfig.path = ui->localPath->text().toStdString();
		break;

#ifdef HAS_SSH

	case REMOTE:
		launchConfig.path = ui->remotePath->text().toStdString();
		launchConfig.server = ui->remoteServer->text().toStdString();
		launchConfig.port = ui->remotePort->value();
		launchConfig.userName = ui->remoteUserName->text().toStdString();
		launchConfig.remoteDir = ui->remoteRemoteDir->text().toStdString();
		break;
	case PBS:
		launchConfig.path = ui->PBSPath->text().toStdString();
		launchConfig.server = ui->PBSServer->text().toStdString();
		launchConfig.port = ui->PBSPort->value();
		launchConfig.userName = ui->PBSUserName->text().toStdString();
		launchConfig.remoteDir = ui->PBSRemoteDir->text().toStdString();
//		launchConfig.jobName = ui->PBSJobName->text().toStdString();
//		launchConfig.walltime = ui->PBSWalltime->text().toStdString();
//		launchConfig.procNum = ui->PBSProcNum->value();
//		launchConfig.ram = ui->PBSRam->value();
		launchConfig.setText(ui->PBSText->toPlainText().toStdString());
		break;
	case SLURM:
		launchConfig.path = ui->SlurmPath->text().toStdString();
		launchConfig.server = ui->SlurmServer->text().toStdString();
		launchConfig.port = ui->SlurmPort->value();
		launchConfig.userName = ui->SlurmUserName->text().toStdString();
		launchConfig.remoteDir = ui->SlurmRemoteDir->text().toStdString();
//		launchConfig.jobName = ui->SlurmJobName->text().toStdString();
//		launchConfig.walltime = ui->SlurmWalltime->text().toStdString();
//		launchConfig.procNum = ui->SlurmProcNum->value();
//		launchConfig.ram = ui->SlurmRam->value();
		launchConfig.setText(ui->SlurmText->toPlainText().toStdString());
		break;
	case CUSTOM:
//		launchConfig.customFile = ui->customFile->text().toStdString();
		launchConfig.server = ui->customServer->text().toStdString();
		launchConfig.port = ui->customPort->value();
		launchConfig.userName = ui->customUserName->text().toStdString();
		launchConfig.remoteDir = ui->customRemoteDir->text().toStdString();
		launchConfig.setText(ui->customText->toPlainText().toStdString());
		break;
#endif
	case DEFAULT:
		break;
	}

	// Set width for launch config list
//	ui->launchConfigList->setFixedWidth(ui->launchConfigList->sizeHintForColumn(0) + 10);
}

void CDlgEditPath::on_selection_change(QListWidgetItem* current, QListWidgetItem* previous)
{
	if(!current) return;

	// Update the previous config
	UpdateConfig(previous);

	// Change to the current config
	ChangeToConfig(current);
}

void CDlgEditPath::on_dblClick(QListWidgetItem* item)
{
	item->setFlags (item->flags () | Qt::ItemIsEditable);

	ui->launchConfigList->editItem(item);
}

void CDlgEditPath::ChangeToConfig(QListWidgetItem* item)
{
	if (item == nullptr)
	{
		ui->rhsStack->setCurrentIndex(0);
		return;
	}

	CLaunchConfig& launchConfig = ui->tempLaunchConfigs[item];
	if (launchConfig.type == launchTypes::DEFAULT)
	{
		ui->rhsStack->setCurrentIndex(0);
		return;
	}
	else ui->rhsStack->setCurrentIndex(1);

	ui->launchType->setCurrentIndex(launchConfig.type);
	ui->localPath->setText(QString::fromStdString(launchConfig.path));

#ifdef HAS_SSH
	ui->remotePath->setText(QString::fromStdString(launchConfig.path));
	ui->remoteServer->setText(QString::fromStdString(launchConfig.server));
	ui->remotePort->setValue(launchConfig.port);
	ui->remoteUserName->setText(QString::fromStdString(launchConfig.userName));
	ui->remoteRemoteDir->setText(QString::fromStdString(launchConfig.remoteDir));

	ui->PBSPath->setText(QString::fromStdString(launchConfig.path));
	ui->PBSServer->setText(QString::fromStdString(launchConfig.server));
	ui->PBSPort->setValue(launchConfig.port);
	ui->PBSUserName->setText(QString::fromStdString(launchConfig.userName));
	ui->PBSRemoteDir->setText(QString::fromStdString(launchConfig.remoteDir));
//	ui->PBSJobName->setText(QString::fromStdString(launchConfig.jobName));
//	ui->PBSWalltime->setText(QString::fromStdString(launchConfig.walltime));
//	ui->PBSProcNum->setValue(launchConfig.procNum);
//	ui->PBSRam->setValue(launchConfig.ram);
	ui->PBSText->document()->setPlainText(ui->DefaultPBSText);

	ui->SlurmPath->setText(QString::fromStdString(launchConfig.path));
	ui->SlurmServer->setText(QString::fromStdString(launchConfig.server));
	ui->SlurmPort->setValue(launchConfig.port);
	ui->SlurmUserName->setText(QString::fromStdString(launchConfig.userName));
	ui->SlurmRemoteDir->setText(QString::fromStdString(launchConfig.remoteDir));
//	ui->SlurmJobName->setText(QString::fromStdString(launchConfig.jobName));
//	ui->SlurmWalltime->setText(QString::fromStdString(launchConfig.walltime));
//	ui->SlurmProcNum->setValue(launchConfig.procNum);
//	ui->SlurmRam->setValue(launchConfig.ram);
	ui->SlurmText->document()->setPlainText(ui->DefaultSlurmText);

//	ui->customFile->setText(QString::fromStdString(launchConfig.customFile));
	ui->customServer->setText(QString::fromStdString(launchConfig.server));
	ui->customPort->setValue(launchConfig.port);
	ui->customUserName->setText(QString::fromStdString(launchConfig.userName));
	ui->customRemoteDir->setText(QString::fromStdString(launchConfig.remoteDir));
	ui->customText->document()->setPlainText(ui->DefaultCustomText);

	if(!launchConfig.getText().empty())
	{
		switch(launchConfig.type)
		{
		case PBS:
			ui->PBSText->document()->setPlainText(launchConfig.getText().c_str());

			break;
		case SLURM:
			ui->SlurmText->document()->setPlainText(launchConfig.getText().c_str());
			break;
		case CUSTOM:
			ui->customText->document()->setPlainText(launchConfig.getText().c_str());
			break;
		}
	}

#endif

	ui->configStack->setCurrentIndex(launchConfig.type);
}

void CDlgEditPath::on_addConfigBtn_Clicked()
{
	ui->launchConfigList->addItem("New");

	QListWidgetItem* newItem = ui->launchConfigList->item(ui->launchConfigList->count() - 1);
	newItem->setFlags (newItem->flags () | Qt::ItemIsEditable);

	ui->tempLaunchConfigs[newItem] = CLaunchConfig();

	ui->launchConfigList->setCurrentItem(newItem);

	ui->launchConfigList->editItem(newItem);
}

void CDlgEditPath::on_delConfigBtn_Clicked()
{
	if(ui->tempLaunchConfigs.size() == 1)
	{
		QMessageBox::critical(this, "FEBio Studio", "You cannot delete the last launch configuration.");
		return;
	}

	QListWidgetItem* current = ui->launchConfigList->takeItem(ui->launchConfigList->currentRow());

	ui->tempLaunchConfigs.erase(current);

	delete current;

	ui->launchConfigList->setCurrentRow(0);
}

void CDlgEditPath::on_editLocalPath_clicked()
{
#ifdef WIN32
	QString filter("Executables (*.exe)");
#else
	QString filter("All files (*)");
#endif

	QString exePath = QFileDialog::getOpenFileName(this, "Edit Local Path", "", filter);
	if (exePath.isEmpty() == false)
	{
		ui->localPath->setText(exePath);
	}
}

int CDlgEditPath::GetLCIndex()
{
	return ui->launchConfigList->currentRow();
}

bool CDlgEditPath::ErrorCheck(int index)
{
	ui->launchConfigList->setCurrentRow(index);

	int type = ui->launchType->currentIndex();

	if(ui->launchConfigList->currentItem()->text().isEmpty())
	{
		QMessageBox::critical(this, "FEBio Studio", "Please enter a launch configuration name.");
		return false;
	}

	switch(type)
	{
	case LOCAL:
		if(ui->localPath->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a path to the FEBio executable.");
			return false;
		}
		break;

#ifdef HAS_SSH
	case REMOTE:
		if(ui->remotePath->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a path to the FEBio executable.");
			return false;
		}
		if(ui->remoteServer->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a server address.");
			return false;
		}

		if(ui->remoteUserName->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a username.");
			return false;
		}

		if(ui->remoteRemoteDir->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a remote directory.");
			return false;
		}
		break;

	case PBS:
		if(ui->PBSPath->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a path to the FEBio executable.");
			return false;
		}
		if(ui->PBSServer->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a server address.");
			return false;
		}

		if(ui->PBSUserName->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a username.");
			return false;
		}

		if(ui->PBSRemoteDir->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a remote directory.");
			return false;
		}

//		if(ui->PBSWalltime->text().isEmpty())
//		{
//			QMessageBox::critical(this, "FEBio Studio", "Please enter a walltime.");
//			return false;
//		}
//		break;

	case SLURM:
		if(ui->SlurmPath->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a path to the FEBio executable.");
			return false;
		}
		if(ui->SlurmServer->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a server address.");
			return false;
		}

		if(ui->SlurmUserName->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a username.");
			return false;
		}

		if(ui->SlurmRemoteDir->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a remote directory.");
			return false;
		}

//		if(ui->SlurmWalltime->text().isEmpty())
//		{
//			QMessageBox::critical(this, "FEBio Studio", "Please enter a walltime.");
//			return false;
//		}
		break;
	case CUSTOM:
		if(ui->customText->toPlainText().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Your custom script cannot be empty.");
			return false;
		}

		if(ui->customServer->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a server address.");
			return false;
		}

		if(ui->customUserName->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a username.");
			return false;
		}

		if(ui->customRemoteDir->text().isEmpty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Please enter a remote directory.");
			return false;
		}


		break;
#endif
	}

	return true;
}


void CDlgEditPath::accept()
{
	// Trigger an update for the current config
	UpdateConfig(ui->launchConfigList->currentItem());

	// See if any of the configs have changed
	bool changed = false;

	if(ui->launchConfigs.size() != ui->launchConfigList->count())
	{
		changed = true;
	}
	else
	{
		for(int index = 1; index < ui->launchConfigs.size(); index++)
		{
			if(ui->launchConfigs.at(index) != ui->tempLaunchConfigs[ui->launchConfigList->item(index)])
			{
				changed = true;
				break;
			}
		}
	}

	if(changed)
	{
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(this, "FEBio Studio", "Would you like to save your changes?",
				QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);


		if(reply == QMessageBox::Yes)
		{
			// Check to make sure that each config has all necessary fields filled out
			for(int index = 1; index < ui->launchConfigList->count(); index++)
			{
				if(!ErrorCheck(index)) return;
			}

			for(int index = 1; index < ui->launchConfigList->count(); index++)
			{
				CLaunchConfig& lci = ui->tempLaunchConfigs[ui->launchConfigList->item(index)];

				if (index < ui->launchConfigs.size())
					ui->launchConfigs[index] = lci;
				else 
					ui->launchConfigs.push_back(lci);
			}
		}
		else if(reply == QMessageBox::Cancel)
		{
			return;
		}
	}

	QDialog::accept();
}
