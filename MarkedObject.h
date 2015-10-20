#ifndef MARKEDOBJECT_H
#define MARKEDOBJECT_H


#include "cc2DLabel.h"
#include "ccColorTypes.h"
#include "ccPointCloud.h"
#include <QColor>

class MarkedObject : public cc2DLabel
{

protected:

    //������𣬼��õ�����״�б��еĸ��ڵ�
    ccHObject *mMarkedType;

    //���ƣ�ֱ��ʹ��ccObject��m_name, setName() �� getName()��
    //QString mMarkedName;
    static const QString DEFAULT_NAME;

    //��ɫ
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
