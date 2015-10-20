#ifndef SHORTESTPATHCOMPUTER_H
#define SHORTESTPATHCOMPUTER_H


#include <boost/config.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>

#include "ccMesh.h"
#include <QWidget>

using namespace boost;

class ShortestPathComputer
{

public:

    typedef adjacency_list< listS, vecS, directedS, no_property, property<edge_weight_t, float> > MyGraph;
    typedef graph_traits<MyGraph>::vertex_descriptor VertexDescriptor;
    typedef std::pair<int, int> Edge;

private:

    ccMesh *mMesh;

    MyGraph mGraph;

public:

    ShortestPathComputer();

    //��ʼ��������BGL����ṹ
    void init(ccMesh *mesh, QWidget *parentWidget = NULL);

    //��ȡ����֮������·���������Ķ����б���������β�����㣩���Լ���·���ĳ��ȣ���ѡ��
    QList<unsigned> getShortestPathVertices(const unsigned startVertexIndex, const unsigned endVertexIndex, float *pathLength = NULL);

    //��ȡ��Ӧ��mesh
    ccMesh* getAssociatedMesh();

};


#endif // MARKEDLINE_H
