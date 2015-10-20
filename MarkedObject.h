#ifndef MARKEDOBJECT_H
#define MARKEDOBJECT_H


#include "cc2DLabel.h"
#include "ccColorTypes.h"
#include "ccPointCloud.h"
#include <QColor>

class MarkedObject : public cc2DLabel
{

protected:

    //所属类别，即该点在树状列表中的父节点
    ccHObject *mMarkedType;

    //名称（直接使用ccObject的m_name, setName() 和 getName()）
    //QString mMarkedName;
    static const QString DEFAULT_NAME;

    //颜色
    QColor mColor;

public:

    MarkedObject();

    MarkedObject(QString name);

    void setMarkedType(ccHObject *markedType);

    ccHObject* getMarkedType();

    void setColor(const QColor &color);

    QColor getColor();

    //inherited from cc2DLabel
    inline virtual void setSelected(bool state);

    //inherited from cc2DLabel
    QStringList getLabelContent(int precision);
};


#endif // MARKEDOBJECT_H
