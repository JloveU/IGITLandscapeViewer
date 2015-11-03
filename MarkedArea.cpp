#include "MarkedArea.h"
#include "ccSphere.h"
#include "ccGenericGLDisplay.h"
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <math.h>

using namespace Eigen;

MarkedArea::MarkedArea()
    : MarkedObject()
    , mShortestPathComputer(NULL)
    , mFillPathLength(0.0)
    , mMarkedPathLength(0.0)
    , mArea(0.0)
    , mCurrentPositionInHistory(-1)
{
}

MarkedArea::MarkedArea(QString name)
    : MarkedObject(name)
    , mShortestPathComputer(NULL)
    , mFillPathLength(0.0)
    , mMarkedPathLength(0.0)
    , mArea(0.0)
    , mCurrentPositionInHistory(-1)
{
}

bool MarkedArea::addPoint(ccMesh* mesh, unsigned pointIndex)
{
    assert(mesh);
    assert(mShortestPathComputer);
    assert(mShortestPathComputer->getAssociatedMesh() == mesh);

    ccGenericPointCloud *cloud = mesh->getAssociatedCloud();
    assert(cloud && cloud->size() > pointIndex);

    //计算最短路径
    QList<unsigned> path;
    float pathLength;
    if (size() != 0)
    {
        assert(cloud == getPoint(size() - 1).cloud);
        if (pointIndex == getPoint(size() - 1).index) //如果本次要添加的点和上次添加的点相同，则不做任何操作
        {
            return true;
        }
        else
        {
            path = mShortestPathComputer->getShortestPathVertices(getPoint(size() - 1).index, pointIndex, &pathLength);
            mMarkedPathLength += pathLength;
        }
    }
    const unsigned pathSize = path.size();
    m_points.reserve(size() + pathSize + 1);
    for (unsigned i = 0; i < pathSize; i++)
    {
        m_points.push_back(PickedPoint(mesh, path.at(i)));
    }
    m_points.push_back(PickedPoint(mesh, pointIndex));

    //从第3次addPoint开始，每次都需计算补全路径、寻找内部点和寻找内部三角面片
    if (size() > 1)
    {
        //计算补全路径
        QList<unsigned> fillPath;
        fillPath = mShortestPathComputer->getShortestPathVertices(pointIndex, getPoint(0).index, &mFillPathLength);
        const unsigned fillPathSize = fillPath.size();
        mFillPathPoints.clear();
        mFillPathPoints.resize(fillPathSize);
        for (unsigned i = 0; i < fillPathSize; i++)
        {
            mFillPathPoints[i].mesh = mesh;
            mFillPathPoints[i].cloud = cloud;
            mFillPathPoints[i].index = fillPath.at(i);
        }

        //寻找内部点
        QVector<PickedPoint> boundaryVertices = QVector<PickedPoint>::fromStdVector(m_points);
        boundaryVertices += mFillPathPoints;
        //寻找一个内部点
        bool oneInnerPointFound = false;
        const unsigned boundaryVertexNum = boundaryVertices.size();
        for (unsigned boundaryVertexIndex = 0; boundaryVertexIndex < boundaryVertexNum; boundaryVertexIndex++)
        {
            const QVector<unsigned> neighborVertices = boundaryVertices[boundaryVertexIndex].mesh->getVertVertIndexs(boundaryVertices[boundaryVertexIndex].index);
            const unsigned neighborVertexNum = neighborVertices.size();
            for (unsigned neighborVertexIndex = 0; neighborVertexIndex < neighborVertexNum; neighborVertexIndex++)
            {
                unsigned neighborVertexRealIndex = neighborVertices[neighborVertexIndex];
                if (findAllInnerPoints(PickedPoint(mesh, neighborVertexRealIndex))) //如果该邻域点不在标记的区域内
                {
                    oneInnerPointFound = true;
                    break;
                }
            }
            if (oneInnerPointFound)
            {
                break;
            }
        }

        //寻找内部三角面片
        mTriangleIndexs.clear();
        const unsigned pointNum = m_points.size();
        const unsigned fillPathPointNum = mFillPathPoints.size();
        const unsigned innerPointNum = mInnerPoints.size();
        QVector<unsigned> allAreaPointIndexs(pointNum + fillPathPointNum + innerPointNum);
        for (unsigned i = 0; i < pointNum; i++)
        {
            allAreaPointIndexs[i] = m_points[i].index;
        }
        for (unsigned i = 0; i < fillPathPointNum; i++)
        {
            allAreaPointIndexs[pointNum + i] = mFillPathPoints[i].index;
        }
        for (unsigned i = 0; i < innerPointNum; i++)
        {
            allAreaPointIndexs[pointNum + fillPathPointNum + i] = mInnerPoints[i].index;
        }
        const unsigned allAreaPointNum = allAreaPointIndexs.size();
        for (unsigned allAreaPointIndex = 0; allAreaPointIndex < allAreaPointNum; allAreaPointIndex++)
        {
            const unsigned allAreaPointRealIndex = allAreaPointIndexs[allAreaPointIndex];
            const QVector<unsigned> neighborTriangleIndexs = mesh->getVertTriIndexs(allAreaPointRealIndex);
            const unsigned neighborTriangleNum = neighborTriangleIndexs.size();
            for (unsigned neighborTriangleIndex = 0; neighborTriangleIndex < neighborTriangleNum; neighborTriangleIndex++)
            {
                const unsigned neighborTriangleRealIndex = neighborTriangleIndexs[neighborTriangleIndex];
                if (mTriangleIndexs.contains(neighborTriangleRealIndex))
                {
                    continue;
                }
                const unsigned *triVertIndexs = mesh->getTriangleIndexes(neighborTriangleRealIndex)->i;
                if (allAreaPointIndexs.contains(triVertIndexs[0]) && allAreaPointIndexs.contains(triVertIndexs[1]) && allAreaPointIndexs.contains(triVertIndexs[2]))
                {
                    mTriangleIndexs.push_back(neighborTriangleRealIndex);
                }
            }
        }

        //计算面积
        mArea = 0.0;
        const unsigned triangleNum = mTriangleIndexs.size();
        if (triangleNum > 0)
        {
            ccMesh *mesh = m_points[0].mesh;
            ccGenericPointCloud *cloud = mesh->getAssociatedCloud();

            //计算质心
            mCenterPoint = CCVector3(0.0, 0.0, 0.0); //质心点
            for (unsigned allAreaPointIndex = 0; allAreaPointIndex < allAreaPointNum; allAreaPointIndex++)
            {
                const unsigned allAreaPointRealIndex = allAreaPointIndexs[allAreaPointIndex];
                mCenterPoint += *(cloud->getPoint(allAreaPointRealIndex));
            }
            mCenterPoint /= allAreaPointNum;

            //计算拟合平面法向量
            Matrix3f covarMat; //协方差矩阵
            covarMat.setZero(3, 3);
            Matrix3f tempMat;
            CCVector3 tempPoint;
            for (unsigned allAreaPointIndex = 0; allAreaPointIndex < allAreaPointNum; allAreaPointIndex++)
            {
                const unsigned allAreaPointRealIndex = allAreaPointIndexs[allAreaPointIndex];
                tempPoint = *(cloud->getPoint(allAreaPointRealIndex));
                tempPoint -= mCenterPoint;
                tempMat << tempPoint[0] * tempPoint[0], tempPoint[0] * tempPoint[1], tempPoint[0] * tempPoint[2],
                    tempPoint[1] * tempPoint[0], tempPoint[1] * tempPoint[1], tempPoint[1] * tempPoint[2],
                    tempPoint[2] * tempPoint[0], tempPoint[2] * tempPoint[1], tempPoint[2] * tempPoint[2];
                covarMat += tempMat;
            }
            covarMat /= allAreaPointNum;
            EigenSolver<Matrix3f> eigenSolver(covarMat, true);
            EigenSolver<Matrix3f>::EigenvalueType eigenvalues = eigenSolver.eigenvalues();
            EigenSolver<Matrix3f>::EigenvectorsType eigenvectors = eigenSolver.eigenvectors();
            float eigenvalueMin = eigenvalues(0).real();
            int eigenvalueMinIndex = 0;
            for(int i = 1; i < 3; i++)
            {
                if(eigenvalues(i).real() < eigenvalueMin)
                {
                    eigenvalueMin = eigenvalues(i).real();
                    eigenvalueMinIndex = i;
                }
            }
            mProjectPlaneNormal = CCVector3(eigenvectors(0, eigenvalueMinIndex).real(), eigenvectors(1, eigenvalueMinIndex).real(), eigenvectors(2, eigenvalueMinIndex).real()); //eigenvectors的每一列为一个特征向量

            //计算投影面积
            CCVector3 tempTriangleNormal;
            for (unsigned triangleIndex = 0; triangleIndex < triangleNum; triangleIndex++)
            {
                const unsigned triangleRealIndex = mTriangleIndexs[triangleIndex];
                const unsigned *triVertIndexs = mesh->getTriangleIndexes(triangleRealIndex)->i;
                const CCVector3 *triangleVertices[3] = 
                {
                    cloud->getPoint(triVertIndexs[0]),
                    cloud->getPoint(triVertIndexs[1]),
                    cloud->getPoint(triVertIndexs[2])
                };
                tempTriangleNormal = (*triangleVertices[1] - *triangleVertices[0]).cross(*triangleVertices[2] - *triangleVertices[0]);
                float cosValue = tempTriangleNormal.dot(mProjectPlaneNormal) / (tempTriangleNormal.norm() * mProjectPlaneNormal.norm());
                mArea += tempTriangleNormal.norm() * 0.5 * abs(cosValue);
            }
        }
    }

    //历史纪录
    if (!mHistory.isEmpty() && mCurrentPositionInHistory < mHistory.size() - 1) //舍弃历史纪录中被撤销的操作
    {
        mHistory.remove(mCurrentPositionInHistory + 1, mHistory.size() - mCurrentPositionInHistory- 1);
    }
    mHistory.resize(mHistory.size() + 1);
    mHistory.back().points = m_points;
    mHistory.back().fillPathPoints = mFillPathPoints;
    mHistory.back().innerPoints = mInnerPoints;
    mHistory.back().triangleIndexs = mTriangleIndexs;
    mHistory.back().fillPathLength = mFillPathLength;
    mHistory.back().markedPathLength = mMarkedPathLength;
    mHistory.back().area = mArea;
    mHistory.back().centerPoint = mCenterPoint;
    mHistory.back().projectPlaneNormal = mProjectPlaneNormal;
    mCurrentPositionInHistory = mHistory.size() - 1;

    //updateName();

    //we want to be notified whenever an associated cloud is deleted (in which case
    //we'll automatically clear the label)
    cloud->addDependency(this,DP_NOTIFY_OTHER_ON_DELETE);
    //we must also warn the cloud whenever we delete this label
    //addDependency(cloud,DP_NOTIFY_OTHER_ON_DELETE); //DGM: automatically done by the previous call to addDependency!

    return true;
}

