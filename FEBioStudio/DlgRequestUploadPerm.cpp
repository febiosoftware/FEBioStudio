#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QDialogButtonBox>
#include "DlgRequestUploadPerm.h"
#include "WrapLabel.h"

class Ui::CDlgRequestUploadPerm
{
public:
	QLineEdit* email;
	QLineEdit* org;
	QTextEdit* description;

public:
	void setupUI(::CDlgRequestUploadPerm* parent)
	{
		QVBoxLayout* layout = new QVBoxLayout;
		layout->addWidget(new WrapLabel("In order to upload to the repository, you must first request permission. "
				"Please fill out this form, and our team will notify you when your account has been given permission to "
				"upload."));

		QFormLayout* form = new QFormLayout;
		form->addRow("Email Address:", email = new QLineEdit);
		email->setPlaceholderText("user@example.com");
		form->addRow("Organization:", org = new QLineEdit);
		org->setPlaceholderText("(e.g. University of Utah)");

		layout->addLayout(form);

		layout->addWidget(new QLabel("Why do you want to upload files?"));

		layout->addWidget(description = new QTextEdit);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		layout->addWidget(bb);

		QObject::connect(bb, &QDialogButtonBox::accepted, parent, &::CDlgRequestUploadPerm::accept);
		QObject::connect(bb, &QDialogButtonBox::rejected, parent, &::CDlgRequestUploadPerm::reject);

		parent->setLayout(layout);

	}

};


CDlgRequestUploadPerm::CDlgRequestUploadPerm(QWidget* parent)
	: QDialog(parent), ui(new Ui::CDlgRequestUploadPerm)
{
	ui->setupUI(this);
}


QString CDlgRequestUploadPerm::getEmail()
{
	return ui->email->text();
}
QString CDlgRequestUploadPerm::getOrg()
{
	return ui->org->text();
}
QString CDlgRequestUploadPerm::getDescription()
{
	return ui->description->toPlainText();
}






