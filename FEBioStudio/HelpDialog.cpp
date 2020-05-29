#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <MeshTools/FEProject.h>
#include "HelpDialog.h"
#include "WebDefines.h"

#ifdef WEBHELP
	#include <QWebEngineView>
#endif

class Ui::CHelpDialog
{
public:
#ifdef WEBHELP
	QPushButton* helpButton;
	QWebEngineView* helpView;
#endif

	QHBoxLayout* helpLayout;

public:
	void setupUi(QWidget* parent)
	{
		QVBoxLayout* mainLayout = new QVBoxLayout;
		helpLayout = new QHBoxLayout;

#ifdef WEBHELP
		helpLayout->addWidget(helpView = new QWebEngineView, 2);
		helpView->setMinimumSize(600,400);
		helpView->setVisible(false);
#endif

		mainLayout->addLayout(helpLayout);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

#ifdef WEBHELP
		helpButton = new QPushButton("Help");
		helpButton->setCheckable(true);

		bb->addButton(helpButton, QDialogButtonBox::HelpRole);
#endif

		mainLayout->addWidget(bb);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
		QObject::connect(bb, SIGNAL(helpRequested()), parent, SLOT(on_help_clicked()));

		parent->setLayout(mainLayout);
	}
};


CHelpDialog::CHelpDialog(FEProject& prj, QWidget* parent) : QDialog(parent), ui(new Ui::CHelpDialog)
{
	ui->setupUi(this);

	m_module = prj.GetModule();
}

CHelpDialog::~CHelpDialog() { delete ui; }

void CHelpDialog::on_help_clicked()
{
#ifdef WEBHELP
	if(ui->helpButton->isChecked())
	{
		m_withoutHelp = size();
		// reset min size
		setMinimumSize(0,0);
		LoadPage();
		ui->helpView->setVisible(true);
		resize(m_withHelp);
	}
	else
	{
		m_withHelp = size();
		ui->helpView->setVisible(false);
		// reset min size
		setMinimumSize(0,0);
		resize(m_withoutHelp);
	}
#endif
}

void CHelpDialog::SetLeftSideLayout(QLayout* layout)
{
	ui->helpLayout->insertLayout(0, layout);
}

void CHelpDialog::LoadPage()
{
#ifdef WEBHELP
	QString oldURL = m_url;

	SetURL();

	if(!m_url.isEmpty())
	{
		if(m_url == UNSELECTED_HELP)
		{
			ui->helpView->setHtml(QString("<html><body><p><b>%1</b></p></body></html>").arg(m_unselectedHelp));
			return;
		}

		m_url.insert(0, currentManualURL);

		if(m_url != oldURL)
		{
			ui->helpView->load(m_url);
		}
	}
	else
	{
		ui->helpView->setHtml("<html><body><p><b>There is currently no help article available for this item.</b></p></body></html>");
	}
#endif
}
