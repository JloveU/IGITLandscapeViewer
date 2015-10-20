#ifndef BOOSTGRAPHINITTHREAD_H
#define BOOSTGRAPHINITTHREAD_H


#include <QThread>
#include "ShortestPathComputer.h"

//BGL图初始化子线程
class BoostGraphInitThread : public QThread
{

    Q_OBJECT

private:

    ShortestPathComputer::MyGraph *graph;
    ShortestPathComputer::Edge *edges;
    unsigned edgeNum;
    float *edgeWeights;
    unsigned vertexNum;

public:

    void setGraphData(ShortestPathComputer::MyGraph *graph, ShortestPathComputer::Edge *edges, unsigned edgeNum, float *edgeWeights, unsigned vertexNum);

protected:

    void run();

};


#endif // MARKEDLINE_H
