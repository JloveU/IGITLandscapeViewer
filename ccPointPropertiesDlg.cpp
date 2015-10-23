//##########################################################################
//#                                                                        #
//#                            CLOUDCOMPARE                                #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; version 2 of the License.               #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#          COPYRIGHT: EDF R&D / TELECOM ParisTech (ENST-TSI)             #
//#                                                                        #
//##########################################################################

#include "ccPointPropertiesDlg.h"

//Local
#include "ccCommon.h"
#include "ccGLWindow.h"
#include "ccGuiParameters.h"

//qCC_db
#include <ccLog.h>
#include <ccPointCloud.h>
#include <cc2DViewportLabel.h>
#include <cc2DLabel.h>

//CCLib
#include <ScalarField.h>

//Qt
#include <QInputDialog>

//System
#include <assert.h>

#include "ccDBRoot.h"

#include <QMessageBox>
#include <QProgressDialog>
#include "ccProgressDialog.h"

#include "MarkedObject.h"

ccPointPropertiesDlg::ccPointPropertiesDlg(QWidget* parent, ccDBRoot *ccRoot)
	: ccPointPickingGenericInterface(parent)
	, Ui::PointPropertiesDlg()
	, m_pickingMode(POINT_INFO)
    , m_ccRoot(ccRoot)
    , mMarkedPointTreeRoot(NULL)
    , mSetMarkedObjectPropertyDlg(NULL)
    , mCurrentMarkedPoint(NULL)
    , mCurrentMarkedLine(NULL)
    , mLastMarkedObject(NULL)
    , mShortestPathComputer(NULL)
    , mCurrentMarkedArea(NULL)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint |Qt::Tool);

    connect(markPointButton,		    SIGNAL(clicked()), this, SLOT(onActivatePointMarking()));
    connect(markLineButton,		        SIGNAL(clicked()), this, SLOT(onActivateLineMarking()));
    connect(markAreaButton,	  	        SIGNAL(clicked()), this, SLOT(onActivateAreaMarking()));
    connect(markUndoButton,	  	        SIGNAL(clicked()), this, SLOT(onMarkUndo()));
    connect(markRedoButton,	  	        SIGNAL(clicked()), this, SLOT(onMarkRedo()));
    connect(markDoneButton,	  	        SIGNAL(clicked()), this, SLOT(onMarkDone()));
	connect(closeButton,				SIGNAL(clicked()), this, SLOT(onClose()));
	connect(pointPropertiesButton,		SIGNAL(clicked()), this, SLOT(activatePointPropertiesDisplay()));
	connect(pointPointDistanceButton,	SIGNAL(clicked()), this, SLOT(activateDistanceDisplay()));
	connect(pointsAngleButton,			SIGNAL(clicked()), this, SLOT(activateAngleDisplay()));
	connect(rectZoneToolButton,			SIGNAL(clicked()), this, SLOT(activate2DZonePicking()));
	connect(saveLabelButton,			SIGNAL(clicked()), this, SLOT(exportCurrentLabel()));
	connect(razButton,					SIGNAL(clicked()), this, SLOT(initializeState()));

	//for points picking
	m_label = new cc2DLabel();
	m_label->setSelected(true);

	//for 2D zone picking
	m_rect2DLabel = new cc2DViewportLabel();
	m_rect2DLabel->setVisible(false);	//=invalid
	m_rect2DLabel->setSelected(true);	//=closed

    //隐藏不使用的按钮
    pointPropertiesButton->setVisible(false);
    pointPointDistanceButton->setVisible(false);
    pointsAngleButton->setVisible(false);
    rectZoneToolButton->setVisible(false);
    saveLabelButton->setVisible(false);
    razButton->setVisible(false);
}