void MarkedArea::clear()
{
    m_points.clear();
    mFillPathPoints.clear();
    mInnerPoints.clear();
    mTriangleIndexs.clear();
    mFillPathLength = 0.0;
    mMarkedPathLength = 0.0;
    mArea = 0.0;
}

//copied from cc2DLabel.cpp
//unit point marker
static QSharedPointer<ccSphere> c_unitPointMarker(0);

void MarkedArea::drawMeOnly3D(CC_DRAW_CONTEXT& context)
{
    assert(!m_points.empty());

	//standard case: list names pushing
	bool pushName = MACRO_DrawEntityNames(context);
	if (pushName)
	{
		//not particularily fast
		if (MACRO_DrawFastNamesOnly(context))
			return;
		glPushName(getUniqueIDForDisplay());
	}

    //画法线
    //glPushAttrib(GL_LINE_BIT);
    //glEnable(GL_LINE_SMOOTH);
    //glLineWidth(3.0);
    //ccGL::Color3v(ccColor::Rgba(mColor.red(), mColor.green(), mColor.blue(), mColor.alpha()).rgba);
    //glBegin(GL_LINES);
    //ccGL::Vertex3v(mCenterPoint.u);
    //ccGL::Vertex3v((mCenterPoint + mProjectPlaneNormal * 3).u);
    //glEnd();
    //glPopAttrib();


    //画区域内的所有三角形（半透明）
    const unsigned triangleNum = mTriangleIndexs.size();
    if (triangleNum > 0)
    {
        ccMesh *mesh = m_points[0].mesh;
        ccGenericPointCloud *cloud = mesh->getAssociatedCloud();
        glPushAttrib(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);
        //we draw the triangle
        glColor4ubv(ccColor::Rgba(mColor.red(), mColor.green(), mColor.blue(), 128).rgba);
        glBegin(GL_TRIANGLES);
        for (unsigned triangleIndex = 0; triangleIndex < triangleNum; triangleIndex++)
        {
            const unsigned triangleRealIndex = mTriangleIndexs[triangleIndex];
            const unsigned *triVertIndexs = mesh->getTriangleIndexes(triangleRealIndex)->i;
            ccGL::Vertex3v(cloud->getPoint(triVertIndexs[0])->u);
            ccGL::Vertex3v(cloud->getPoint(triVertIndexs[1])->u);
            ccGL::Vertex3v(cloud->getPoint(triVertIndexs[2])->u);
        }
        glEnd();
        glPopAttrib();
    }

    ////画控制点之间的连线（直线）
	const float c_sizeFactor = 10.0f;
	bool loop = false;
    size_t count = m_points.size();
    //segment width
    glPushAttrib(GL_LINE_BIT);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(c_sizeFactor);
    //we draw the segments
    ccGL::Color3v(ccColor::Rgba(mColor.red(), mColor.green(), mColor.blue(), mColor.alpha()).rgba);
    glBegin(GL_LINES);
    for (unsigned i=0; i<count; i++)
    {
        if (i+1<count || loop)
        {
            ccGL::Vertex3v(m_points[i].cloud->getPoint(m_points[i].index)->u);
            ccGL::Vertex3v(m_points[(i+1)%count].cloud->getPoint(m_points[(i+1)%count].index)->u);
        }
    }
    if (mFillPathPoints.size() > 0) //如果存在补全路径，则画之以形成闭合路径，否则直接将m_points的首尾相连（此时其首尾肯定是在mesh中就存在相连的一条边的）
    {
        ccGL::Vertex3v(m_points[count-1].cloud->getPoint(m_points[count-1].index)->u);
        ccGL::Vertex3v(mFillPathPoints[0].cloud->getPoint(mFillPathPoints[0].index)->u);
        unsigned fillPathPointsCount = mFillPathPoints.size();
        for (unsigned i = 0; i < fillPathPointsCount - 1; i++)
        {
            ccGL::Vertex3v(mFillPathPoints[i].cloud->getPoint(mFillPathPoints[i].index)->u);
            ccGL::Vertex3v(mFillPathPoints[i+1].cloud->getPoint(mFillPathPoints[i+1].index)->u);
        }
        ccGL::Vertex3v(mFillPathPoints[fillPathPointsCount-1].cloud->getPoint(mFillPathPoints[fillPathPointsCount-1].index)->u);
        ccGL::Vertex3v(m_points[0].cloud->getPoint(m_points[0].index)->u);
    }
    else
    {
        ccGL::Vertex3v(m_points[count-1].cloud->getPoint(m_points[count-1].index)->u);
        ccGL::Vertex3v(m_points[0].cloud->getPoint(m_points[0].index)->u);
    }
    glEnd();
    glPopAttrib();

    //显示小球标记
    //display point marker as spheres
	{
		if (!c_unitPointMarker)
		{
			c_unitPointMarker = QSharedPointer<ccSphere>(new ccSphere(1.0f,0,"PointMarker",12));
            c_unitPointMarker->showColors(true);
			c_unitPointMarker->setVisible(true);
			c_unitPointMarker->setEnabled(true);
		}
	
		//build-up point maker own 'context'
		CC_DRAW_CONTEXT markerContext = context;
		markerContext.flags &= (~CC_DRAW_ENTITY_NAMES); //we must remove the 'push name flag' so that the sphere doesn't push its own!
		markerContext._win = 0;

        //始终显示同一颜色，不管是否选中状态
        c_unitPointMarker->setTempColor(ccColor::Rgb(mColor.red(), mColor.green(), mColor.blue()));

        //显示首尾端点处的小球（大）
        c_unitPointMarker->setRadius(1.0);
        for (unsigned i = 0; i < count; i += (count == 1 ? 1 : count - 1))
        {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            const CCVector3* P = m_points[i].cloud->getPoint(m_points[i].index);
            ccGL::Translate(P->x,P->y,P->z);
            glScalef(context.labelMarkerSize,context.labelMarkerSize,context.labelMarkerSize);
            c_unitPointMarker->draw(markerContext);
            glPopMatrix();
        }

        //显示内部点处的小球（小）
        c_unitPointMarker->setRadius(0.3);
        const unsigned innerPointNum = mInnerPoints.size();
        for (unsigned i = 0; i < innerPointNum; i++)
        {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            const CCVector3* P = mInnerPoints[i].cloud->getPoint(mInnerPoints[i].index);
            ccGL::Translate(P->x,P->y,P->z);
            glScalef(context.labelMarkerSize,context.labelMarkerSize,context.labelMarkerSize);
            c_unitPointMarker->draw(markerContext);
            glPopMatrix();
        }
	}

	if (m_dispIn3D && !pushName) //no need to display label in point picking mode
	{
		//QFont font(context._win->getTextDisplayFont()); //takes rendering zoom into account!
		//font.setPointSize(font.pointSize()+2);
		//font.setBold(true);
		static const QChar ABC[3] = {'A','B','C'};

		//draw their name
		glPushAttrib(GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		for (unsigned j=0; j<count; j++)
		{
			const CCVector3* P = m_points[j].cloud->getPoint(m_points[j].index);
            //QString title = getName() + QString("#%0").arg(j);
            QString title = j == 0 ? getName() : "";
			context._win->display3DLabel(	title,
											*P + CCVector3(	context.labelMarkerTextShift,
															context.labelMarkerTextShift,
															context.labelMarkerTextShift),
											ccColor::white.rgba/*,
											font*/ ); //DGM: I get an OpenGL error if the font is defined this way?!
		}
        glPopAttrib();
	}

	if (pushName)
		glPopName();
}

QStringList MarkedArea::getLabelContent(int precision)
{
    //重定义要显示在2D标签上的内容
    QStringList body;

    //body << QString("Vertex number  : %1").arg(size() + mFillPathPoints.size());

    body << QString("Boundary length     : %1").arg(getBoundaryLength(), 0, 'f', precision);

    body << QString("Area                : %1").arg(getArea(), 0, 'f', precision);

    QVector3D centerPoint = getCenterPoint();
    body << QString("Center point        :(%1, %2, %3)").arg(centerPoint.x(), 0, 'f', precision).arg(centerPoint.y(), 0, 'f', precision).arg(centerPoint.z(), 0, 'f', precision);

    QVector3D projectPlaneNormal = getProjectPlaneNormal();
    body << QString("Project plane normal:(%1, %2, %3)").arg(projectPlaneNormal.x(), 0, 'f', precision).arg(projectPlaneNormal.y(), 0, 'f', precision).arg(projectPlaneNormal.z(), 0, 'f', precision);

    return body;
}

QVector3D MarkedArea::getCenterPoint()
{
    return QVector3D(mCenterPoint.x, mCenterPoint.y, mCenterPoint.z);
}

QVector3D MarkedArea::getProjectPlaneNormal()
{
    return QVector3D(mProjectPlaneNormal.x, mProjectPlaneNormal.y, mProjectPlaneNormal.z);
}

float MarkedArea::getBoundaryLength()
{
    return mMarkedPathLength + mFillPathLength;
}

float MarkedArea::getArea()
{
    return mArea;
}

void MarkedArea::setShortestPathComputer(ShortestPathComputer *shortestPathComputer)
{
    mShortestPathComputer = shortestPathComputer;
}

bool MarkedArea::undo()
{
    if (mCurrentPositionInHistory > 0)
    {
        mCurrentPositionInHistory--;
        m_points = mHistory.at(mCurrentPositionInHistory).points;
        mFillPathPoints = mHistory.at(mCurrentPositionInHistory).fillPathPoints;
        mInnerPoints = mHistory.at(mCurrentPositionInHistory).innerPoints;
        mTriangleIndexs = mHistory.at(mCurrentPositionInHistory).triangleIndexs;
        mFillPathLength = mHistory.at(mCurrentPositionInHistory).fillPathLength;
        mMarkedPathLength = mHistory.at(mCurrentPositionInHistory).markedPathLength;
        mArea = mHistory.at(mCurrentPositionInHistory).area;
        mCenterPoint = mHistory.at(mCurrentPositionInHistory).centerPoint;
        mProjectPlaneNormal = mHistory.at(mCurrentPositionInHistory).projectPlaneNormal;
        return true;
    }
    else if (mCurrentPositionInHistory == 0)
    {
        mCurrentPositionInHistory--;
        m_points.clear();
        mFillPathPoints.clear();
        mInnerPoints.clear();
        mTriangleIndexs.clear();
        mFillPathLength = 0.0;
        mMarkedPathLength = 0.0;
        mArea = 0.0;
        mCenterPoint = CCVector3();
        mProjectPlaneNormal = CCVector3();
        return true;
    }
    else
    {
        return false;
    }
}

bool MarkedArea::redo()
{
    if (mCurrentPositionInHistory < mHistory.size() - 1)
    {
        mCurrentPositionInHistory++;
        m_points = mHistory.at(mCurrentPositionInHistory).points;
        mFillPathPoints = mHistory.at(mCurrentPositionInHistory).fillPathPoints;
        mInnerPoints = mHistory.at(mCurrentPositionInHistory).innerPoints;
        mTriangleIndexs = mHistory.at(mCurrentPositionInHistory).triangleIndexs;
        mFillPathLength = mHistory.at(mCurrentPositionInHistory).fillPathLength;
        mMarkedPathLength = mHistory.at(mCurrentPositionInHistory).markedPathLength;
        mArea = mHistory.at(mCurrentPositionInHistory).area;
        mCenterPoint = mHistory.at(mCurrentPositionInHistory).centerPoint;
        mProjectPlaneNormal = mHistory.at(mCurrentPositionInHistory).projectPlaneNormal;
        return true;
    }
    else
    {
        return false;
    }
}

bool MarkedArea::findAllInnerPoints(const PickedPoint &seedVertex)
{
    //所有边界点
    QVector<unsigned> boundaryVertexIndexs(m_points.size() + mFillPathPoints.size());
    const unsigned pointNum = m_points.size();
    for (unsigned i = 0; i < pointNum; i++)
    {
        boundaryVertexIndexs[i] = m_points[i].index;
    }
    const unsigned fillPathPointNum = mFillPathPoints.size();
    for (unsigned i = 0; i < fillPathPointNum; i++)
    {
        boundaryVertexIndexs[pointNum + i] = mFillPathPoints[i].index;
    }

    //初始化visited
    const unsigned vertexNum = m_points[0].cloud->size();
    QVector<bool> visited(vertexNum);
    visited.fill(false);

    //清空mInnerPoints
    mInnerPoints.clear();

    //添加种子点
    visited[seedVertex.index] = true;
    mInnerPoints.push_back(seedVertex);

    ccMesh *mesh = seedVertex.mesh;

    QList<unsigned> seeds; //种子点列表
    seeds.push_back(seedVertex.index);
    while (!seeds.empty())
    {
        unsigned vertexIndex = seeds.front();
        seeds.pop_front();
        const QVector<unsigned> neighborVertices = mesh->getVertVertIndexs(vertexIndex);
        const unsigned neighborVertexNum = neighborVertices.size();
        for (unsigned neighborVertexIndex = 0; neighborVertexIndex < neighborVertexNum; neighborVertexIndex++)
        {
            unsigned neighborVertexRealIndex = neighborVertices[neighborVertexIndex];
            if (boundaryVertexIndexs.contains(neighborVertexRealIndex))
            {
                continue;
            }
            if (visited[neighborVertexRealIndex])
            {
                continue;
            }
            if (mesh->isBoundaryVertex(neighborVertexRealIndex))
            {
                mInnerPoints.clear();
                return false;
            }
            visited[neighborVertexRealIndex] = true;
            mInnerPoints.push_back(PickedPoint(mesh, neighborVertexRealIndex));
            seeds.push_back(neighborVertexRealIndex);
        }
    }

    return true;
}
