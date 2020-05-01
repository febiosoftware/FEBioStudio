#include "stdafx.h"
#include "DlgTimeSettings.h"
#include <QFormLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QLineEdit>
#include "PostDocument.h"

class Ui::CDlgTimeSettings
{
public:
	QComboBox*			mode;
	QDoubleSpinBox*		fps;
	QSpinBox*			start;
	QSpinBox*			end;	
	QCheckBox*			loop;
	QCheckBox*			fix;
	QLineEdit*		step;

public:
	void setupUi(QDialog* parent)
	{
		QVBoxLayout* mainLayout = new QVBoxLayout;
		QFormLayout* pform = new QFormLayout;
		pform->addRow("Mode:", mode = new QComboBox);
		pform->addRow("FPS:" , fps = new QDoubleSpinBox);
		pform->addRow("Start:", start = new QSpinBox);
		pform->addRow("End:", end = new QSpinBox);
		mainLayout->addLayout(pform);

		mode->addItem("Forward");
		mode->addItem("Reverse");
		mode->addItem("Cycle");

		fps->setRange(1.0, 100.0);
		fps->setSingleStep(0.5);
		fps->setDecimals(1);

		mainLayout->addWidget(loop = new QCheckBox("loop"));
		mainLayout->addWidget(fix  = new QCheckBox("fixed time step"));

		pform = new QFormLayout;
		pform->addRow("Time step size:", step = new QLineEdit);
		mainLayout->addLayout(pform);

		step->setValidator(new QDoubleValidator(0.0, 1e5, 4));

		QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		mainLayout->addWidget(buttonBox);

		parent->setLayout(mainLayout);

		QObject::connect(buttonBox, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(buttonBox, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgTimeSettings::CDlgTimeSettings(CPostDocument* doc, QWidget* parent) : QDialog(parent), ui(new Ui::CDlgTimeSettings), m_doc(doc)
{
	ui->setupUi(this);

	TIMESETTINGS& time = doc->GetTimeSettings();
	int N = doc->GetStates();
	ui->mode->setCurrentIndex(time.m_mode);
	ui->fps->setValue(time.m_fps);
	ui->start->setRange(1, N);
	ui->start->setValue(time.m_start + 1);
	ui->end->setRange(1, N);
	ui->end->setValue(time.m_end + 1);
	ui->loop->setChecked(time.m_bloop);
	ui->fix->setChecked(time.m_bfix);
	ui->step->setText(QString::number(time.m_dt));
}

void CDlgTimeSettings::accept()
{
	TIMESETTINGS& time = m_doc->GetTimeSettings();
	int N = m_doc->GetStates();

	time.m_mode = ui->mode->currentIndex();
	time.m_fps  = ui->fps->value();
	time.m_start = ui->start->value() - 1;
	time.m_end   = ui->end->value() - 1;
	time.m_bloop = ui->loop->isChecked();
	time.m_bfix  = ui->fix->isChecked();
	time.m_dt    = ui->step->text().toDouble();

	if ((time.m_start < 0) || (time.m_end >= N) || (time.m_start > time.m_end))
	{
		QMessageBox::critical(this, "Time Settings", "Invalid time range");
		return;
	}

	QDialog::accept();
}
