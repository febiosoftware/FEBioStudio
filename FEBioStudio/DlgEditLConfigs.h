#include "stdafx.h"
#include <QDialog>
#include "LaunchConfig.h"

namespace Ui
{
	class CDlgEditPath;
}

class QListWidgetItem;

class CDlgEditPath : public QDialog
{
	Q_OBJECT

public:
	CDlgEditPath(QWidget* parent, std::vector<CLaunchConfig>* launchConfigs);
	~CDlgEditPath() {}

	int GetLCIndex();

	void accept() override;

public slots:
	void on_selection_change(QListWidgetItem* current, QListWidgetItem* previous);
	void on_dblClick(QListWidgetItem* item);
	void on_addConfigBtn_Clicked();
	void on_delConfigBtn_Clicked();

private:
	Ui::CDlgEditPath* ui;

	bool ErrorCheck(int index);
	void UpdateConfig(QListWidgetItem* item);
	void ChangeToConfig(QListWidgetItem* item);

};
