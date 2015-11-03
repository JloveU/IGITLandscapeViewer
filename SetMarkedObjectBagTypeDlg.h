#ifndef SETMARKEDOBJECTBAGTYPEDLG_H
#define SETMARKEDOBJECTBAGTYPEDLG_H


#include <QDialog>
#include <QButtonGroup>
#include "ui_setMarkedObjectBagTypeDlg.h"
#include "MarkedObjectBag.h"

class SetMarkedObjectBagTypeDlg : public QDialog, public Ui::SetMarkedObjectBagTypeDlg
{
    Q_OBJECT

private:

    QButtonGroup *mTypeRadioButtonGroup;

public:

    SetMarkedObjectBagTypeDlg(QWidget* parent);

    void setMarkedObjectBagType(const MarkedObjectBag::Type markedObjectBagType);

    MarkedObjectBag::Type getMarkedObjectBagType();

protected slots:

    void checkInputAndCommit();

signals:

    void committed(MarkedObjectBag::Type markedObjectBagType);

};


#endif //SETMARKEDOBJECTBAGTYPEDLG_H
