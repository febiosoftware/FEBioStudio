#include <QLabel>
#include <QResizeEvent>
#include <QBoxLayout>
#include "WrapLabel.h"



WrapLabel::WrapLabel(QString text, QWidget* parent)
		: QWidget(parent), processEvent(true)
	{
		words = text.split(" ");

		QLabel temp;
		for(QString word : words)
		{
			lengths.push_back(temp.fontMetrics().horizontalAdvance(word));
		}

		spaceSize = temp.fontMetrics().horizontalAdvance(" ");

		layout = new QVBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);
		setLayout(layout);

		setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);

		setContentsMargins(0, 0, 0, 0);

	}


void WrapLabel::resizeEvent(QResizeEvent *event)
{

	if(event->oldSize().width() != event->size().width())
	{

		for(auto label : labels)
		{
			delete label;
		}

		labels.clear();

		int width = event->size().width();

		int index, currentWidth = 0;
		QString currentString;
		for(index = 0; index < lengths.size(); index++)
		{


			if(currentWidth + lengths[index] > width)
			{
				QLabel* next = new QLabel(currentString);
				next->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
				layout->addWidget(next);
				labels.push_back(next);

				currentString.clear();
				currentWidth = 0;

			}

			currentString += words[index] + " ";

			currentWidth += lengths[index] + spaceSize;

		}

		QLabel* next = new QLabel(currentString);
		next->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
		layout->addWidget(next);
		labels.push_back(next);
	}

	QWidget::resizeEvent(event);
}
