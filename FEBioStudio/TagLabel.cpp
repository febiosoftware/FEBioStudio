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

#include <QBoxLayout>
#include <QLabel>
#include "TagLabel.h"



TagLabel::TagLabel(QString text, QWidget* parent)
	: WrapLabel(text, parent)
{

}

void TagLabel::addLabel(QString& text)
{
	QLabel* next = new QLabel(text);
	next->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
	next->setTextInteractionFlags(Qt::TextBrowserInteraction);
	QObject::connect(next, &QLabel::linkActivated, this, &TagLabel::linkActivated);

	layout->addWidget(next);
	labels.push_back(next);
}

void TagLabel::setTagList(QStringList tags)
{
	words.clear();
	words.push_back(tags);

	setLengths();
}

QString TagLabel::text()
{
	QString text;

	if(words.size() > 0)
	{
		for(auto tag : words[0])
		{
			text += tag + " ";
		}

		text.remove(text.size() - 1, 1);
	}

	return text;
}





void TagLabel::reflow(int width)
{
	for(auto label : labels)
	{
		delete label;
	}

	labels.clear();

	for(int line = 0; line < words.size(); line++)
	{
		QString base("<a href=\"%1\">%1</a>");

		int index, currentWidth = 0;
		QString currentString;
		for(index = 0; index < words[line].size(); index++)
		{
			if(currentWidth + lengths[line][index] > width)
			{
				addLabel(currentString);

				currentString.clear();
				currentWidth = 0;
			}

			currentString += base.arg(words[line][index]) + " ";

			currentWidth += lengths[line][index] + spaceSize;

		}

		addLabel(currentString);
	}
}
