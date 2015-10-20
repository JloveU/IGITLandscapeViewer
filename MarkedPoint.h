#ifndef MARKEDPOINT_H
#define MARKEDPOINT_H


#include "MarkedObject.h"
#include <QVector3D>

class MarkedPoint : public MarkedObject
{

public:

    MarkedPoint();

    MarkedPoint(QString name);

    //���õ�
    void setPoint(ccGenericPointCloud* cloud, unsigned pointIndex);

    //inherited from cc2DLabel
    bool addPoint(ccGenericPointCloud* cloud, unsigned pointIndex);

    //inherited from cc2DLabel
    virtual void drawMeOnly3D(CC_DRAW_CONTEXT& context);

    //inherited from cc2DLabel
    virtual void drawMeOnly2D(CC_DRAW_CONTEXT& context);

    //��ȡ������
    QVector3D getPoint();

};


#endif // MARKEDPOINT_H
