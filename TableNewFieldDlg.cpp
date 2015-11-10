#include "TableNewFieldDlg.h"

TableNewFieldDlg::TableNewFieldDlg(QDialog *parent)
    : QDialog(parent)
{
	setupUi(this);

	comboBoxType->addItem("Short Integer");
	comboBoxType->addItem("Integer");
	comboBoxType->addItem("Char");
	comboBoxType->addItem("Varchar");
	comboBoxType->addItem("Float");
	comboBoxType->addItem("Double");
	comboBoxType->addItem("Text");
	comboBoxType->addItem("Date");
}

QString TableNewFieldDlg::getName()
{
    return lineEditName->text();
}

TableNewFieldDlg::ValueType TableNewFieldDlg::getType()
{
    return static_cast<ValueType>(comboBoxType->currentIndex());
}

size_t TableNewFieldDlg::getLength()
{
    return static_cast<size_t>(spinBoxLength->value());
}
