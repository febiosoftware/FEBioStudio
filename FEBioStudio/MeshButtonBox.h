#pragma once
#include <QWidget>
#include "ClassDescriptor.h"

namespace Ui {
	class CMeshButtonBox;
}

class CMeshButtonBox : public QWidget
{
	Q_OBJECT;

public:
	CMeshButtonBox(int classType, unsigned int nflag = 0xFFFF, QWidget* parent = 0);

	QSize sizeHint() const { return QSize(75, 75); }

	ClassDescriptor* GetClassDescriptor(int n);

	void setFlag(unsigned int flag);

private slots:
	void onButtonClicked(int n);

signals:
	void buttonSelected(int n);

private:
	Ui::CMeshButtonBox*	ui;
};
