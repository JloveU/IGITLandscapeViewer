#ifndef SETMARKEDPOINTTREEROOTNAMEDLG_H
#define SETMARKEDPOINTTREEROOTNAMEDLG_H


#include <QtGui/QDialog>
#include "ui_setMarkedPointTreeRootNameDlg.h"

class SetMarkedPointTreeRootNameDlg : public QDialog, public Ui::SetMarkedPointTreeRootNameDlg
{
    Q_OBJECT

public:

    SetMarkedPointTreeRootNameDlg(QWidget* parent);

    void setItems(QStringList items);

    void setName(const QString &name);

    QString getName();

};


#endif //SETMARKEDPOINTTREEROOTNAMEDLG_H
