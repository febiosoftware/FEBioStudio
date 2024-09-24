/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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
#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>
#include <QTableWidget>
#include <QSpinBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QDialogButtonBox>
#include <QMessageBox>
#include "DlgImportData.h"

enum SeparatorOptions 
{
    TAB = 0, SPACE, COMMA, OTHER
};

class Ui::CDlgImportData
{
public:
    QTextEdit* dataEdit;
    QButtonGroup* separatorGroup;
    QLineEdit* otherSeparator;
    QSpinBox* startRow;
    QLineEdit* defaultValue;
    QTableWidget* table;

public:

    void setupUI(::CDlgImportData* parent, QString data, DataType dataType, int numCols)
    {
        this->dataType = dataType;
        this->numCols = numCols;

        QVBoxLayout* layout = new QVBoxLayout;

        QHBoxLayout* hLayout = new QHBoxLayout;

        QVBoxLayout* leftLayout = new QVBoxLayout;

        leftLayout->addWidget(dataEdit = new QTextEdit);
        dataEdit->setPlainText(data);

        QLabel* separatorLabel = new QLabel("<b>Separator</b>");
        leftLayout->addWidget(separatorLabel);

        QHBoxLayout* separatorLayout = new QHBoxLayout;

        separatorGroup = new QButtonGroup;

        QRadioButton* tabButton = new QRadioButton("Tab");
        separatorGroup->addButton(tabButton, TAB);
        separatorLayout->addWidget(tabButton);

        QRadioButton* spaceButton = new QRadioButton("Space");
        separatorGroup->addButton(spaceButton, SPACE);
        separatorLayout->addWidget(spaceButton);

        QRadioButton* commaButton = new QRadioButton("Comma");
        separatorGroup->addButton(commaButton, COMMA);
        separatorLayout->addWidget(commaButton);

        QRadioButton* otherButton = new QRadioButton("Other:");
        separatorGroup->addButton(otherButton, OTHER);
        separatorLayout->addWidget(otherButton);

        if(data.count(",") > data.count("\t"))
        {
            commaButton->setChecked(true);
        }
		else if (data.count('\t') > 0)
		{
			tabButton->setChecked(true);
		}
		else
        {
			spaceButton->setChecked(true);
        }

        separatorLayout->addWidget(otherSeparator = new QLineEdit(";"));
        otherSeparator->setEnabled(false);
        
        leftLayout->addLayout(separatorLayout);

        QLabel* otherOptionsLabel = new QLabel("<b>Other Options</b>");
        leftLayout->addWidget(otherOptionsLabel);        

        QHBoxLayout* otherOptionsLayout = new QHBoxLayout;

        otherOptionsLayout->addWidget(new QLabel("Start From Row:"));
        
        startRow = new QSpinBox;
        startRow->setMinimum(1);
        startRow->setValue(1);
        otherOptionsLayout->addWidget(startRow);

        QLabel* defaultValueLabel = new QLabel("Default Value:");
        otherOptionsLayout->addWidget(defaultValueLabel);

        defaultValue = new QLineEdit("0");
        if(dataType == DataType::DOUBLE)
        {
            defaultValue->setValidator(new QDoubleValidator);
        }
        else if (dataType == DataType::INT)
        {
            defaultValue->setValidator(new QIntValidator);
        }

        otherOptionsLayout->addWidget(defaultValue);

        leftLayout->addLayout(otherOptionsLayout);

        hLayout->addLayout(leftLayout);

        table = new QTableWidget;
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionMode(QAbstractItemView::NoSelection);
        hLayout->addWidget(table);

        layout->addLayout(hLayout);

        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(buttonBox);

        parent->setLayout(layout);
        parent->setWindowTitle("Import Data");

        QObject::connect(dataEdit, &QTextEdit::textChanged, parent, &::CDlgImportData::UpdateTable);
        QObject::connect(separatorGroup, &QButtonGroup::idClicked, parent, &::CDlgImportData::on_separatorGroup_idClicked);
        QObject::connect(otherSeparator, &QLineEdit::textChanged, parent, &::CDlgImportData::UpdateTable);
        QObject::connect(startRow, &QSpinBox::valueChanged, parent, &::CDlgImportData::UpdateTable);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, parent, &::CDlgImportData::accept);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, parent, &::CDlgImportData::reject);
    }

public:
    DataType dataType;
    int numCols;
    QList<QStringList> values;
    bool tooManyCols;
    bool badValue;
};

CDlgImportData::CDlgImportData(QString& data, DataType dataType, int numCols)
    : ui(new Ui::CDlgImportData)
{
    ui->setupUI(this, data, dataType, numCols);

    UpdateTable();
}

