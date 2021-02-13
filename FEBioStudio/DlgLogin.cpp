/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in
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

		QLabel* msg = new QLabel("To upload or manage projects, please log in using your febio.org username and password.");
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



