#ifndef SETMARKEDPOINTPROPERTYDLG_H
#define SETMARKEDPOINTPROPERTYDLG_H


#include <QtGui/QDialog>
#include <QColor>
#include <QList>
#include "ui_setMarkedPointPropertyDlg.h"
#include "ccPointCloud.h"
#include "ccHObject.h"

class SetMarkedPointPropertyDlg : public QDialog, public Ui::SetMarkedPointPropertyDlg
{
    Q_OBJECT

private:

    QColor mMarkedPointColor;

    bool mColorSelectorLocked;

    ccHObject *mMarkedPointTreeRoot;

    QString mMarkedTypeName;

    QList<QColor> mColorList;

public:

    SetMarkedPointPropertyDlg(QWidget* parent);

    void setMarkedPointTreeRoot(ccHObject *markedPointTreeRoot);

private:

    void setMarkedTypeItems(const QStringList &items);

    void setMarkedTypeName(const QString &name);

    QString getMarkedTypeName();

    void setMarkedPointName(const QString &name);

    QString getMarkedPointName();

    void setMarkedPointColor(const QColor &color);

    QColor getMarkedPointColor();

    QColor gray2PseudoColor(float grayValue);

protected slots:

    void selectColor();

    void autoSetMarkedPointName(const QString &markedTypeName);

    void autoSetMarkedPointColor(const QString &markedTypeName);

    void checkInputAndCommit();

public slots:

    //inherited from QDialog
    void show();

signals:

    void committed(const QString &markedTypeName, const QString &markedPointName, const QColor &markedPointColor);

};


#endif //SETMARKEDPOINTPROPERTYDLG_H
