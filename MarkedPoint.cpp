#include "MarkedPoint.h"
#include "ccSphere.h"
#include "ccGenericGLDisplay.h"

MarkedPoint::MarkedPoint(QString name)
    : MarkedObject(name)
{
    mColor = QColor(0, 0, 255);
}

void MarkedPoint::setPoint(ccMesh* mesh, unsigned pointIndex)
{
    m_points.clear();

    cc2DLabel::addPoint(mesh, pointIndex);
}

bool MarkedPoint::addPoint(ccGenericPointCloud* cloud, unsigned pointIndex)
{
    //forbid to add point
    assert(false);
    return false;
}

bool MarkedPoint::addPoint(ccMesh* mesh, unsigned pointIndex)
{
    //forbid to add point
    assert(false);
    return false;
}

static QSharedPointer<ccSphere> staticUnitPointMarker(0);

void MarkedPoint::drawMeOnly3D(CC_DRAW_CONTEXT& context)
{
    assert(!m_points.empty());
    assert(m_points.size() == 1);

	//standard case: list names pushing
	bool pushName = MACRO_DrawEntityNames(context);
	if (pushName)
	{
		//not particularily fast
		if (MACRO_DrawFastNamesOnly(context))
			return;
		glPushName(getUniqueIDForDisplay());
	}

	const float c_sizeFactor = 4.0f;
	bool loop = false;

	//display point marker as spheres
	{
		if (!staticUnitPointMarker)
		{
			staticUnitPointMarker = QSharedPointer<ccSphere>(new ccSphere(1.0f,0,"PointMarker",12));
            staticUnitPointMarker->showColors(true);
			staticUnitPointMarker->setVisible(true);
			staticUnitPointMarker->setEnabled(true);
		}
        staticUnitPointMarker->setTempColor(ccColor::Rgb(mColor.red(), mColor.green(), mColor.blue()));
	
		//build-up point maker own 'context'
		CC_DRAW_CONTEXT markerContext = context;
		markerContext.flags &= (~CC_DRAW_ENTITY_NAMES); //we must remove the 'push name flag' so that the sphere doesn't push its own!
		markerContext._win = 0;

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        const CCVector3* P = m_points[0].cloud->getPoint(m_points[0].index);
        ccGL::Translate(P->x,P->y,P->z);
        glScalef(context.labelMarkerSize,context.labelMarkerSize,context.labelMarkerSize);
        staticUnitPointMarker->draw(markerContext);
        glPopMatrix();
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
		const CCVector3* P = m_points[0].cloud->getPoint(m_points[0].index);
		QString title;
        title = getName(); //for single-point labels we prefer the name
		context._win->display3DLabel(	title,
										*P + CCVector3(	context.labelMarkerTextShift,
														context.labelMarkerTextShift,
														context.labelMarkerTextShift),
										ccColor::white.rgba/*,
										font*/ ); //DGM: I get an OpenGL error if the font is defined this way?!
        glPopAttrib();
	}

	if (pushName)
		glPopName();
}

QStringList MarkedPoint::getLabelContent(int precision)
{
    //重定义要显示在2D标签上的内容
    QStringList body;

    QVector3D point = getPoint();
    body << QString("Coordinate: (%1, %2, %3)").arg(point.x(), 0, 'f', precision).arg(point.y(), 0, 'f', precision).arg(point.z(), 0, 'f', precision);

    //测试
    //body << QString("Is boundary: %1").arg(m_points[0].mesh->isBoundaryVertex(m_points[0].index));

    return body;
}

QVector3D MarkedPoint::getPoint()
{
    const cc2DLabel::PickedPoint pickedPoint = cc2DLabel::getPoint(0);
    CCVector3d point = pickedPoint.cloud->toGlobal3d(*pickedPoint.cloud->getPoint(pickedPoint.index));
    return QVector3D(point.x, point.y, point.z);
}

bool MarkedPoint::undo()
{
    return false;
}

bool MarkedPoint::redo()
{
    return false;
}

