#include <QLineEdit>
#include <QComboBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFormLayout>
#include <QBoxLayout>
#include <FEMLib/FEMKernel.h>
#include "MeshTools/FEProject.h"
#include "DlgAddPhysicsItem.h"


CDlgAddPhysicsItem::CDlgAddPhysicsItem(QString windowName, int superID, FEProject& prj, QWidget* parent)
	: CHelpDialog(prj, parent), m_superID(superID)
{
	setWindowTitle(windowName);

	// Setup UI
	QString placeHolder = "(leave blank for default)";
	name = new QLineEdit; name->setPlaceholderText(placeHolder);
	name->setMinimumWidth(name->fontMetrics().size(Qt::TextSingleLine, placeHolder).width()*1.3);

	step = new QComboBox;
	type = new QListWidget;

	QFormLayout* form = new QFormLayout;
	form->setLabelAlignment(Qt::AlignRight);
	form->addRow("Name:", name);
	form->addRow("Step:", step);

	QVBoxLayout* layout = new QVBoxLayout;

	layout->addLayout(form);
	layout->addWidget(type);

	SetLeftSideLayout(layout);


	// add the steps
	FEModel& fem = prj.GetFEModel();
	for (int i = 0; i<fem.Steps(); ++i)
	{
		step->addItem(QString::fromStdString(fem.GetStep(i)->GetName()));
	}

	m_module = prj.GetModule();

	// set the types
	vector<FEClassFactory*> l = FEMKernel::FindAllClasses(m_module, superID);
	for (int i=0; i<(int)l.size(); ++i)
	{
		FEClassFactory* fac = l[i];

		QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(fac->GetTypeStr()));
		item->setData(Qt::UserRole, fac->GetClassID());
		type->addItem(item);
	}

	type->setCurrentRow(0);

	QObject::connect(type, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(accept()));
	QObject::connect(type, &QListWidget::currentRowChanged, this, &CHelpDialog::LoadPage);
}

std::string CDlgAddPhysicsItem::GetName()
{
	return name->text().toStdString();
}

int CDlgAddPhysicsItem::GetStep()
{
	return step->currentIndex();
}

int CDlgAddPhysicsItem::GetClassID()
{
	return type->currentItem()->data(Qt::UserRole).toInt();
}

void CDlgAddPhysicsItem::SetURL()
{
	int classID = type->currentItem()->data(Qt::UserRole).toInt();

	m_url = FEMKernel::FindClass(m_module, m_superID, classID)->GetHelpURL();
}





