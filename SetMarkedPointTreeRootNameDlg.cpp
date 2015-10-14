#include "SetMarkedPointTreeRootNameDlg.h"

SetMarkedPointTreeRootNameDlg::SetMarkedPointTreeRootNameDlg(QWidget* parent)
    : QDialog(parent)
    , Ui::SetMarkedPointTreeRootNameDlg()
{
    setupUi(this);
}

void SetMarkedPointTreeRootNameDlg::setItems(QStringList items)
{
    nameComboBox->clear();
    nameComboBox->addItems(items);
}

void SetMarkedPointTreeRootNameDlg::setName(const QString &name)
{
    nameComboBox->setCurrentText(name);
}

QString SetMarkedPointTreeRootNameDlg::getName()
{
    return nameComboBox->currentText();
}
