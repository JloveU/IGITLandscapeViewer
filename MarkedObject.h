#ifndef MARKEDOBJECT_H
#define MARKEDOBJECT_H


#include "cc2DLabel.h"
#include "ccColorTypes.h"
#include "ccPointCloud.h"
#include <QColor>
#include "ccBBox.h"

class MarkedObject : public cc2DLabel
{

protected:

    //������𣬼��õ�����״�б��еĸ��ڵ�
    ccHObject *mMarkedType;

    //��ɫ
    QColor mColor;

    //Bounding-box
    ccBBox mBBox;

protected:

    //����Bounding-box����������������Ĭ�ϸ���m_points���㣬������ݴ���Ĳ���points����
    void refreshBBox(const std::vector<PickedPoint> &points);
    virtual void refreshBBox();

public:

    MarkedObject(QString name = "UntitledObject");

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

    virtual const ccBBox& getBBox();
};


#endif // MARKEDOBJECT_H
