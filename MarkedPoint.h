#ifndef MARKEDPOINT_H
#define MARKEDPOINT_H


#include "MarkedObject.h"
#include <QVector3D>

class MarkedPoint : public MarkedObject
{

public:

    MarkedPoint(QString name = "UntitledPoint");

    //���õ�
    void setPoint(ccMesh* mesh, unsigned pointIndex);

    //inherited from cc2DLabel
    bool addPoint(ccGenericPointCloud* cloud, unsigned pointIndex);

    //inherited from cc2DLabel
    bool addPoint(ccMesh* mesh, unsigned pointIndex);

    //inherited from cc2DLabel
    virtual void drawMeOnly3D(CC_DRAW_CONTEXT& context);

    //inherited from cc2DLabel
    QStringList getLabelContent(int precision);

    //��ȡ������
    QVector3D getPoint();

    //Inherited from MarkedObject
    bool undo();

    //Inherited from MarkedObject
    bool redo();

};


#endif // MARKEDPOINT_H
