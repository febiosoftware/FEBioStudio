/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include <QWidget>
#include <vector>
#include "PublicationWidget.h"

namespace Ui{
class CPublicationWidgetView;
}

class CPublicationWidget;

class CPublicationWidgetView : public QWidget
{
	Q_OBJECT

public:

	enum Type {LIST, EDITABLE, SELECTABLE};

	CPublicationWidgetView(Type type = LIST, bool scroll = true, bool frame = false);

	CPublicationWidget* addPublication(CPublicationWidget* pub);
	CPublicationWidget* addPublication(QVariantMap& data);
	CPublicationWidget* addPublication(QString title, QString year, QString journal,
			QString volume, QString issue, QString pages, QString DOI,
			QStringList authorGiven, QStringList authorFamily);

	CPublicationWidget* addPublicationCopy(CPublicationWidget& oldPub);

	void clear();
	int count();

	int getType() const;

	QList<QVariant> getPublicationInfo();
	const std::vector<CPublicationWidget*>& getPublications();

public slots:
	void on_addPub_triggered();
	void on_delPub_triggered();

signals:
	void chosen_publication(CPublicationWidget* pub);

private:
	Ui::CPublicationWidgetView* ui;
	Type type;

	std::vector<CPublicationWidget*> pubs;
};
