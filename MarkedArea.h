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

    //补全路径（为了使曲线闭合，连接起止点的最短路径）
    QVector<PickedPoint> mFillPathPoints;

    //内部点
    QVector<PickedPoint> mInnerPoints;

    //区域内部所有三角面片的索引
    QVector<unsigned> mTriangleIndexs;

    //补全路径长度
    float mFillPathLength;

    //周长 - 补全路径长度
    float mMarkedPathLength;

    //面积
    float mArea;

    //中心点
    CCVector3 mCenterPoint;

    //拟合平面法向量
    CCVector3 mProjectPlaneNormal;

    //操作历史纪录
    struct HistoryItem
    {
        //某次操作后的各成员变量状态
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
    QVector<HistoryItem> mHistory; //历史纪录
    int mCurrentPositionInHistory; //当前状态在历史记录中的位置（下标）

public:

    MarkedArea(QString name = "UntitledArea");

    //添加控制点
    bool addPoint(ccMesh* mesh, unsigned pointIndex);

    //清空
    void clear();

    //inherited from cc2DLabel
    virtual void drawMeOnly3D(CC_DRAW_CONTEXT& context);

    //inherited from cc2DLabel
    QStringList getLabelContent(int precision);

    //获取中心点坐标
    QVector3D getCenterPoint();

    //获取拟合平面法向量
    QVector3D getProjectPlaneNormal();

    //获取标记区域的边界线长度
    float getBoundaryLength();

    //获取标记区域的面积
    float getArea();

    //设置最短路径计算器
    void setShortestPathComputer(ShortestPathComputer *shortestPathComputer);

    //Inherited from MarkedObject
    bool undo();

    //Inherited from MarkedObject
    bool redo();

protected:

    //利用区域生长寻找所有内部点，输入seedVertex为假设属于内部点的一个点，由该点开始进行区域生长，如果该点实际不是内部点（即不能成功找到所有内部点），则返回false，否则返回true
    bool findAllInnerPoints(const PickedPoint &seedVertex);

    void refreshBBox();

};


#endif // MARKEDAREA_H
