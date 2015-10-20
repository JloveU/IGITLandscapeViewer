#ifndef MARKEDLINE_H
#define MARKEDLINE_H


#include "MarkedObject.h"
#include <QVector3D>
#include "ccMesh.h"
#include "ShortestPathComputer.h"
#include <QVector>

class MarkedLine : public MarkedObject
{

private:

    //名称（不同于ccObject的m_name，mMarkedName是原始输入的名称，m_name会在其后添加控制点数量等信息）
    QString mMarkedName;

    ShortestPathComputer *mShortestPathComputer;

    float mLineLength;

    //操作历史纪录
    struct HistoryItem
    {
        std::vector<PickedPoint> points; //某次操作后的m_points
        float lineLength; //某次操作后的mLineLength

        HistoryItem()
            : points()
            , lineLength(0.0)
        {
        }

        HistoryItem(std::vector<PickedPoint> &_points, float _lineLength)
            : points(_points)
            , lineLength(_lineLength)
        {
        }
    };
    QVector<HistoryItem> mHistory; //历史纪录
    int mCurrentPositionInHistory; //当前状态在历史记录中的位置（下标）

public:

    MarkedLine();

    MarkedLine(QString name);

    //添加控制点
    bool addPoint(ccMesh* mesh, unsigned pointIndex);

    //清空
    void clear();

    //inherited from cc2DLabel
    virtual void drawMeOnly3D(CC_DRAW_CONTEXT& context);

    //inherited from cc2DLabel
    virtual void drawMeOnly2D(CC_DRAW_CONTEXT& context);

    //inherited from cc2DLabel
    void updateName();

    //inherited from cc2DLabel
    QStringList getLabelContent(int precision);

    //获取起点坐标
    QVector3D getStartPoint();

    //获取终点坐标
    QVector3D getEndPoint();

    //获取标记线长度
    float getLineLength();

    //设置最短路径计算器
    void setShortestPathComputer(ShortestPathComputer *shortestPathComputer);

    //撤销一步
    void undo();

    //重做一步
    void redo();

};


#endif // MARKEDLINE_H
