#include <QLabel>
#include <QResizeEvent>
#include <QBoxLayout>
#include "WrapLabel.h"



WrapLabel::WrapLabel(QString text, QWidget* parent)
		: QWidget(parent), processEvent(true)
{

	layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);

	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);

	setContentsMargins(0, 0, 0, 0);


	setText(text);
}

void WrapLabel::setText(QString text)
{
	fullString = text;
	lengths.clear();
	words.clear();

	// Create separate word lists for each new line
	// This forces the end to a label before the new line.
	for(auto line : text.split("\n"))
	{
		words.push_back(line.split(" "));
	}

	QLabel temp;
	for(auto line : words)
	{
		lengths.push_back(std::vector<int>());

		for(QString word : line)
		{
			lengths.back().push_back(temp.fontMetrics().horizontalAdvance(word));
		}
	}


	spaceSize = temp.fontMetrics().horizontalAdvance(" ");

	reflow(geometry().width());
}

QString WrapLabel::text()
{
	return fullString;
}

void WrapLabel::reflow(int width)
{
	for(auto label : labels)
	{
		delete label;
	}

	labels.clear();

	for(int line = 0; line < words.size(); line++)
	{
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

			currentString += words[line][index] + " ";

			currentWidth += lengths[line][index] + spaceSize;

		}

		addLabel(currentString);
	}
}

void WrapLabel::addLabel(QString& text)
{
	QLabel* next = new QLabel(text);
	next->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
	layout->addWidget(next);
	labels.push_back(next);
}


void WrapLabel::resizeEvent(QResizeEvent *event)
{

	if(event->oldSize().width() != event->size().width())
	{
		reflow(event->size().width());
	}

	QWidget::resizeEvent(event);
}
