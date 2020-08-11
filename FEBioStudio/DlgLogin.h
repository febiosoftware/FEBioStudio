#include <QDialog>

namespace Ui
{
	class CDlgLogin;
}

class CDlgLogin : public QDialog
{

public:
	CDlgLogin();

	QString username();
	QString password();

	void accept() override;

private:
	Ui::CDlgLogin* ui;
};
