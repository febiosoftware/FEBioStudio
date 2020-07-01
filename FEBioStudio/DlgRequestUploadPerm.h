#pragma once

#include <QDialog>

namespace Ui {
	class CDlgRequestUploadPerm;
}

class CDlgRequestUploadPerm : public QDialog
{
	Q_OBJECT

public:
	CDlgRequestUploadPerm(QWidget* parent = nullptr);

	QString getEmail();
	QString getOrg();
	QString getDescription();

private:
	Ui::CDlgRequestUploadPerm* ui;
};
