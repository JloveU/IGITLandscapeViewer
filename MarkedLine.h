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

    ShortestPathComputer *mShortestPathComputer;

    float mLineLength;

    //������ʷ��¼
    struct HistoryItem
    {
        std::vector<PickedPoint> points; //ĳ�β������m_points
        float lineLength; //ĳ�β������mLineLength

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
    QVector<HistoryItem> mHistory; //��ʷ��¼
    int mCurrentPositionInHistory; //��ǰ״̬����ʷ��¼�е�λ�ã��±꣩

public:

    MarkedLine();

    MarkedLine(QString name);

    //��ӿ��Ƶ�
    bool addPoint(ccMesh* mesh, unsigned pointIndex);

    //���
    void clear();

    //inherited from cc2DLabel
    virtual void drawMeOnly3D(CC_DRAW_CONTEXT& context);

    //inherited from cc2DLabel
    QStringList getLabelContent(int precision);

    //��ȡ�������
    QVector3D getStartPoint();

    //��ȡ�յ�����
    QVector3D getEndPoint();

    //��ȡ����߳���
    float getLineLength();

    //�������·��������
    void setShortestPathComputer(ShortestPathComputer *shortestPathComputer);

    //Inherited from MarkedObject
    bool undo();

    //Inherited from MarkedObject
    bool redo();

};


#endif // MARKEDLINE_H
