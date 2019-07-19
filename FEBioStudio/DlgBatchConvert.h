#pragma once
#include <QDialog>

namespace Ui {
	class CDlgBatchConvert;
}

class CDlgBatchConvert : public QDialog
{
public:
	CDlgBatchConvert(QWidget* parent = nullptr);

	void accept() override;

	int GetFileFormat();

private:
	Ui::CDlgBatchConvert*	ui;
};