ccPointPropertiesDlg::~ccPointPropertiesDlg()
{
	if (m_label)
		delete m_label;
	m_label = 0;

	if (m_rect2DLabel)
		delete m_rect2DLabel;
	m_rect2DLabel = 0;

    if (mSetMarkedObjectPropertyDlg)
        delete mSetMarkedObjectPropertyDlg;
    mSetMarkedObjectPropertyDlg = 0;
}

bool ccPointPropertiesDlg::linkWith(ccGLWindow* win)
{
	assert(m_label && m_rect2DLabel);

	ccGLWindow* oldWin = m_associatedWin;

	if (!ccPointPickingGenericInterface::linkWith(win))
		return false;

	//old window?
	if (oldWin)
	{
		oldWin->removeFromOwnDB(m_label);
		oldWin->removeFromOwnDB(m_rect2DLabel);
		oldWin->setInteractionMode(ccGLWindow::TRANSFORM_CAMERA);
		disconnect(oldWin, SIGNAL(mouseMoved(int,int,Qt::MouseButtons)), this, SLOT(update2DZone(int,int,Qt::MouseButtons)));
		disconnect(oldWin, SIGNAL(leftButtonClicked(int,int)), this, SLOT(processClickedPoint(int,int)));
		disconnect(oldWin, SIGNAL(buttonReleased()), this, SLOT(close2DZone()));
	}

	m_rect2DLabel->setVisible(false);	//=invalid
	m_rect2DLabel->setSelected(true);	//=closed
	m_label->clear();

	//new window?
	if (m_associatedWin)
	{
		m_associatedWin->addToOwnDB(m_label);
		m_associatedWin->addToOwnDB(m_rect2DLabel);
		connect(m_associatedWin, SIGNAL(mouseMoved(int,int,Qt::MouseButtons)), this, SLOT(update2DZone(int,int,Qt::MouseButtons)));
		connect(m_associatedWin, SIGNAL(leftButtonClicked(int,int)), this, SLOT(processClickedPoint(int,int)));
		connect(m_associatedWin, SIGNAL(buttonReleased()), this, SLOT(close2DZone()));
	}

	return true;
}

bool ccPointPropertiesDlg::start()
{
	onActivatePointMarking();

	return ccPointPickingGenericInterface::start();
}

void ccPointPropertiesDlg::stop(bool state)
{
	assert(m_label && m_rect2DLabel);

	m_label->clear();
	m_rect2DLabel->setVisible(false);	//=invalid
	m_rect2DLabel->setSelected(true);	//=closed

	if (m_associatedWin)
		m_associatedWin->setInteractionMode(ccGLWindow::TRANSFORM_CAMERA);

	ccPointPickingGenericInterface::stop(state);
}

void ccPointPropertiesDlg::onClose()
{
    onCancelLineMarking();
    onCancelAreaMarking();

    //将所有标记物体的2D标签隐藏
    if (mCurrentMarkedPoint)
    {
        mCurrentMarkedPoint->setSelected(false);
    }
    if (mCurrentMarkedLine)
    {
        mCurrentMarkedLine->setSelected(false);
    }
    if (mCurrentMarkedArea)
    {
        mCurrentMarkedArea->setSelected(false);
    }
    if (mLastMarkedObject)
    {
        mLastMarkedObject->setSelected(false);
    }

    if (m_associatedWin)
    {
        m_associatedWin->updateGL();
    }

    stop(false);
}

void ccPointPropertiesDlg::onActivatePointMarking()
{
    m_pickingMode = MARK_POINT;
    markPointButton->setDown(true);
    markLineButton->setDown(false);
    markAreaButton->setDown(false);

    onCancelLineMarking();
    onCancelAreaMarking();

    if (mLastMarkedObject)
    {
        mLastMarkedObject->setSelected(false);
    }

    if (m_associatedWin)
    {
        m_associatedWin->setInteractionMode(ccGLWindow::TRANSFORM_CAMERA);
        m_associatedWin->updateGL();
    }

    mCurrentMarkedPoint = new MarkedPoint();
}

