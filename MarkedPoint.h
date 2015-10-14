#ifndef MARKEDPOINT_H
#define MARKEDPOINT_H


#include "cc2DLabel.h"
#include "ccColorTypes.h"
#include "ccPointCloud.h"
#include <QColor>

class MarkedPoint : public cc2DLabel
{

private:

    //所属类别，即该点在树状列表中的父节点
    ccHObject *mMarkedType;

    QColor mColor;

public:

    MarkedPoint(QString name, ccGenericPointCloud* cloud, unsigned index);

    void setColor(const QColor &color);

    QColor getColor();

    void setMarkedType(ccHObject *markedType);

    ccHObject* getMarkedType();

    //inherited from cc2DLabel
    //forbid to add point
    bool addPoint(ccGenericPointCloud* cloud, unsigned pointIndex);

    //inherited from cc2DLabel
    virtual void drawMeOnly3D(CC_DRAW_CONTEXT& context);

    //inherited from cc2DLabel
    virtual void drawMeOnly2D(CC_DRAW_CONTEXT& context);

    //inherited from cc2DLabel
    inline virtual void setSelected(bool state);

};


#endif // MARKEDPOINT_H
