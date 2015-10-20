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

    //初始化，生成BGL所需结构
    void init(ccMesh *mesh, QWidget *parentWidget = NULL);

    //获取两点之间的最短路径所经过的顶点列表（不包括首尾两个点），以及该路径的长度（可选）
    QList<unsigned> getShortestPathVertices(const unsigned startVertexIndex, const unsigned endVertexIndex, float *pathLength = NULL);

    //获取对应的mesh
    ccMesh* getAssociatedMesh();

};


#endif // MARKEDLINE_H
