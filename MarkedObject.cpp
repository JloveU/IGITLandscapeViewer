#include "MarkedObject.h"

const QString MarkedObject::DEFAULT_NAME = "Untitled";

MarkedObject::MarkedObject()
    : cc2DLabel(DEFAULT_NAME)
    , mMarkedType(NULL)
    , mColor(255, 0, 0)
{
    setSelected(true);
    setVisible(true);
}

MarkedObject::MarkedObject(QString name)
    : cc2DLabel(name)
    , mMarkedType(NULL)
    , mColor(255, 0, 0)
{
    setSelected(true);
    setVisible(true);
}

void MarkedObject::setColor(const QColor &color)
{
    mColor = color;
}

QColor MarkedObject::getColor()
{
    return mColor;
}

void MarkedObject::setMarkedType(ccHObject *markedType)
{
    mMarkedType = markedType;
}

ccHObject* MarkedObject::getMarkedType()
{
    return mMarkedType;
}

inline void MarkedObject::setSelected(bool state)
{
    //修改父类的选中与非选中的显示效果，是的选中与非选中的区别为是否显示2D标签（两种情况下均显示3D标志）
    setDisplayedIn2D(state);
    cc2DLabel::setSelected(state);
}

QStringList MarkedObject::getLabelContent(int precision)
{
    //不使用getLabelContent()
    //return QStringList();
    return cc2DLabel::getLabelContent(precision);
}
