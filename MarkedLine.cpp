#include "MarkedLine.h"
#include "ccBox.h"
#include "ccGenericGLDisplay.h"

MarkedLine::MarkedLine(QString name)
    : MarkedObject(name)
    , mShortestPathComputer(NULL)
    , mLineLength(0.0)
    , mCurrentPositionInHistory(-1)
{
    mColor = QColor(255, 0, 0);
}

bool MarkedLine::addPoint(ccMesh* mesh, unsigned pointIndex)
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
        }
    }
    const unsigned pathSize = path.size();

    //历史纪录
    if (!mHistory.isEmpty() && mCurrentPositionInHistory < mHistory.size() - 1) //舍弃历史纪录中被撤销的操作
    {
        mHistory.remove(mCurrentPositionInHistory + 1, mHistory.size() - mCurrentPositionInHistory- 1);
    }
    if (!mHistory.isEmpty())
    {
        mHistory.insert(mHistory.end(), mHistory.back()); //复制最后一个状态
    }
    else
    {
        mHistory.resize(1);
    }
    for (int i = 0; i < pathSize; i++) //在复制得到的状态上增加本次要添加的点
    {
        mHistory.back().points.push_back(PickedPoint(mesh, path.at(i)));
    }
    mHistory.back().points.push_back(PickedPoint(mesh, pointIndex));
    mHistory.back().lineLength += pathLength;
    mCurrentPositionInHistory = mHistory.size() - 1;

    //更新m_points和mLineLength
    m_points = mHistory.back().points;
    mLineLength = mHistory.back().lineLength;

    //updateName();

    //we want to be notified whenever an associated cloud is deleted (in which case
    //we'll automatically clear the label)
    cloud->addDependency(this,DP_NOTIFY_OTHER_ON_DELETE);
    //we must also warn the cloud whenever we delete this label
    //addDependency(cloud,DP_NOTIFY_OTHER_ON_DELETE); //DGM: automatically done by the previous call to addDependency!

    refreshBBox();

    return true;
}

void MarkedLine::clear()
{
    m_points.clear();
    mLineLength = 0.0;
}

static CCVector3 staticBoxPointMarkerDimsBig(2.0, 2.0, 2.0);
static CCVector3 staticBoxPointMarkerDimsSmall(0.66, 0.66, 0.66);
static QColor staticPointMarkerColor(0, 255, 0); //顶点固定为绿色，顶点之间的连线颜色为mColor
static QSharedPointer<ccBox> staticUnitPointMarkerBig(0);
static QSharedPointer<ccBox> staticUnitPointMarkerSmall(0);
static ccColor::Rgb staticBBoxColor(255, 255, 0); //Bounding-box颜色

