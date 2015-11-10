#include "TableViewerDlg.h"
#include <QSqlQuery>
#include <qsqlrecord.h>
#include <qsqlfield.h>
#include <QMessageBox>
#include "MarkedObjectBag.h"
#include "TableNewFieldDlg.h"

TableViewerDlg::TableViewerDlg(QWidget* parent)
    : QDialog(parent)
    , Ui::TableViewerDlg()
{
    setupUi(this);

    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    //baseForm.view->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionBehavior(QAbstractItemView::SelectColumns);
    tableView->resizeColumnsToContents();
    tableView->setEditTriggers(QAbstractItemView::DoubleClicked);

    mTableModel = new QSqlTableModel();
    tableView->setModel(mTableModel);

    connect(buttonAddField, SIGNAL(clicked()), SLOT(addField()));
    connect(buttonDeleteField, SIGNAL(clicked()), SLOT(deleteField()));
}

void TableViewerDlg::setTableModel(QSqlTableModel *model)
{
    mTableModel = model;
    tableView->setModel(mTableModel);
    tableView->resizeColumnsToContents();
}

void TableViewerDlg::addField()
{
    TableNewFieldDlg dialog(this);
    if (!dialog.exec())
    {
        return;
    }

    // Header name of the property
    QString name = dialog.getName();

    // whether the property is already existed
    QSqlRecord record = mTableModel->record();
    for (int i = 0; i < mTableModel->columnCount(); i++)
    {
        QSqlField field = record.field(i);
        if (name.toUpper() == field.name().toUpper())
        {
            QMessageBox::warning(this,
                tr("Warning"),
                tr("Field has already existed!"));
            dialog.reject();
            return;
        }
    }

    // Length of the value type
    size_t length = dialog.getLength();

    // value type
    TableNewFieldDlg::ValueType valueType = dialog.getType();
    QString valueTypeString;
    switch (valueType)
    {
    case 0: 
        valueTypeString = QString("short interger(%1)").arg(length);
        break;
    case 1:
        valueTypeString = QString("interger(%1)").arg(length);
        break;
    case 2:
        valueTypeString = QString("char(%1)").arg(length);
        break;
    case 3:
        valueTypeString = QString("varchar(%1)").arg(length);
        break;
    case 4:
        valueTypeString = QString("float(%1)").arg(length);
        break;
    case 5:
        valueTypeString = QString("double(%1)").arg(length);
        break;
    case 6:
        valueTypeString = QString("text(%1)").arg(length);
        break;
    case 7:
        valueTypeString = QString("date(%1)").arg(length);
        break;
    default:
        assert(false);
        return;
    }

    QString queryStringAddColumn = QString("alter table %1 add column %2 %3").arg(mTableModel->tableName()).arg(name.toUpper()).arg(valueTypeString);
    QSqlQuery query;
    query.exec(queryStringAddColumn);

    mTableModel->setTable(mTableModel->tableName());
    mTableModel->select();
    tableView->resizeColumnsToContents();
}

void TableViewerDlg::deleteField()
{
    {
        //     bool s = mTableModel->removeColumns(1, 1); //此方法只将这些列的内容清空，但是并没有删除这些列对应的字段

        //     mTableModel->clear();
        //     QSqlQuery query;
        //     bool success = false;
        //     //success = query.exec(QString("alter table point drop column name"));
        //     QString tempTableName = mTableName + "_";
        //     success = query.exec(QString("create table %1 as select id from %2").arg(tempTableName).arg(mTableName));
        //     success = query.exec(QString("drop table if exists %1").arg(mTableName));
        //     success = query.exec(QString("alter table %1 rename to %2").arg(tempTableName).arg(mTableName));
        //     mTableModel->setTable(mTableName);
        //     mTableModel->select();
    }

    //获取所有列的名称
    QSqlRecord record = mTableModel->record();
    QStringList fieldNames;
    for (int i = 0; i < mTableModel->columnCount(); i++)
    {
        fieldNames << record.fieldName(i);
    }

    //获取当前选择的列的名称
    int selectedColumnIndex = tableView->currentIndex().column();
    QString selectedFiledName = record.fieldName(selectedColumnIndex);

    //如果当前选则的是只读列，则不能删除
    if (MarkedObjectBag::TABLE_READONLY_FIELD_NAMES.contains(selectedFiledName))
    {
        QMessageBox::warning(this,
            tr("Delete Current Field"),
            tr("This field can't be deleted because it's read-only!"));
        return;
    }

    //弹出窗口询问是否确认删除
    int ok = QMessageBox::warning(this,
        tr("Delete Current Field"),
        tr("Are you sure to delete current field?"),
        QMessageBox::Yes,
        QMessageBox::No);
    if(ok == QMessageBox::No)
    {
        return;
    }

    //将当前选择的列除外的所有列复制到一个新的表
    QStringList::iterator selectedPosition = qFind(fieldNames.begin(), fieldNames.end(), selectedFiledName);
    if (selectedPosition == fieldNames.end())
    {
        return;
    }
    fieldNames.erase(selectedPosition);
    QString namesToke = fieldNames.join(",");
    QString tableName = mTableModel->tableName();
    QString queryStringCreateNewTable = QString("create table tmp as select %1 from %2").arg(namesToke).arg(tableName);
    QSqlQuery query;
    assert(query.exec(queryStringCreateNewTable));

    mTableModel->clear();

    //删除原表
    QString queryStringDropOriginalTable = QString("drop table if exists %1").arg(tableName);
    assert(query.exec(queryStringDropOriginalTable));

    //将新表重命名为原表名
    QString queryStringRenameTable = QString("alter table tmp rename to %1").arg(tableName);
    assert(query.exec(queryStringRenameTable));

    //显示修改后的表
    mTableModel->setTable(tableName);
    mTableModel->select();
}
