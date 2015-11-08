#ifndef MARKEDAREA_H
#define MARKEDAREA_H


#include "MarkedObject.h"
#include <QVector3D>
#include "ccMesh.h"
#include "ShortestPathComputer.h"
#include <QVector>

class MarkedArea : public MarkedObject
{

private:

    ShortestPathComputer *mShortestPathComputer;

    //��ȫ·����Ϊ��ʹ���߱պϣ�������ֹ������·����
    QVector<PickedPoint> mFillPathPoints;

    //�ڲ���
    QVector<PickedPoint> mInnerPoints;

    //�����ڲ�����������Ƭ������
    QVector<unsigned> mTriangleIndexs;

    //��ȫ·������
    float mFillPathLength;

    //�ܳ� - ��ȫ·������
    float mMarkedPathLength;

    //���
    float mArea;

    //���ĵ�
    CCVector3 mCenterPoint;

    //���ƽ�淨����
    CCVector3 mProjectPlaneNormal;

    //������ʷ��¼
    struct HistoryItem
    {
        //ĳ�β�����ĸ���Ա����״̬
        std::vector<PickedPoint> points; //m_points
        QVector<PickedPoint> fillPathPoints; //mFillPathPoints
        QVector<PickedPoint> innerPoints; //mInnerPoints
        QVector<unsigned> triangleIndexs; //mTriangleIndexs
        float fillPathLength; //mFillPathLength
        float markedPathLength; //mMarkedPathLength
        float area; //mArea
        CCVector3 centerPoint; //mCenterPoint
        CCVector3 projectPlaneNormal; //mProjectPlaneNormal

        HistoryItem()
            : points()
            , fillPathPoints()
            , innerPoints()
            , triangleIndexs()
            , fillPathLength(0.0)
            , markedPathLength(0.0)
            , area(0.0)
            , centerPoint()
            , projectPlaneNormal()
        {
        }

        HistoryItem(std::vector<PickedPoint> &_points
            , QVector<PickedPoint> &_fillLinePoints
            , QVector<PickedPoint> &_innerPoints
            , QVector<unsigned> &_triangleIndexs
            , float _fillPathLength
            , float _markedPathLength
            , float _area
            , CCVector3 &_centerPoint
            , CCVector3 &_projectPlaneNormal)
            : points(_points)
            , fillPathPoints(_fillLinePoints)
            , innerPoints(_innerPoints)
            , triangleIndexs(_triangleIndexs)
            , fillPathLength(_fillPathLength)
            , markedPathLength(_markedPathLength)
            , area(_area)
            , centerPoint(_centerPoint)
            , projectPlaneNormal(_projectPlaneNormal)
        {
        }
    };
    QVector<HistoryItem> mHistory; //��ʷ��¼
    int mCurrentPositionInHistory; //��ǰ״̬����ʷ��¼�е�λ�ã��±꣩

public:

    MarkedArea(QString name = "UntitledArea");

    //��ӿ��Ƶ�
    bool addPoint(ccMesh* mesh, unsigned pointIndex);

    //���
    void clear();

    //inherited from cc2DLabel
    virtual void drawMeOnly3D(CC_DRAW_CONTEXT& context);

    //inherited from cc2DLabel
    QStringList getLabelContent(int precision);

    //��ȡ���ĵ�����
    QVector3D getCenterPoint();

    //��ȡ���ƽ�淨����
    QVector3D getProjectPlaneNormal();

    //��ȡ�������ı߽��߳���
    float getBoundaryLength();

    //��ȡ�����������
    float getArea();

    //�������·��������
    void setShortestPathComputer(ShortestPathComputer *shortestPathComputer);

    //Inherited from MarkedObject
    bool undo();

    //Inherited from MarkedObject
    bool redo();

protected:

    //������������Ѱ�������ڲ��㣬����seedVertexΪ���������ڲ����һ���㣬�ɸõ㿪ʼ������������������õ�ʵ�ʲ����ڲ��㣨�����ܳɹ��ҵ������ڲ��㣩���򷵻�false�����򷵻�true
    bool findAllInnerPoints(const PickedPoint &seedVertex);

    void refreshBBox();

};


#endif // MARKEDAREA_H
