#include "stdafx.h"
#include "ObjectPanel.h"
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QFormLayout>
#include <QMenu>
#include "CColorButton.h"
#include <GeomLib/GObject.h>
#include <GLWLib/convert.h>
#include "MainWindow.h"
#include "Document.h"

CObjectPanel::CObjectPanel(CMainWindow* wnd, QWidget* parent) : QWidget(parent)
{
	name = new QLineEdit;
	name->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
	type = new QLabel;
	color = new CColorButton;

	m_wnd = wnd;

	QMenu* menu = new QMenu(this);
	menu->setObjectName("menu");
	QAction* convertAction1 = menu->addAction("Editable Surface");
	QAction* convertAction2 = menu->addAction("Editable Mesh");
	convertAction1->setObjectName("convert1");
	convertAction2->setObjectName("convert2");

	QPushButton* pb = new QPushButton("Convert");
	pb->setMenu(menu);

	QGridLayout* objGrid = new QGridLayout;
	QLabel* nameLabel = new QLabel("Name:"); nameLabel->setBuddy(name);
	QLabel* typeLabel = new QLabel("Type:"); typeLabel->setBuddy(type);

	objGrid->addWidget(nameLabel, 0, 0);
	objGrid->addWidget(name, 0, 1, 1, 2);
	objGrid->addWidget(color, 0, 3);
	objGrid->addWidget(typeLabel, 1, 0);
	objGrid->addWidget(type, 1, 1);
	objGrid->addWidget(pb, 1, 2);
	setLayout(objGrid);

	QObject::connect(color, SIGNAL(colorChanged(QColor)), this, SLOT(onColorChanged(QColor)));
}

void CObjectPanel::onColorChanged(QColor c)
{
	CDocument* doc = m_wnd->GetDocument();
	GObject* po = doc->GetActiveObject();
	if (po)
	{
		po->SetColor(toGLColor(c));
		m_wnd->RedrawGL();
	}
}

void CObjectPanel::Update()
{
	CDocument* doc = m_wnd->GetDocument();
	GObject* po = doc->GetActiveObject();
	if (po)
	{
		if (isEnabled() == false) setEnabled(true);

		name->setText(QString::fromStdString(po->GetName()));
		color->setColor(toQColor(po->GetColor()));

		std::string stype = doc->GetTypeString(po);
		type->setText(QString::fromStdString(stype));
	}
	else 
	{
		setEnabled(false);
		name->setText("");
		type->setText("");
		color->setColor(QColor(0,0,0));
	}
}