void CDlgImportData::SetDataType(DataType dataType)
{
    ui->dataType = dataType;
}

void CDlgImportData::SetNumCols(int numCols)
{
    ui->numCols = numCols;
}

QList<QStringList> CDlgImportData::GetValues()
{
    QList<QStringList> values;

    for(auto row : ui->values)
    {
        QStringList rowList;

        for(int col = 0; col < ui->numCols; col++)
        {
            QString value;
            if(col < row.size())
            {
                value = row[col];

                if(!validValue(value))
                {
                    value = ui->defaultValue->text();
                }
            }
            else
            {
                value = ui->defaultValue->text();
            }

            rowList.append(value);
        }

        values.append(rowList);
    }

    return values;
}

QList<QList<double> > CDlgImportData::GetDoubleValues()
{
	QList<QStringList> strVals = GetValues();

	QList<QList<double> > vals;
	for (QStringList& s : strVals)
	{
		QList<double> row;
		for (QString& si : s)
		{
			double a = si.toDouble();
			row.push_back(a);
		}
		vals.push_back(row);
	}
	return vals;
}

void CDlgImportData::calcValues()
{
    ui->values.clear();

    QString sep = "";

    switch (ui->separatorGroup->checkedId())
    {
    case TAB:
        sep = "\t";
        break;
    case SPACE:
        sep = " ";
        break;
    case COMMA:
        sep = ",";
        break;
    case OTHER:
    {
        sep = ui->otherSeparator->text();
        if(sep.isEmpty())
        {
            QMessageBox::critical(this, "Missing Separator", "Please enter a separator for your data.");
            return;
        }
        break;
    }
    default:
        return;
    }

    QStringList lines = ui->dataEdit->toPlainText().split("\n");

    if(ui->startRow->value() > lines.size())
    {
        return;
    }

    for(int line = ui->startRow->value() - 1; line < lines.size(); line++)
    {
        if(lines[line].trimmed().isEmpty()) continue;

        ui->values.append(lines[line].split(sep));
    }

}

bool CDlgImportData::validValue(QString value)
{
    bool okay;

    switch(ui->dataType)
    {
    case DataType::STRING:
        return true;
    case DataType::INT:
        value.toInt(&okay);
        return okay;
    case DataType::DOUBLE:
        value.toDouble(&okay);
        return okay;
    }

    return false;
}

void CDlgImportData::UpdateTable()
{
    calcValues();

    int cols = 0;
    for(auto row : ui->values)
    {
        if(row.size() > cols)
        {
            cols = row.size();
        }
    }

	if (ui->numCols == -1)
	{
		ui->numCols = cols;
	}

    if(cols < ui->numCols)
    {
        cols = ui->numCols;
    }

    ui->table->setColumnCount(cols);

    int rows = ui->values.size();
    ui->table->setRowCount(rows);

    ui->table->clear();

    ui->badValue = false;
    ui->tooManyCols = false;
    int row = 0;
    for(auto line : ui->values)
    {
        for(int col = 0; col < cols; col++)
        {
            QString value;
            if(col < line.size())
            {
                value = line[col];
            }
            else
            {
                value = ui->defaultValue->text();
            }

            QTableWidgetItem* item = new QTableWidgetItem(value);

            if(ui->numCols != -1 && ui->numCols < col + 1)
            {
                ui->tooManyCols = true;

                item->setBackground(Qt::red);
                item->setToolTip("Too many columns. These values will be truncated.");
            }
            else if(!validValue(value))
            {
                ui->badValue = true;

                item->setBackground(Qt::red);
                item->setToolTip(QString("This is not a valid %1.").arg(ui->dataType == DataType::INT ? "integer" : "number"));
            }

            ui->table->setItem(row, col, item);
        }

        row++;
    }
}

void CDlgImportData::accept()
{
    UpdateTable();

    QString err;

    if(ui->badValue)
    {
        err = QString("Some of your values are not valid %1. They will be "
            "replaced with your default value.").arg(ui->dataType == DataType::INT ? "integers" : "numbers");
    }

    if(ui->tooManyCols)
    {
        if(!err.isEmpty())
        {
            err += "\n\n";
        }

        err += QString("Your data must be %1 column(s) wide. Extra columns will "
            "be truncated").arg(ui->numCols);
    }

    if(!err.isEmpty())
    {
        err += "\n\nContinue?";

        auto response = QMessageBox::warning(this, "Error Parsing Data", err, QMessageBox::Ok | QMessageBox:: Cancel);

        if(response == QMessageBox::Cancel)
        {
            return;
        }
    }

    QDialog::accept();
}

void CDlgImportData::on_separatorGroup_idClicked(int id)
{
    ui->otherSeparator->setEnabled(id == OTHER);
    UpdateTable();
}