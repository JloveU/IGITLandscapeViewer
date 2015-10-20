#include "BoostGraphInitThread.h"

void BoostGraphInitThread::setGraphData(ShortestPathComputer::MyGraph *graph, ShortestPathComputer::Edge *edges, unsigned edgeNum, float *edgeWeights, unsigned vertexNum)
{
    this->graph = graph;
    this->edges = edges;
    this->edgeNum = edgeNum;
    this->edgeWeights = edgeWeights;
    this->vertexNum = vertexNum;
}

void BoostGraphInitThread::run()
{
    *graph = ShortestPathComputer::MyGraph(edges, edges + edgeNum, edgeWeights, vertexNum);
}
