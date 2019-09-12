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
#include <QStackedWidget>
#include "LaunchConfig.h"

namespace Ui
{
	class CDlgEditPath;
}

class CDlgEditPath : public QDialog
{
	Q_OBJECT

public:
	CDlgEditPath(QWidget* parent);
	~CDlgEditPath() {}

	void SetLaunchConfig(CLaunchConfig launchConfig);
	CLaunchConfig GetLaunchConfig();

	void accept() override;

private:
	Ui::CDlgEditPath* ui;

};
