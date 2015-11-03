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

#ifndef CC_POINT_PROPERTIES_DIALOG_HEADER
#define CC_POINT_PROPERTIES_DIALOG_HEADER

#include "ccPointPickingGenericInterface.h"

//Local
#include <ui_pointPropertiesDlg.h>

#include "ccDBRoot.h"

class cc2DLabel;
class cc2DViewportLabel;
class ccHObject;

#include "MarkedPoint.h"
#include "SetMarkedObjectPropertyDlg.h"
#include "MarkedLine.h"
#include "MarkedObject.h"
#include "ccMesh.h"
#include "MarkedArea.h"
#include "MarkedObjectBag.h"

//! Dialog for simple point picking (information, distance, etc.)
class ccPointPropertiesDlg : public ccPointPickingGenericInterface, public Ui::PointPropertiesDlg
{
	Q_OBJECT

public:

	//! Default constructor
	ccPointPropertiesDlg(QWidget* parent, ccDBRoot *ccRoot);
	//! Default destructor
	virtual ~ccPointPropertiesDlg();

	//inherited from ccPointPickingGenericInterface
	virtual bool start();
	virtual void stop(bool state);
	virtual bool linkWith(ccGLWindow* win);

    //设置标记点根标签名称
    void setMarkedPointTreeRootName(QString name);

protected slots:

    void onActivatePointMarking();
    void onActivateLineMarking();
    void onActivateAreaMarking();
    void onActivateObjectBagMarking();
    void onMarkUndo();
    void onMarkRedo();
    void onMarkDone();
    void onCancelLineMarking();
    void onCancelAreaMarking();
    void onCancelObjectBagMarking();
	void onClose();
	void activatePointPropertiesDisplay();
	void activateDistanceDisplay();
	void activateAngleDisplay();
	void activate2DZonePicking();
	void initializeState();
	void exportCurrentLabel();
	void update2DZone(int x, int y, Qt::MouseButtons buttons);
	void processClickedPoint(int x, int y);
	void close2DZone();

    //将选择的点添加到数据库并显示
    void addMarkedObjectToDB(const QString &markedTypeName, const QString &markedObjectName, const QColor &markedObjectColor);

signals:

	//! Signal emitted when a new label is created
	void newLabel(ccHObject*);

private:

    //检查标记点根标签是否存在，若不存在则新建之
    void ensureMarkedPointTreeRootExist();

protected:

	//! Picking mode
	enum Mode
	{
		POINT_INFO,
		POINT_POINT_DISTANCE,
		POINTS_ANGLE,
		RECT_ZONE,
        MARK_POINT,
        MARK_LINE,
        MARK_AREA,
        MARK_OBJECT_BAG
	};

	//inherited from ccPointPickingGenericInterface
	void processPickedPoint(ccPointCloud* cloud, unsigned pointIndex, int x, int y);

	//inherited from ccPointPickingGenericInterface
	void processPickedTriangle(ccMesh* mesh, unsigned triangleIndex, int x, int y);

	//! Current picking mode
	Mode m_pickingMode;

	//! Associated 3D label
	cc2DLabel* m_label;

	//! Associated 2D label
	cc2DViewportLabel* m_rect2DLabel;

    //MainWindow类的m_ccRoot
    ccDBRoot *m_ccRoot;

    //标记点根标签名称
    QString mMarkedPointTreeRootName;

    //标记点根标签
    ccHObject *mMarkedPointTreeRoot;

    //当前点击的点
    MarkedPoint *mCurrentMarkedPoint;

    //设置标记点属性对话框
    SetMarkedObjectPropertyDlg *mSetMarkedObjectPropertyDlg;

    //上次输入的标记点类型
    QString mLastMarkedTypeName;

    //当前标记的线
    MarkedLine *mCurrentMarkedLine;

    //上次标记的物体
    MarkedObject *mLastMarkedObject;

    //最短路径计算器
    ShortestPathComputer *mShortestPathComputer;

    //当前标记的线
    MarkedArea *mCurrentMarkedArea;

    //当前标记的ObjectBag
    MarkedObjectBag *mCurrentMarkedObjectBag;

};

#endif
