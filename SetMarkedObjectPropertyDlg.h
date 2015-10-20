#ifndef SETMARKEDOBJECTPROPERTYDLG_H
#define SETMARKEDOBJECTPROPERTYDLG_H


#include <QtGui/QDialog>
#include <QColor>
#include <QList>
#include "ui_setMarkedObjectPropertyDlg.h"
#include "ccPointCloud.h"
#include "ccHObject.h"

class SetMarkedObjectPropertyDlg : public QDialog, public Ui::SetMarkedObjectPropertyDlg
{
    Q_OBJECT

private:

    QColor mMarkedObjectColor;

    bool mColorSelectorLocked;

    ccHObject *mMarkedObjectTreeRoot;

    QString mMarkedTypeName;

    QList<QColor> mColorList;

public:

    SetMarkedObjectPropertyDlg(QWidget* parent);

    void setMarkedObjectTreeRoot(ccHObject *markedObjectTreeRoot);

private:

    void setMarkedTypeItems(const QStringList &items);

    void setMarkedTypeName(const QString &name);

    QString getMarkedTypeName();

    void setMarkedObjectName(const QString &name);

    QString getMarkedObjectName();

    void setMarkedObjectColor(const QColor &color);

    QColor getMarkedObjectColor();

    QColor gray2PseudoColor(float grayValue);

protected slots:

    void selectColor();

    void autoSetMarkedObjectName(const QString &markedTypeName);

    void autoSetMarkedObjectColor(const QString &markedTypeName);

    void checkInputAndCommit();

public slots:

    //inherited from QDialog
    void show();

signals:

    void committed(const QString &markedTypeName, const QString &markedObjectName, const QColor &markedObjectColor);

};


#endif //SETMARKEDOBJECTPROPERTYDLG_H