void ccPointPropertiesDlg::onActivateLineMarking()
{
    m_pickingMode = MARK_LINE;
    markPointButton->setDown(false);
    markLineButton->setDown(true);
    markAreaButton->setDown(false);

    onCancelAreaMarking();

    if (mLastMarkedObject)
    {
        mLastMarkedObject->setSelected(false);
    }

    if (m_associatedWin)
    {
        m_associatedWin->setInteractionMode(ccGLWindow::TRANSFORM_CAMERA);
        m_associatedWin->updateGL();
    }

    mCurrentMarkedLine = new MarkedLine();
}

void ccPointPropertiesDlg::onActivateAreaMarking()
{
    m_pickingMode = MARK_AREA;
    markPointButton->setDown(false);
    markLineButton->setDown(false);
    markAreaButton->setDown(true);

    onCancelLineMarking();

    if (mLastMarkedObject)
    {
        mLastMarkedObject->setSelected(false);
    }

    if (m_associatedWin)
    {
        m_associatedWin->setInteractionMode(ccGLWindow::TRANSFORM_CAMERA);
        m_associatedWin->updateGL();
    }

    mCurrentMarkedArea = new MarkedArea();
}

void ccPointPropertiesDlg::onMarkUndo()
{
    switch(m_pickingMode)
    {
    case MARK_LINE:
        assert(mCurrentMarkedLine);
        mCurrentMarkedLine->undo();
        if (m_associatedWin)
        {
            m_associatedWin->redraw();
        }
        break;
    case MARK_AREA:
        assert(mCurrentMarkedArea);
        mCurrentMarkedArea->undo();
        if (m_associatedWin)
        {
            m_associatedWin->redraw();
        }
        break;
    }
}

void ccPointPropertiesDlg::onMarkRedo()
{
    switch(m_pickingMode)
    {
    case MARK_LINE:
        assert(mCurrentMarkedLine);
        mCurrentMarkedLine->redo();
        if (m_associatedWin)
        {
            m_associatedWin->redraw();
        }
        break;
    case MARK_AREA:
        assert(mCurrentMarkedArea);
        mCurrentMarkedArea->redo();
        if (m_associatedWin)
        {
            m_associatedWin->redraw();
        }
        break;
    }
}

void ccPointPropertiesDlg::onCancelLineMarking()
{
    if (!mCurrentMarkedLine)
    {
        return;
    }

    if (m_associatedWin)
    {
        //取消显示该MarkedLine
        m_associatedWin->removeFromOwnDB(mCurrentMarkedLine);
        m_associatedWin->redraw();
    }

    mCurrentMarkedLine->clear();
}

void ccPointPropertiesDlg::onCancelAreaMarking()
{
    if (!mCurrentMarkedArea)
    {
        return;
    }

    if (m_associatedWin)
    {
        //取消显示该MarkedLine
        m_associatedWin->removeFromOwnDB(mCurrentMarkedArea);
        m_associatedWin->redraw();
    }

    mCurrentMarkedArea->clear();
}

void ccPointPropertiesDlg::activatePointPropertiesDisplay()
{
	if (m_associatedWin)
		m_associatedWin->setInteractionMode(ccGLWindow::TRANSFORM_CAMERA);

	m_pickingMode = POINT_INFO;
    pointPropertiesButton->setDown(true);
	pointPointDistanceButton->setDown(false);
	pointsAngleButton->setDown(false);
	rectZoneToolButton->setDown(false);
	m_label->setVisible(true);
	m_rect2DLabel->setVisible(false);
}

void ccPointPropertiesDlg::activateDistanceDisplay()
{
	m_pickingMode = POINT_POINT_DISTANCE;
	pointPropertiesButton->setDown(false);
	pointPointDistanceButton->setDown(true);
	pointsAngleButton->setDown(false);
	rectZoneToolButton->setDown(false);
	m_label->setVisible(true);
	m_rect2DLabel->setVisible(false);

	if (m_associatedWin)
	{
		m_associatedWin->setInteractionMode(ccGLWindow::TRANSFORM_CAMERA);
		m_associatedWin->updateGL();
	}
}

