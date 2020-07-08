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
