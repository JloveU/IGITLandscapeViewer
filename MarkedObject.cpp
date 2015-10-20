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
    //�޸ĸ����ѡ�����ѡ�е���ʾЧ�����ǵ�ѡ�����ѡ�е�����Ϊ�Ƿ���ʾ2D��ǩ����������¾���ʾ3D��־��
    setDisplayedIn2D(state);
    cc2DLabel::setSelected(state);
}

QStringList MarkedObject::getLabelContent(int precision)
{
    //��ʹ��getLabelContent()
    //return QStringList();
    return cc2DLabel::getLabelContent(precision);
}