void ccPointPropertiesDlg::activateAngleDisplay()
{
	m_pickingMode = POINTS_ANGLE;
	pointPropertiesButton->setDown(false);
	pointPointDistanceButton->setDown(false);
	pointsAngleButton->setDown(true);
	rectZoneToolButton->setDown(false);
	m_label->setVisible(true);
	m_rect2DLabel->setVisible(false);

	if (m_associatedWin)
	{
		m_associatedWin->setInteractionMode(ccGLWindow::TRANSFORM_CAMERA);
		m_associatedWin->updateGL();
	}
}

void ccPointPropertiesDlg::activate2DZonePicking()
{
	m_pickingMode = RECT_ZONE;
	pointPropertiesButton->setDown(false);
	pointPointDistanceButton->setDown(false);
	pointsAngleButton->setDown(false);
	rectZoneToolButton->setDown(true);
	m_label->setVisible(false);
	//m_rect2DLabel->setVisible(false);

	if (m_associatedWin)
	{
		m_associatedWin->setInteractionMode(ccGLWindow::SEGMENT_ENTITY);
		m_associatedWin->updateGL();
	}
}

void ccPointPropertiesDlg::initializeState()
{
	assert(m_label && m_rect2DLabel);
	m_label->clear();
	m_rect2DLabel->setVisible(false);	//=invalid
	m_rect2DLabel->setSelected(true);	//=closed

	if (m_associatedWin)
		m_associatedWin->redraw();
}

void ccPointPropertiesDlg::exportCurrentLabel()
{
	ccHObject* labelObject = 0;
	if (m_pickingMode == RECT_ZONE)
		labelObject = (m_rect2DLabel->isSelected() && m_rect2DLabel->isVisible() ? m_rect2DLabel : 0);
	else
		labelObject = (m_label && m_label->size()>0 ? m_label : 0);
	if (!labelObject)
		return;

	//detach current label from window
	if (m_associatedWin)
		m_associatedWin->removeFromOwnDB(labelObject);
	labelObject->setSelected(false);

	ccHObject* newLabelObject = 0;
	if (m_pickingMode == RECT_ZONE)
	{
		//if (m_associatedWin)
		//	m_rect2DLabel->setParameters(m_associatedWin->getViewportParameters());
		newLabelObject = m_rect2DLabel = new cc2DViewportLabel();
		m_rect2DLabel->setVisible(false);	//=invalid
		m_rect2DLabel->setSelected(true);	//=closed
	}
	else
	{
		//attach old label to first point cloud by default
		m_label->getPoint(0).cloud->addChild(labelObject);
		newLabelObject = m_label = new cc2DLabel();
		m_label->setSelected(true);
	}

	emit newLabel(labelObject);

	if (m_associatedWin)
	{
		m_associatedWin->addToOwnDB(newLabelObject);
		m_associatedWin->redraw();
	}	
}

void ccPointPropertiesDlg::processPickedPoint(ccPointCloud* cloud, unsigned pointIndex, int x, int y)
{
	assert(cloud);
	assert(m_label);
	assert(m_associatedWin);

	switch(m_pickingMode)
	{
	case POINT_INFO:
		m_label->clear();
		break;
	case POINT_POINT_DISTANCE:
		if (m_label->size() >= 2)
			m_label->clear();
		break;
	case POINTS_ANGLE:
		if (m_label->size() >= 3)
			m_label->clear();
		break;
	case RECT_ZONE:
		return; //we don't use this slot for 2D mode
	}

	m_label->addPoint(cloud,pointIndex);
	m_label->setVisible(true);
	if (m_label->size()==1 && m_associatedWin)
		m_label->setPosition((float)(x+20)/(float)m_associatedWin->width(),(float)(y+20)/(float)m_associatedWin->height());

	//output info to Console
	QStringList body = m_label->getLabelContent(ccGui::Parameters().displayedNumPrecision);
	ccLog::Print(QString("[Picked] ")+m_label->getName());
	for (int i=0;i<body.size();++i)
		ccLog::Print(QString("[Picked]\t- ")+body[i]);

	if (m_associatedWin)
		m_associatedWin->redraw();
}

