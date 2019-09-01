#pragma once
#include <QDialog>
#include "modelcheck.h"

namespace Ui {
	class CDlgCheck;
}

class CDlgCheck : public QDialog
{
public:
	CDlgCheck(QWidget* parent);

	void SetWarnings(const std::vector<MODEL_ERROR>& errorList);

private:
	Ui::CDlgCheck*	ui;
};
