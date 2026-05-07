#include "FEBioStudyReportDoc.h"
#include <FECore/FEBioReport.h>
#include "HTMLComposer.h"
#include "ChartBuilder.h"
#include <QFileInfo>

static const QColor palette[] = {
	QColor("#1F77B4"),
	QColor("#D62728"),
	QColor("#2CA02C"),
	QColor("#FF7F0E"),
	QColor("#9467BD"),
	QColor("#8C564B"),
	QColor("#17BECF"),
	QColor("#E377C2"),
	QColor("#7F7F7F"),
	QColor("#BCBD22"),
	QColor("#AEC7E8"),
	QColor("#FF9896"),
	QColor("#98DF8A"),
	QColor("#C5B0D5"),
	QColor("#FFBB78"),
	QColor("#9EDAE5"),
};

CFEBioStudyReportDoc::CFEBioStudyReportDoc(CMainWindow* wnd) : CHTMLDocument(wnd)
{
}

bool CFEBioStudyReportDoc::OpenReportFile(const QString& fileName)
{
	SetDocFilePath(fileName.toStdString());

	FEBioReport report;
	if (!report.Load(fileName.toStdString()))
	{
		return false;
	}

	int tableCount = 1; // table counter for naming extracted tables
	int figCount = 1; // figure counter for naming extracted figures

	// compose the html
	HTMLComposer html;
	html.heading1(QString::fromStdString(report.GetTitle()));
	html.paragraph(html.bold("Options file: ") + QString::fromStdString(report.GetOptionsFile()));
	html.paragraph(html.bold("Status: ") + (report.GetStatus() != 0 ? html.bg_color("Success", "green") : html.bg_color("Failed", "red")));
	size_t nsections = report.Sections();
	if (nsections > 0)
	{
		bool tableOpen = false;
		for (size_t i = 0; i < nsections; ++i)
		{
			const FEReportSection& sec = report.GetSection(i);
			html.heading2(QString::fromStdString(sec.name));

			size_t items = sec.Items();
			for (size_t j = 0; j < items; ++j)
			{
				const FEReportItem& item = sec.GetItem(j);

				// process value items first. These will be displayed in a table format.
				if (const auto* valItem = dynamic_cast<const FEReportValue*>(&item))
				{
					if (!tableOpen) { html.table_start(); tableOpen = true; }
					QStringList row;
					QString name = QString::fromStdString(valItem->name);
					QString value = QString::fromStdString(valItem->value);
					if (!valItem->units.empty())
					{
						value += " " + QString::fromStdString(valItem->units);
					}
					row << html.bold(name) << value;
					html.table_row(row);
				}
				else if (tableOpen)
				{
					html.table_end();
					tableOpen = false;
				}

				if (const auto* txtItem = dynamic_cast<const FEReportText*>(&item))
				{
					html.paragraph(QString::fromStdString(txtItem->text));
				}
				else if (const auto* fileItem = dynamic_cast<const FEReportFile*>(&item))
				{
					QString fileName = QString::fromStdString(fileItem->filename);
					QString description = QString::fromStdString(fileItem->description);
					if (description.isEmpty())
						html.paragraph(html.link(fileName));
					else
						html.paragraph(description + ": " + html.link(fileName));
				}
				else if (const auto* tableItem = dynamic_cast<const FEReportTable*>(&item))
				{
					// Don't display tables directly. They will be displayed either through table-views or charts. 
				}
				else if (const auto* tableViewItem = dynamic_cast<const FEReportTableView*>(&item))
				{
					FEReportTable table = report.GetTable(tableViewItem->tableId);
					size_t nrows = table.Rows();
					size_t ncols = table.Columns();
					if (nrows > 0 && ncols > 0)
					{
						html.break_line();
						if (!tableViewItem->tableTitle.empty())
						{
							html.heading3(QString::fromStdString(tableViewItem->tableTitle));
						}

						QStringList headings;
						for (size_t c = 0; c < ncols; ++c)
						{
							headings << QString::fromStdString(table.GetColumnName(c));
						}
						html.table_start();
						html.table_headings(headings);
						for (size_t r = 0; r < nrows; ++r)
						{
							QStringList row;
							for (size_t c = 0; c < ncols; ++c)
							{
								FEReportTable::TableEntry entry = table.GetEntry(r, c);
								if (std::holds_alternative<double>(entry))
								{
									row << QString::number(std::get<double>(entry));
								}
								else if (std::holds_alternative<std::string>(entry))
								{
									QString val = QString::fromStdString(std::get<std::string>(entry));
									if (val.compare("success", Qt::CaseInsensitive) == 0)
										row << html.bg_color(val, "green");
									else if (val.compare("failed", Qt::CaseInsensitive) == 0)
										row << html.bg_color(val, "red");
									else
										row << val;
								}
								else row << "";
							}
							html.table_row(row);
						}
						html.table_end();

						if (!tableViewItem->tableCaption.empty())
						{
							QString caption = QString::fromStdString(tableViewItem->tableCaption);
							QString tableName = QString("Table %1.").arg(tableCount++);
							html.paragraph(html.italic(html.bold(tableName) + " " + caption));
						}
						html.break_line();
					}
				}
				else if (const auto* chartItem = dynamic_cast<const FEReportChart*>(&item))
				{
					QPixmap pixmap; // the pixmap to hold the generated chart image
					if ((chartItem->chartType == FEReportChart::Bar) && !chartItem->dataSeries.empty())
					{
						auto& series = chartItem->dataSeries[0];

						// get the data items
						FEReportChart::ChartData labelData = series.FindDataByRole(FEReportChart::Label);
						FEReportChart::ChartData valueData = series.FindDataByRole(FEReportChart::Value);

						// get the column data from the referenced table
						FEReportTable::TableColumn labels = report.GetTableColumn(labelData.tableId, labelData.columnName);
						FEReportTable::TableColumn values = report.GetTableColumn(valueData.tableId, valueData.columnName);

						if (labels.data.size() == values.data.size())
						{
							BarChartBuilder bar(800, 400);
							bar.SetTitle(QString::fromStdString(chartItem->chartTitle));

							for (int i=0; i<labels.data.size(); ++i)
							{
								FEReportTable::TableEntry labelEntry = labels.data[i];
								FEReportTable::TableEntry valueEntry = values.data[i];
								if (!std::holds_alternative<std::string>(labelEntry) || !std::holds_alternative<double>(valueEntry))
								{
									continue; // invalid data
								}
								std::string label = std::get<std::string>(labelEntry);
								double value = std::get<double>(valueEntry);
								bar.AddBar(value, palette[0]);
							}

							if (!chartItem->chartCaption.empty())
							{
								QString caption = QString::fromStdString(chartItem->chartCaption);
								QString figName = QString("Figure %1.").arg(figCount);
								html.paragraph(html.italic(html.bold(figName) + " " + caption));
							}

							pixmap = bar.GetPixmap();
						}
					}
					if ((chartItem->chartType == FEReportChart::Pie) && !chartItem->dataSeries.empty())
					{
						auto& series = chartItem->dataSeries[0];
						// get the data items
						FEReportChart::ChartData labelData = series.FindDataByRole(FEReportChart::Label);
						FEReportChart::ChartData valueData = series.FindDataByRole(FEReportChart::Value);

						// get the column data from the referenced table
						FEReportTable::TableColumn labels = report.GetTableColumn(labelData.tableId, labelData.columnName);
						FEReportTable::TableColumn values = report.GetTableColumn(valueData.tableId, valueData.columnName);

						if (labels.data.size() == values.data.size())
						{
							PieChartBuilder pie(400, 400);

							double total = 0.0;
							for (int i = 0; i < labels.data.size(); ++i)
							{
								FEReportTable::TableEntry valueEntry = values.data[i];
								if (!std::holds_alternative<double>(valueEntry))
								{
									continue; // invalid data
								}
								double value = std::get<double>(valueEntry);
								total += value;
							}
							if (total == 0) total = 1.0; // avoid division by zero

							for (int i = 0; i < labels.data.size(); ++i)
							{
								FEReportTable::TableEntry labelEntry = labels.data[i];
								FEReportTable::TableEntry valueEntry = values.data[i];
								if (!std::holds_alternative<std::string>(labelEntry) || !std::holds_alternative<double>(valueEntry))
								{
									continue; // invalid data
								}
								std::string label = std::get<std::string>(labelEntry);
								double value = std::get<double>(valueEntry);
								pie.AddSlice(value / total, palette[i % (sizeof(palette) / sizeof(palette[0]))], QString::fromStdString(label));
							}

							pixmap = pie.GetPixmap();
						}
					}
					if ((chartItem->chartType == FEReportChart::Line) && !chartItem->dataSeries.empty())
					{
						LineChartBuilder lineChart(800, 400);
						lineChart.SetTitle(QString::fromStdString(chartItem->chartTitle));

						int n = 0;
						for (auto& series : chartItem->dataSeries)
						{
							FEReportChart::ChartData xData = series.FindDataByRole(FEReportChart::X);
							FEReportTable::TableColumn xval = report.GetTableColumn(xData.tableId, xData.columnName);
							if (xval.type != FEReportTable::Numeric) continue;
							if (xval.data.empty()) continue;

							if (n == 0) lineChart.SetXAxisLabel(QString::fromStdString(xval.name));

							std::vector< FEReportChart::ChartData> yData = series.FindAllDataByRole(FEReportChart::Y);
							for (auto& y : yData)
							{
								FEReportTable::TableColumn yval = report.GetTableColumn(y.tableId, y.columnName);
								if (yval.type != FEReportTable::Numeric) continue;

								if (xval.data.size() != yval.data.size()) continue; // x and y data size mismatch

								std::vector<QPointF> points;
								for (int i = 0; i < xval.data.size() && i < yval.data.size(); ++i)
								{
									points.push_back(QPointF(std::get<double>(xval.data[i]), std::get<double>(yval.data[i])));
								}

								lineChart.AddLine(points, QString::fromStdString(y.columnName), QPen(palette[n % (sizeof(palette) / sizeof(palette[0]))], 2));
								n++;
							}
						}

						if (n > 0)
						{
							pixmap = lineChart.GetPixmap();
						}
					}

					if (!pixmap.isNull())
					{
						html.break_line();
						QString figRef = QString("figure%1").arg(figCount);
						GetText()->addResource(QTextDocument::ImageResource, QUrl(figRef), pixmap);
						html.image(figRef);

						if (!chartItem->chartCaption.empty())
						{
							QString caption = QString::fromStdString(chartItem->chartCaption);
							QString figName = QString("Figure %1.").arg(figCount);
							html.paragraph(html.italic(html.bold(figName) + " " + caption));
						}
						figCount++;
						html.break_line();
					}
				}
				else if (const auto* valItem = dynamic_cast<const FEReportValue*>(&item))
				{
					// already processed above to display in table format
				}
				else
				{
					html.paragraph(html.italic("Unknown item type: ") + QString::fromStdString(item.Type()));
				}
			}

			// make sure to close any open table at the end of the file
			if (tableOpen)
			{
				html.table_end();
				tableOpen = false;
			}
		}
	}

	htmlText = html.text();
	return true;
}

void CFEBioStudyReportDoc::Activate()
{
	GetText()->setHtml(htmlText);
}