void ccPointPropertiesDlg::processClickedPoint(int x, int y)
{
	if (m_pickingMode != RECT_ZONE)
		return;

	assert(m_rect2DLabel);
	assert(m_associatedWin);

	if (m_rect2DLabel->isSelected()) //already closed? we start a new label
	{
		float roi[4]={(float)x,(float)y,0,0};
		if (m_associatedWin)
			m_rect2DLabel->setParameters(m_associatedWin->getViewportParameters());
		m_rect2DLabel->setVisible(false);	//=invalid
		m_rect2DLabel->setSelected(false);	//=not closed
		m_rect2DLabel->setRoi(roi);
		m_rect2DLabel->setName(""); //reset name before display!
	}
	else //we close the existing one
	{
		float roi[4]={m_rect2DLabel->roi()[0],
						m_rect2DLabel->roi()[1],
						(float)x,
						(float)y};
		m_rect2DLabel->setRoi(roi);
		m_rect2DLabel->setVisible(true);	//=valid
		m_rect2DLabel->setSelected(true);	//=closed
	}

	if (m_associatedWin)
		m_associatedWin->updateGL();
}

void ccPointPropertiesDlg::update2DZone(int x, int y, Qt::MouseButtons buttons)
{
	if (m_pickingMode != RECT_ZONE)
		return;

	if (m_rect2DLabel->isSelected())
		return;

	float roi[4]={m_rect2DLabel->roi()[0],
					m_rect2DLabel->roi()[1],
					(float)x,
					(float)y};
	m_rect2DLabel->setRoi(roi);
	m_rect2DLabel->setVisible(true);

	if (m_associatedWin)
		m_associatedWin->updateGL();
}

static QString s_last2DLabelComment("");
void ccPointPropertiesDlg::close2DZone()
{
	if (m_pickingMode != RECT_ZONE)
		return;

	if (m_rect2DLabel->isSelected() || !m_rect2DLabel->isVisible())
		return;

	m_rect2DLabel->setSelected(true);

	bool ok;
	QString title = QInputDialog::getText(this, "Set area label title", "Title:", QLineEdit::Normal, s_last2DLabelComment, &ok);
	if (!ok)
	{
		m_rect2DLabel->setVisible(false);
	}
	else
	{
		m_rect2DLabel->setName(title);
		s_last2DLabelComment = title;
	}

	if (m_associatedWin)
		m_associatedWin->updateGL();
}

void ccPointPropertiesDlg::setMarkedPointTreeRootName(QString name)
{
    mMarkedPointTreeRootName = name;

    ensureMarkedPointTreeRootExist();
}

