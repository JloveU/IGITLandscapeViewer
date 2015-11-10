#ifndef TABLEVIEWERDLG_H
#define TABLEVIEWERDLG_H


#include <QtGui/QDialog>
#include "ui_TableViewerDlg.h"
#include <QSqlTableModel>

class TableViewerDlg : public QDialog, public Ui::TableViewerDlg
{
    Q_OBJECT

    QSqlTableModel *mTableModel;

public:

    TableViewerDlg(QWidget* parent = NULL);

    void setTableModel(QSqlTableModel *model);

public slots:

    void addField();

    void deleteField();

};


#endif //TABLEVIEWERDLG_H
