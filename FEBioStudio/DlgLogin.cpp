#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include "DlgLogin.h"

class Ui::CDlgLogin
{
public:
	QLineEdit* userName;
	QLineEdit* password;

	void setupUi(QDialog* parent)
	{
		QVBoxLayout* layout = new QVBoxLayout;

		QLabel* msg = new QLabel("To log into the model repository, please enter your febio.org username and password.");
		msg->setWordWrap(true);

		layout->addWidget(msg);

		QFormLayout* loginForm = new QFormLayout;
		loginForm->addRow("Username:",  userName= new QLineEdit);
		loginForm->addRow("Password:", password = new QLineEdit);
		password->setEchoMode(QLineEdit::Password);

		QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok
				| QDialogButtonBox::Cancel);

		QObject::connect(box, &QDialogButtonBox::accepted, parent, &QDialog::accept);
		QObject::connect(box, &QDialogButtonBox::rejected, parent, &QDialog::reject);
		loginForm->addWidget(box);

		layout->addLayout(loginForm);

		parent->setLayout(layout);
	}
};

CDlgLogin::CDlgLogin() : ui(new Ui::CDlgLogin)
{
	ui->setupUi(this);
}

QString CDlgLogin::username()
{
	return ui->userName->text();
}

QString CDlgLogin::password()
{
	return ui->password->text();
}

void CDlgLogin::accept()
{
	if(ui->userName->text().isEmpty())
	{
		QMessageBox::critical(this, "Login", "Please enter your febio.org username.", QMessageBox::Ok);
		return;
	}

	if(ui->password->text().isEmpty())
	{
		QMessageBox::critical(this, "Login", "Please enter your febio.org password.", QMessageBox::Ok);
		return;
	}

	if(ui->userName->text().contains("@"))
	{
		QMessageBox::critical(this, "Login", "Please login using your febio.org username, not your email address.", QMessageBox::Ok);
		return;
	}

	QDialog::accept();
}



