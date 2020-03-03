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

	CPublicationWidgetView(Type type = LIST);

	CPublicationWidget* addPublication(CPublicationWidget* pub);
	CPublicationWidget* addPublication(QVariantMap& data);
	CPublicationWidget* addPublication(QString title, QString year, QString journal,
			QString volume, QString issue, QString pages, QString DOI,
			QStringList authorGiven, QStringList authorFamily);

	void clear();
	int count();

	int getType() const;

	QList<QVariant> getPublicationInfo();

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