void MarkedLine::drawMeOnly3D(CC_DRAW_CONTEXT& context)
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

    //画Bounding-box
    if (isSelected())
    {
        mBBox.draw(staticBBoxColor);
    }

    ////画顶点之间的连线
	const float c_sizeFactor = 10.0f;
	bool loop = false;
    size_t count = m_points.size();
    //segment width
    glPushAttrib(GL_LINE_BIT);
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
    glEnd();
    glPopAttrib();

    //显示小球标记
    //display point marker as spheres
	{
		if (!staticUnitPointMarkerBig)
		{
			staticUnitPointMarkerBig = QSharedPointer<ccBox>(new ccBox(staticBoxPointMarkerDimsBig, 0, "PointMarker"));
            staticUnitPointMarkerBig->showColors(true);
			staticUnitPointMarkerBig->setVisible(true);
			staticUnitPointMarkerBig->setEnabled(true);
            staticUnitPointMarkerBig->setTempColor(ccColor::Rgb(staticPointMarkerColor.red(), staticPointMarkerColor.green(), staticPointMarkerColor.blue()));
		}
        if (!staticUnitPointMarkerSmall)
        {
            staticUnitPointMarkerSmall = QSharedPointer<ccBox>(new ccBox(staticBoxPointMarkerDimsSmall, 0, "PointMarker"));
            staticUnitPointMarkerSmall->showColors(true);
            staticUnitPointMarkerSmall->setVisible(true);
            staticUnitPointMarkerSmall->setEnabled(true);
            staticUnitPointMarkerSmall->setTempColor(ccColor::Rgb(staticPointMarkerColor.red(), staticPointMarkerColor.green(), staticPointMarkerColor.blue()));
        }
	
		//build-up point maker own 'context'
		CC_DRAW_CONTEXT markerContext = context;
		markerContext.flags &= (~CC_DRAW_ENTITY_NAMES); //we must remove the 'push name flag' so that the sphere doesn't push its own!
		markerContext._win = 0;

        //显示首尾端点处的小球（大）
        for (unsigned i = 0; i < count; i += (count == 1 ? 1 : count - 1))
        {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            const CCVector3* P = m_points[i].cloud->getPoint(m_points[i].index);
            ccGL::Translate(P->x,P->y,P->z);
            glScalef(context.labelMarkerSize,context.labelMarkerSize,context.labelMarkerSize);
            staticUnitPointMarkerBig->draw(markerContext);
            glPopMatrix();
        }

        //显示中间的小球（小）
        for (unsigned i = 1; i < count - 1; i++)
        {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            const CCVector3* P = m_points[i].cloud->getPoint(m_points[i].index);
            ccGL::Translate(P->x,P->y,P->z);
            glScalef(context.labelMarkerSize,context.labelMarkerSize,context.labelMarkerSize);
            staticUnitPointMarkerSmall->draw(markerContext);
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

QStringList MarkedLine::getLabelContent(int precision)
{
    //重定义要显示在2D标签上的内容
    QStringList body;

    //body << QString("Vertex number: %1").arg(size());

    body << QString("Length       : %1").arg(getLineLength(), 0, 'f', precision);

    QVector3D startPoint = getStartPoint();
    body << QString("Start point: (%1, %2, %3)").arg(startPoint.x(), 0, 'f', precision).arg(startPoint.y(), 0, 'f', precision).arg(startPoint.z(), 0, 'f', precision);

    QVector3D endPoint = getEndPoint();
    body << QString("End   point: (%1, %2, %3)").arg(endPoint.x(), 0, 'f', precision).arg(endPoint.y(), 0, 'f', precision).arg(endPoint.z(), 0, 'f', precision);

    return body;
}

QVector3D MarkedLine::getStartPoint()
{
    const PickedPoint pickedPoint = getPoint(0);
    CCVector3d point = pickedPoint.cloud->toGlobal3d(*pickedPoint.cloud->getPoint(pickedPoint.index));
    return QVector3D(point.x, point.y, point.z);
}

QVector3D MarkedLine::getEndPoint()
{
    const PickedPoint pickedPoint = getPoint(size() - 1);
    CCVector3d point = pickedPoint.cloud->toGlobal3d(*pickedPoint.cloud->getPoint(pickedPoint.index));
    return QVector3D(point.x, point.y, point.z);
}

float MarkedLine::getLineLength()
{
    return mLineLength;
}

void MarkedLine::setShortestPathComputer(ShortestPathComputer *shortestPathComputer)
{
    mShortestPathComputer = shortestPathComputer;
}

bool MarkedLine::undo()
{
    if (mCurrentPositionInHistory > 0)
    {
        mCurrentPositionInHistory--;
        m_points = mHistory.at(mCurrentPositionInHistory).points;
        mLineLength = mHistory.at(mCurrentPositionInHistory).lineLength;
        refreshBBox();
        return true;
    }
    else if (mCurrentPositionInHistory == 0)
    {
        mCurrentPositionInHistory--;
        m_points.clear();
        mLineLength = 0.0;
        refreshBBox();
        return true;
    }
    else
    {
        refreshBBox();
        return false;
    }
}

bool MarkedLine::redo()
{
    if (mCurrentPositionInHistory < mHistory.size() - 1)
    {
        mCurrentPositionInHistory++;
        m_points = mHistory.at(mCurrentPositionInHistory).points;
        mLineLength = mHistory.at(mCurrentPositionInHistory).lineLength;
        refreshBBox();
        return true;
    }
    else
    {
        refreshBBox();
        return false;
    }
}

bool MarkedLine::isValid()
{
    return (m_points.size() > 1);
}