void ccPointPropertiesDlg::processPickedTriangle(ccMesh* mesh, unsigned triangleIndex, int x, int y)
{
    assert(mesh);
    assert(m_associatedWin);

    CCLib::TriangleSummitsIndexes* summitsIndexes = mesh->getTriangleIndexes(triangleIndex);
    ccGenericPointCloud* cloud = mesh->getAssociatedCloud();

    //计算该三角面片的三个顶点中距离鼠标点击点最近的顶点
    const CCVector3 *triangleVertices[3] = 
    {
        cloud->getPoint(summitsIndexes->i[0]),
        cloud->getPoint(summitsIndexes->i[1]),
        cloud->getPoint(summitsIndexes->i[2])
    };
    const CCVector2i clickedPoint(x, y);
    CCVector3 clickedPoint3D = m_associatedWin->backprojectPointOnTriangle(clickedPoint, *(triangleVertices[0]), *(triangleVertices[1]), *(triangleVertices[2]));
    unsigned nearestSummitIndex = 0;
    float nearestDistance = CCVector3::vdistance(clickedPoint3D.u, triangleVertices[0]->u);
    for (unsigned i = 1; i < 3; i++)
    {
        float tempDistance = CCVector3::vdistance(clickedPoint3D.u, triangleVertices[i]->u);
        if (tempDistance < nearestDistance)
        {
            nearestDistance = tempDistance;
            nearestSummitIndex = i;
        }
    }
    unsigned pointIndex = summitsIndexes->i[nearestSummitIndex];

    //如果cloud中不存在颜色信息，则添加之
    //if (!cloud->hasColors())
    //{
    //    if (!cloud->reserveTheRGBTable())
    //    {
    //        ccLog::Error("Point Cloud 添加颜色信息错误: 内存不足！");
    //    }
    //    else
    //    {
    //        cloud->resizeTheRGBTable(true);
    //        cloud->showColors(true);
    //    }
    //}

    if (!mShortestPathComputer)
    {
        mShortestPathComputer = new ShortestPathComputer();
        mShortestPathComputer->init(mesh, parentWidget());
    }
    else
    {
        if(mesh != mShortestPathComputer->getAssociatedMesh()) //添加的所有点必须属于同一个mesh
        {
            mShortestPathComputer->init(mesh, parentWidget());
        }
    }

    switch(m_pickingMode)
    {
    case MARK_POINT:
        assert(mCurrentMarkedPoint);
        mCurrentMarkedPoint->setPoint(mesh, pointIndex);
        if (m_associatedWin)
        {
            mCurrentMarkedPoint->setPosition((float)(x+20)/(float)m_associatedWin->width(),(float)(y+20)/(float)m_associatedWin->height());
        }
        onMarkDone();
        break;
    case MARK_LINE:
        assert(mCurrentMarkedLine);
        mCurrentMarkedLine->setShortestPathComputer(mShortestPathComputer);
        mCurrentMarkedLine->addPoint(mesh, pointIndex);
        if (m_associatedWin)
        {
            if (mCurrentMarkedLine->size() == 1)
            {
                mCurrentMarkedLine->setPosition((float)(x+20)/(float)m_associatedWin->width(),(float)(y+20)/(float)m_associatedWin->height());
                m_associatedWin->addToOwnDB(mCurrentMarkedLine);
            }
            //实时显示该MarkedLine
            m_associatedWin->redraw();
        }
        break;
    case MARK_AREA:
        assert(mCurrentMarkedArea);
        mCurrentMarkedArea->setShortestPathComputer(mShortestPathComputer);
        mCurrentMarkedArea->addPoint(mesh, pointIndex);
        if (m_associatedWin)
        {
            if (mCurrentMarkedArea->size() == 1)
            {
                mCurrentMarkedArea->setPosition((float)(x+20)/(float)m_associatedWin->width(),(float)(y+20)/(float)m_associatedWin->height());
                m_associatedWin->addToOwnDB(mCurrentMarkedArea);
            }
            //实时显示该MarkedLine
            m_associatedWin->redraw();
        }
        break;
    default:
        ccLog::Error("[Mark operation] Mark mode invalid!");
        return;
    }
}

