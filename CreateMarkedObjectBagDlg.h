#ifndef CREATEMARKEDOBJECTBAGDLG_H
#define CREATEMARKEDOBJECTBAGDLG_H


#include <QtGui/QDialog>
#include <QColor>
#include <QList>
#include "ui_createMarkedObjectBagDlg.h"
#include "ccPointCloud.h"
#include "ccHObject.h"
#include "MarkedObjectBag.h"
#include "ccDBRoot.h"

class CreateMarkedObjectBagDlg : public QDialog, public Ui::CreateMarkedObjectBagDlg
{
    Q_OBJECT

private:

    ccHObject *mMarkedObjectBagDBTreeParent;

    QButtonGroup *mRadioButtonGroupObjectBagType;

    QColor mMarkedObjectBagColor;

    bool mColorSelectorLocked;

    QList<QColor> mColorList;

    unsigned mCreatedObjectCount;

public:

    CreateMarkedObjectBagDlg(QWidget* parent);

    void setMarkedObjectBagDBTreeParent(ccHObject *parent);

    //ccHObject* getMarkedObjectBagDBTreeParent();

    void setMarkedObjectBagName(const QString &name);

    QString getMarkedObjectBagName();

    void setMarkedObjectBagType(const MarkedObjectBag::Type type);

    MarkedObjectBag::Type getMarkedObjectBagType();

    void setMarkedObjectBagColor(const QColor &color);

    QColor getMarkedObjectBagColor();

private:

    QColor gray2PseudoColor(float grayValue);

protected slots:

    void selectColor();

    void autoSetMarkedObjectBagName();

    void autoSetMarkedObjectBagType();

    void autoSetMarkedObjectBagColor();

    void checkInputAndCommit();

public slots:

    //inherited from QDialog
    int exec();

signals:

    void committed(const QString &name, const MarkedObjectBag::Type type, const QColor &color);

};


#endif //CREATEMARKEDOBJECTBAGDLG_H
