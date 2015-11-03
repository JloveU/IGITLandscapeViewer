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

    //���ñ�ǵ����ǩ����
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

    //��ѡ��ĵ���ӵ����ݿⲢ��ʾ
    void addMarkedObjectToDB(const QString &markedTypeName, const QString &markedObjectName, const QColor &markedObjectColor);

signals:

	//! Signal emitted when a new label is created
	void newLabel(ccHObject*);

private:

    //����ǵ����ǩ�Ƿ���ڣ������������½�֮
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

    //MainWindow���m_ccRoot
    ccDBRoot *m_ccRoot;

    //��ǵ����ǩ����
    QString mMarkedPointTreeRootName;

    //��ǵ����ǩ
    ccHObject *mMarkedPointTreeRoot;

    //��ǰ����ĵ�
    MarkedPoint *mCurrentMarkedPoint;

    //���ñ�ǵ����ԶԻ���
    SetMarkedObjectPropertyDlg *mSetMarkedObjectPropertyDlg;

    //�ϴ�����ı�ǵ�����
    QString mLastMarkedTypeName;

    //��ǰ��ǵ���
    MarkedLine *mCurrentMarkedLine;

    //�ϴα�ǵ�����
    MarkedObject *mLastMarkedObject;

    //���·��������
    ShortestPathComputer *mShortestPathComputer;

    //��ǰ��ǵ���
    MarkedArea *mCurrentMarkedArea;

    //��ǰ��ǵ�ObjectBag
    MarkedObjectBag *mCurrentMarkedObjectBag;

};

#endif