void ccPointPropertiesDlg::addMarkedObjectToDB(const QString &markedTypeName, const QString &markedObjectName, const QColor &markedObjectColor)
{
    assert(mMarkedPointTreeRoot);

    ensureMarkedPointTreeRootExist();

    //若子标签中没有当前输入的类别名称，则新建，否则直接添加到该子标签
    ccHObject *markedType = mMarkedPointTreeRoot->findDirectChild(markedTypeName);
    if (!markedType)
    {
        markedType = new ccHObject(markedTypeName);
        mMarkedPointTreeRoot->addChild(markedType);
        m_ccRoot->addElement(markedType);
    }

    //将之前的标记的2D标签隐藏
    if (mLastMarkedObject)
    {
        mLastMarkedObject->setSelected(false);
    }

    MarkedObject *currentMarkedObject = NULL;
    switch(m_pickingMode)
    {
    case MARK_POINT:
        currentMarkedObject = mCurrentMarkedPoint;
        //重新生成MarkedObject
        mCurrentMarkedPoint = new MarkedPoint();
        break;
    case MARK_LINE:
        currentMarkedObject = mCurrentMarkedLine;
        //重新生成MarkedObject
        mCurrentMarkedLine = new MarkedLine();
        break;
    case MARK_AREA:
        currentMarkedObject = mCurrentMarkedArea;
        //重新生成MarkedObject
        mCurrentMarkedArea = new MarkedArea();
        break;
    default:
        ccLog::Error("[Add mark object to db] Mark mode invalid!");
        return;
    }
    mLastMarkedObject = currentMarkedObject;

    currentMarkedObject->setMarkedType(markedType);
    mLastMarkedTypeName = markedTypeName;

    currentMarkedObject->setName(markedObjectName);

    currentMarkedObject->setColor(markedObjectColor);

    //output info to Console
    ccLog::Print(QString("[Marked] ") + currentMarkedObject->getName());

    //添加到数据库并显示
    markedType->addChild(currentMarkedObject);
    m_ccRoot->addElement(currentMarkedObject);

    //添加并更新显示
    if (m_associatedWin)
    {
        m_associatedWin->removeFromOwnDB(currentMarkedObject); //可能之前add过该object
        m_associatedWin->addToOwnDB(currentMarkedObject);
        m_associatedWin->redraw();
    }
}

void ccPointPropertiesDlg::ensureMarkedPointTreeRootExist()
{
    assert(m_ccRoot);
    ccHObject *dbRootEntity = m_ccRoot->getRootEntity();
    assert(dbRootEntity);

    if (m_ccRoot->getRootEntity()->getChildrenNumber() != 0) //如果数据库非空，则检查其中的根标签中是否存在标记点根标签，如果不存在则创建之
    {
        int dbRootNum = dbRootEntity->getChildrenNumber();
        bool found = false;
        for (int i = 0; i < dbRootNum; i++)
        {
            if (dbRootEntity->getChild(i)->getName() == mMarkedPointTreeRootName)
            {
                found = true;
                mMarkedPointTreeRoot = dbRootEntity->getChild(i);
                break;
            }
        }
        if (!found)
        {
            mMarkedPointTreeRoot = new ccHObject(mMarkedPointTreeRootName);
            m_ccRoot->addElement(mMarkedPointTreeRoot);
        }
    }
    else
    {
        mMarkedPointTreeRoot = new ccHObject(mMarkedPointTreeRootName);
        m_ccRoot->addElement(mMarkedPointTreeRoot);
    }
}

void ccPointPropertiesDlg::onMarkDone()
{
    switch(m_pickingMode)
    {
    case MARK_POINT:
        if (mCurrentMarkedPoint->size() == 0)
        {
            return;
        }
        break;
    case MARK_LINE:
        if (mCurrentMarkedLine->size() == 0)
        {
            return;
        }
        break;
    case MARK_AREA:
        if (mCurrentMarkedArea->size() == 0)
        {
            return;
        }
        break;
    default:
        return;
    }

    //弹出设置标记属性对话框
    if (!mSetMarkedObjectPropertyDlg)
    {
        mSetMarkedObjectPropertyDlg = new SetMarkedObjectPropertyDlg(parentWidget());
        connect(mSetMarkedObjectPropertyDlg, SIGNAL(committed(const QString &, const QString &, const QColor &)), this, SLOT(addMarkedObjectToDB(const QString &, const QString &, const QColor &)));
    }
    mSetMarkedObjectPropertyDlg->setMarkedObjectTreeRoot(mMarkedPointTreeRoot);
    mSetMarkedObjectPropertyDlg->show();
}
