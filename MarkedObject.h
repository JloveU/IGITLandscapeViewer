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

    virtual void setMarkedType(ccHObject *markedType);

    ccHObject* getMarkedType();

    virtual void setColor(const QColor &color); //ǰ���virtual�ؼ������ο���ʹ��������ָ��ǿ��ת���õ��ĸ���ָ����ø÷���ʱʵ�ʵ�������ͬ������

    QColor getColor();

    //inherited from cc2DLabel
    virtual inline void setSelected(bool state);

    //inherited from cc2DLabel
    void drawMeOnly2D(CC_DRAW_CONTEXT& context);

    //inherited from cc2DLabel
    virtual QStringList getLabelContent(int precision) = 0;

    //inherited from cc2DLabel
    //virtual bool move2D(int x, int y, int dx, int dy, int screenWidth, int screenHeight);

    //����һ��
    virtual bool undo() = 0;

    //����һ��
    virtual bool redo() = 0;
};


#endif // MARKEDOBJECT_H
