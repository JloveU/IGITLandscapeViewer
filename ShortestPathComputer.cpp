#include "ShortestPathComputer.h"
#include "ccPointCloud.h"
#include <QProgressDialog>
#include <QApplication>
#include "BoostGraphInitThread.h"

ShortestPathComputer::ShortestPathComputer()
{
}

void ShortestPathComputer::init(ccMesh *mesh, QWidget *parentWidget/* = NULL*/)
{
    mMesh = mesh;
    ccGenericPointCloud *cloud = mesh->getAssociatedCloud();
    const unsigned vertexNum = cloud->size();
    const unsigned triangleNum = mesh->size();
    const unsigned edgeNum = triangleNum * 3; //半边

    Edge *edges = new Edge[edgeNum];
    float *edgeWeights = new float[edgeNum];
    const unsigned triangleEdgeVertexIndexs[3][2] = 
    {
        0, 1,
        1, 2,
        2, 0
    };
    unsigned edgeIndexBase, edgeIndex;
    mesh->placeIteratorAtBegining();
    for (unsigned i = 0; i < triangleNum; i++)
    {
        CCLib::TriangleSummitsIndexes* indexs = mesh->getNextTriangleIndexes();

        assert(indexs->i[0] < vertexNum && indexs->i[1] < vertexNum && indexs->i[2] < vertexNum);

        const CCVector3 *triangleVertices[3] = 
        {
            cloud->getPoint(indexs->i[0]),
            cloud->getPoint(indexs->i[1]),
            cloud->getPoint(indexs->i[2])
        };

        edgeIndexBase = i * 3;
        for (unsigned j = 0; j < 3; j++)
        {
            edgeIndex = edgeIndexBase + j;
            edges[edgeIndex].first = indexs->i[triangleEdgeVertexIndexs[j][0]];
            edges[edgeIndex].second = indexs->i[triangleEdgeVertexIndexs[j][1]];
            edgeWeights[edgeIndex] = CCVector3::vdistance(triangleVertices[triangleEdgeVertexIndexs[j][0]]->u, triangleVertices[triangleEdgeVertexIndexs[j][1]]->u);
        }
    }

    //开启新线程初始化BGL图并显示进度条
    BoostGraphInitThread thread;
    thread.setGraphData(&mGraph, edges, edgeNum, edgeWeights, vertexNum);
    thread.start();

    QProgressDialog progress;
    if (parentWidget)
    {
        progress.setParent(parentWidget);
    }
    progress.setWindowModality(Qt::WindowModal);
    progress.setWindowFlags(Qt::SubWindow | Qt::Popup);
    progress.setMinimumWidth(200);
    progress.setCancelButton(0);
    progress.setWindowTitle(QString::fromAscii("BGL图初始化"));
    progress.setLabelText(QString::fromAscii("BGL图初始化中，请稍后..."));
    progress.setRange(0, 0);
    progress.show();

    while (thread.isRunning())
    {
        QApplication::processEvents();
    }

    progress.close();

    //mGraph = MyGraph(edges, edges + edgeNum, edgeWeights, vertexNum);
}

QList<unsigned> ShortestPathComputer::getShortestPathVertices(const unsigned startVertexIndex, const unsigned endVertexIndex, float *pathLength/* = NULL*/)
{
    QList<unsigned> shortestPathVertices;

    VertexDescriptor startVertex = vertex(startVertexIndex, mGraph);
    std::vector<VertexDescriptor> predecessorMap(num_vertices(mGraph));
    std::vector<float> distanceMap(num_vertices(mGraph));
    dijkstra_shortest_paths(mGraph, startVertex,
        predecessor_map(boost::make_iterator_property_map(predecessorMap.begin(), get(boost::vertex_index, mGraph))).
        distance_map(boost::make_iterator_property_map(distanceMap.begin(), get(boost::vertex_index, mGraph))));

    if (pathLength)
    {
        *pathLength = distanceMap.at(endVertexIndex);
    }

    unsigned vertexIndex1 = endVertexIndex;
    unsigned vertexIndex2 = predecessorMap.at(vertexIndex1);
    while (vertexIndex2 != vertexIndex1)
    {
        shortestPathVertices.push_front(vertexIndex2);
        vertexIndex1 = vertexIndex2;
        vertexIndex2 = predecessorMap.at(vertexIndex1);
    }
    shortestPathVertices.pop_front(); //去掉起点

    return shortestPathVertices;
}

ccMesh* ShortestPathComputer::getAssociatedMesh()
{
    return mMesh;
}
