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

#include "ccPropertiesTreeDelegate.h"

//Local
#include "sfEditDlg.h"
#include "matrixDisplayDlg.h"
#include "mainwindow.h"
#include "ccColorScaleEditorDlg.h"
#include "ccColorScaleSelector.h"

//qCC_glWindow
#include <ccGLWindow.h>
#include <ccGuiParameters.h>

//qCC_db
#include <ccHObjectCaster.h>
#include <ccHObject.h>
#include <ccPointCloud.h>
#include <ccMesh.h>
#include <ccPolyline.h>
#include <ccSubMesh.h>
#include <ccOctree.h>
#include <ccKdTree.h>
#include <ccImage.h>
#include <cc2DLabel.h>
#include <cc2DViewportLabel.h>
#include <cc2DViewportObject.h>
#include <ccGBLSensor.h>
#include <ccCameraSensor.h>
#include <ccMaterialSet.h>
#include <ccAdvancedTypes.h>
#include <ccGenericPrimitive.h>
#include <ccSphere.h>
#include <ccCone.h>
#include <ccFacet.h>
#include <ccSensor.h>
#include <ccIndexedTransformationBuffer.h>
#include <ccScalarField.h>
#include <ccColorScalesManager.h>

//Qt
#include <QStandardItemModel>
#include <QAbstractItemView>
#include <QSpinBox>
#include <QSlider>
#include <QComboBox>
#include <QCheckBox>
#include <QLocale>
#include <QPushButton>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QToolButton>

//System
#include <assert.h>

#include "ccLog.h"
#include <QColor>
#include "CCGeom.h" //CCVector3d defined
#include "MarkedObject.h"
#include "MarkedPoint.h"
#include "MarkedLine.h"
#include "MarkedArea.h"
#include "MarkedObjectBag.h"

// Default 'None' string
static const QString c_noneString = QString("None");

// Default color sources string
static const QString s_rgbColor("RGB");
static const QString s_sfColor ("Scalar field");

// Other strings
static const QString c_defaultPointSizeString = QString("Default");
static const QString c_defaultPolyWidthSizeString = QString("Default Width");

// Default separator colors
static QString SEPARATOR_STYLESHEET("QLabel { background-color : darkGray; color : white; }");
//=========================================================TTEM=======================================================//
//Shortcut to create a delegate item
QStandardItem* ITEM(QString name,
					Qt::ItemFlag additionalFlags = Qt::NoItemFlags,
					ccPropertiesTreeDelegate::CC_PROPERTY_ROLE role = ccPropertiesTreeDelegate::OBJECT_NO_PROPERTY)
{
	QStandardItem* item = new QStandardItem(name);
	//flags
	item->setFlags(Qt::ItemIsEnabled | additionalFlags);
	//role (if any)
	if (role != ccPropertiesTreeDelegate::OBJECT_NO_PROPERTY)
		item->setData(role);

	return item;
}
//======================================================CHECHABLE_ITEM=================================================//
//Shortcut to create a checkable delegate item
QStandardItem* CHECKABLE_ITEM(bool checkState, ccPropertiesTreeDelegate::CC_PROPERTY_ROLE role)
{
	QStandardItem* item = ITEM("",Qt::ItemIsUserCheckable,role);
	//check state
	item->setCheckState(checkState ? Qt::Checked : Qt::Unchecked);

	return item;
}
//======================================================PERSISTENT_EDITOR=============================================//
//Shortcut to create a persistent editor item
QStandardItem* PERSISTENT_EDITOR(ccPropertiesTreeDelegate::CC_PROPERTY_ROLE role)
{
	return ITEM(QString(),Qt::ItemIsEditable,role);
}

ccPropertiesTreeDelegate::ccPropertiesTreeDelegate(	QStandardItemModel* model,
													QAbstractItemView* view,
													QObject *parent)
		: QStyledItemDelegate(parent)
		, m_currentObject(0)
		, m_model(model)
		, m_view(view)
{
	assert(m_model && m_view);
}

ccPropertiesTreeDelegate::~ccPropertiesTreeDelegate()
{
	unbind();
}

QSize ccPropertiesTreeDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	assert(m_model);

	QStandardItem* item = m_model->itemFromIndex(index);

	if (item && item->data().isValid())
	{
		switch (item->data().toInt())
		{
		case OBJECT_CURRENT_DISPLAY:
		case OBJECT_CURRENT_SCALAR_FIELD:
		case OBJECT_OCTREE_TYPE:
		case OBJECT_COLOR_RAMP_STEPS:
		case OBJECT_CLOUD_POINT_SIZE:
			return QSize(50,18);
		case OBJECT_COLOR_SOURCE:
		case OBJECT_POLYLINE_WIDTH:
		case OBJECT_CURRENT_COLOR_RAMP:
			return QSize(70,22);
		case OBJECT_CLOUD_SF_EDITOR:
			return QSize(250,200);
		case OBJECT_SENSOR_MATRIX_EDITOR:
		case OBJECT_HISTORY_MATRIX_EDITOR:
			return QSize(250,120);
		}
	}

	return QStyledItemDelegate::sizeHint(option,index);
}
//*/
//=======================================================unbind================================================//
void ccPropertiesTreeDelegate::unbind()
{
	if (m_model)
		m_model->disconnect(this);
}

ccHObject* ccPropertiesTreeDelegate::getCurrentObject()
{
	return m_currentObject;
}

//=========================================fillModel===========================================================//
void ccPropertiesTreeDelegate::fillModel(ccHObject* hObject)
{
	if (!hObject)return;

	unbind();

	m_currentObject = hObject;

	//save current scroll position
	int scrollPos = (m_view && m_view->verticalScrollBar() ? m_view->verticalScrollBar()->value() : 0);

	if (m_model){
		m_model->removeRows(0,m_model->rowCount());
		m_model->setColumnCount(2); //设置成两列
		//m_model->setHeaderData(0, Qt::Horizontal, "Property"); //属性列
		m_model->setHeaderData(0, Qt::Horizontal, "属性");
		//m_model->setHeaderData(1, Qt::Horizontal, "State/Value"); //状态值
		m_model->setHeaderData(1, Qt::Horizontal, "状态/值");
	}

	if (m_currentObject->isHierarchy()) //如果是分层物体
		if (!m_currentObject->isA(CC_TYPES::VIEWPORT_2D_LABEL)) //don't need to display this kind of info for viewport labels!
			fillWithHObject(m_currentObject);

	if (m_currentObject->isKindOf(CC_TYPES::POINT_CLOUD)){
		fillWithPointCloud(ccHObjectCaster::ToGenericPointCloud(m_currentObject));
	}
	else if (m_currentObject->isKindOf(CC_TYPES::MESH))
	{
		fillWithMesh(ccHObjectCaster::ToGenericMesh(m_currentObject));

		if (m_currentObject->isKindOf(CC_TYPES::PRIMITIVE))
			fillWithPrimitive(ccHObjectCaster::ToPrimitive(m_currentObject));
	}
	else if (m_currentObject->isA(CC_TYPES::FACET))
	{
		fillWithFacet(ccHObjectCaster::ToFacet(m_currentObject));
	}
	else if (m_currentObject->isA(CC_TYPES::POLY_LINE))
	{
		fillWithPolyline(ccHObjectCaster::ToPolyline(m_currentObject));
	}
	else if (m_currentObject->isA(CC_TYPES::POINT_OCTREE))
	{
		fillWithPointOctree(ccHObjectCaster::ToOctree(m_currentObject));
	}
	else if (m_currentObject->isA(CC_TYPES::POINT_KDTREE))
	{
		fillWithPointKdTree(ccHObjectCaster::ToKdTree(m_currentObject));
	}
	else if (m_currentObject->isKindOf(CC_TYPES::IMAGE))
	{
		fillWithImage(ccHObjectCaster::ToImage(m_currentObject));
	}
	else if (m_currentObject->isA(CC_TYPES::LABEL_2D))
	{
        fillWithLabel(ccHObjectCaster::To2DLabel(m_currentObject));
        MarkedObject *markedObject = dynamic_cast<MarkedObject*>(m_currentObject); //若m_currentObject不是MarkedObject类型（cc2DLabel的子类），则dynamic_cast会返回NULL，此处用来判断其是否MarkedObject类型
        if (markedObject != NULL)
        {
            fillWithMarkedObject(markedObject);
        }
	}
	else if (m_currentObject->isKindOf(CC_TYPES::VIEWPORT_2D_OBJECT))
	{
		fillWithViewportObject(ccHObjectCaster::To2DViewportObject(m_currentObject));
	}
	else if (m_currentObject->isKindOf(CC_TYPES::GBL_SENSOR))
	{
		fillWithGBLSensor(ccHObjectCaster::ToGBLSensor(m_currentObject));
	}
	else if (m_currentObject->isKindOf(CC_TYPES::CAMERA_SENSOR))
	{
		fillWithCameraSensor(ccHObjectCaster::ToCameraSensor(m_currentObject));
	}
	else if (m_currentObject->isA(CC_TYPES::MATERIAL_SET))
	{
		fillWithMaterialSet(static_cast<ccMaterialSet*>(m_currentObject));
	}
	else if (m_currentObject->isA(CC_TYPES::NORMAL_INDEXES_ARRAY))
	{
		fillWithChunkedArray(static_cast<NormsIndexesTableType*>(m_currentObject));
	}
	else if (m_currentObject->isA(CC_TYPES::TEX_COORDS_ARRAY))
	{
		fillWithChunkedArray(static_cast<TextureCoordsContainer*>(m_currentObject));
	}
	else if (m_currentObject->isA(CC_TYPES::NORMALS_ARRAY))
	{
		fillWithChunkedArray(static_cast<NormsTableType*>(m_currentObject));
	}
	else if (m_currentObject->isA(CC_TYPES::RGB_COLOR_ARRAY))
	{
		fillWithChunkedArray(static_cast<ColorsTableType*>(m_currentObject));
	}
	else if (m_currentObject->isA(CC_TYPES::TRANS_BUFFER))
	{
		fillWithTransBuffer(static_cast<ccIndexedTransformationBuffer*>(m_currentObject));
	}
    else if (m_currentObject->isA(CC_TYPES::CUSTOM_H_OBJECT))
    {
    }

	//transformation history
	if (	m_currentObject->isKindOf(CC_TYPES::POINT_CLOUD)
		||	m_currentObject->isKindOf(CC_TYPES::MESH)
		||	m_currentObject->isKindOf(CC_TYPES::FACET)
		||	m_currentObject->isKindOf(CC_TYPES::POLY_LINE)
		||	m_currentObject->isKindOf(CC_TYPES::SENSOR))
	{
		//addSeparator("Transformation history");
		addSeparator("变换历史记录");
		appendWideRow(PERSISTENT_EDITOR(OBJECT_HISTORY_MATRIX_EDITOR));//????????????????????????
		fillWithMetaData(m_currentObject);
	}
	
	//go back to original position
	if (scrollPos > 0)
		m_view->verticalScrollBar()->setSliderPosition(scrollPos);

	if (m_model)
		connect(m_model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(updateItem(QStandardItem*)));
}

void ccPropertiesTreeDelegate::appendRow(QStandardItem* leftItem, QStandardItem* rightItem, bool openPersistentEditor/*=false*/)
{
	assert(leftItem && rightItem);
	assert(m_model);

	if (m_model)
	{
		//append row
		QList<QStandardItem*> rowItems;
		{
			rowItems.push_back(leftItem);
			rowItems.push_back(rightItem);
		}
		m_model->appendRow(rowItems);

		//the persistent editor (if any) is always the right one!
		if (openPersistentEditor)
			m_view->openPersistentEditor(m_model->index(m_model->rowCount()-1,1));
	}
}
//======================================================appendWideRow=====================================================//
void ccPropertiesTreeDelegate::appendWideRow(QStandardItem* item, bool openPersistentEditor/*=true*/)
{
	assert(item);
	assert(m_model);

	if (m_model)
	{
		m_model->appendRow(item);
		if (openPersistentEditor)
			m_view->openPersistentEditor(m_model->index(m_model->rowCount()-1,0));
	}
}


void ccPropertiesTreeDelegate::addSeparator(QString title)
{
	if (m_model)
	{
		//DGM: we can't use the 'text' of the item as it will be displayed under the associated editor (label)!
		//So we simply use the 'accessible description' field
		QStandardItem* leftItem = new QStandardItem(/*title*/);
		leftItem->setData(TREE_VIEW_HEADER);
		leftItem->setAccessibleDescription(title);
		m_model->appendRow(leftItem);
		m_view->openPersistentEditor(m_model->index(m_model->rowCount()-1,0));
	}
}
//=======================================================fillWithMetaData=============================================//
void ccPropertiesTreeDelegate::fillWithMetaData(ccObject* _obj)
{
	assert(_obj && m_model);

	const QVariantMap& metaData = _obj->metaData();
	if (metaData.size() == 0)return;

	//addSeparator("Meta data");
	addSeparator("元数据");

	for (QVariantMap::ConstIterator it = metaData.constBegin(); it != metaData.constEnd(); ++it)
	{
		QString value;
		if (it.value().canConvert(QVariant::String)){
			QVariant var = it.value();
			var.convert(QVariant::String);
			value = var.toString();
		}
		else
		{
			value = QString(QVariant::typeToName(it.value().type()));
		}
		//switch (var.type())
		//{
		//	QVariant::Bool
		//}
		appendRow( ITEM(it.key()), ITEM(value) );
	}
}
//============================================================fillWithHObject=============================================//
void ccPropertiesTreeDelegate::fillWithHObject(ccHObject* _obj)
{
	assert(_obj && m_model);

	//addSeparator("CC Object");
	addSeparator("IGIT 物体");

	//name
	//appendRow( ITEM("Name"), ITEM(_obj->getName(),Qt::ItemIsEditable,OBJECT_NAME) );
	appendRow( ITEM("名称"), ITEM(_obj->getName(),Qt::ItemIsEditable/*可编辑*/,OBJECT_NAME) );

	//visibility
	if (!_obj->isVisiblityLocked())
		appendRow( ITEM("可视性"), CHECKABLE_ITEM(_obj->isVisible(),OBJECT_VISIBILITY) );
		//appendRow( ITEM("Visible"), CHECKABLE_ITEM(_obj->isVisible(),OBJECT_VISIBILITY) );

	//normals
	if (_obj->hasNormals())
		appendRow( ITEM("法向量"), CHECKABLE_ITEM(_obj->normalsShown(),OBJECT_NORMALS_SHOWN) );
		//appendRow( ITEM("Normals"), CHECKABLE_ITEM(_obj->normalsShown(),OBJECT_NORMALS_SHOWN) );

	//name in 3D
	//appendRow( ITEM("Show name (in 3D)"), CHECKABLE_ITEM(_obj->nameShownIn3D(),OBJECT_NAME_IN_3D) );
	appendRow( ITEM("显示名称 (3D)"), CHECKABLE_ITEM(_obj->nameShownIn3D(),OBJECT_NAME_IN_3D) );

	//color source
	if (_obj->hasColors() || _obj->hasScalarFields())
		appendRow( ITEM("颜色"), PERSISTENT_EDITOR(OBJECT_COLOR_SOURCE), true );
		//appendRow( ITEM("Colors"), PERSISTENT_EDITOR(OBJECT_COLOR_SOURCE), true );

	//Bounding-box
	{
		ccBBox box;
		bool fitBBox = false;
		if (_obj->getSelectionBehavior() == ccHObject::SELECTION_FIT_BBOX){
			ccGLMatrix trans;
			box = _obj->getOwnFitBB(trans);
			box += trans.getTranslationAsVec3D();
			fitBBox = true;
		}
		else{
			box = _obj->getBB_recursive();
		}

		if (box.isValid())	{
			//Box dimensions
			CCVector3 bboxDiag = box.getDiagVec();
			//appendRow(	ITEM(fitBBox ? "Local box dimensions" : "Box dimensions"),
			//			ITEM(QString("X: %0\nY: %1\nZ: %2").arg(bboxDiag.x).arg(bboxDiag.y).arg(bboxDiag.z)) );
			appendRow(	ITEM(fitBBox ? "局部边界框维度" : "边界框维度"),
						ITEM(QString("X: %0\nY: %1\nZ: %2").arg(bboxDiag.x).arg(bboxDiag.y).arg(bboxDiag.z)) );

			//Box center
			CCVector3 bboxCenter = box.getCenter();
			//appendRow(	ITEM("Box center"),
			//			ITEM(QString("X: %0\nY: %1\nZ: %2").arg(bboxCenter.x).arg(bboxCenter.y).arg(bboxCenter.z)) );
			appendRow(	ITEM("边界框中心"),
				        ITEM(QString("X: %0\nY: %1\nZ: %2").arg(bboxCenter.x).arg(bboxCenter.y).arg(bboxCenter.z)) );
		}
	}

	//infos (unique ID, children) //DGM: on the same line so as to gain space
	//appendRow( ITEM("Info"), ITEM(QString("Object ID: %1 - Children: %2").arg(_obj->getUniqueID()).arg(_obj->getChildrenNumber())) );

	//display window
	if (!_obj->isLocked())
		appendRow( ITEM("当前显示"), PERSISTENT_EDITOR(OBJECT_CURRENT_DISPLAY), true );
		//appendRow( ITEM("Current Display"), PERSISTENT_EDITOR(OBJECT_CURRENT_DISPLAY), true );
}
//======================================================fillWithShifted=================================================//
void ccPropertiesTreeDelegate::fillWithShifted(ccShiftedObject* _obj)
{
	assert(_obj && m_model);

	//global shift & scale
	const CCVector3d& shift = _obj->getGlobalShift();
	//appendRow( ITEM("Global shift"), ITEM(QString("(%1;%2;%3)").arg(shift.x,0,'f',2).arg(shift.y,0,'f',2).arg(shift.z,0,'f',2)) );
	appendRow( ITEM("全局平移"), ITEM(QString("(%1;%2;%3)").arg(shift.x,0,'f',2).arg(shift.y,0,'f',2).arg(shift.z,0,'f',2)) );

	double scale = _obj->getGlobalScale();
	//appendRow( ITEM("Global scale"), ITEM(QString("%1").arg(scale,0,'f',6)) );
	appendRow( ITEM("全局尺度"), ITEM(QString("%1").arg(scale,0,'f',6)) );
}
//=====================================================fillWithPointCloud===============================================//
void ccPropertiesTreeDelegate::fillWithPointCloud(ccGenericPointCloud* _obj)
{
	assert(_obj && m_model);

	//addSeparator("Cloud");
	addSeparator("点云");

	//number of points
	//appendRow( ITEM("Points"), ITEM(QLocale(QLocale::English).toString(_obj->size())) );
	appendRow( ITEM("点的数量"), ITEM(QLocale(QLocale::English).toString(_obj->size())) );

	//global shift & scale
	fillWithShifted(_obj);

	//custom point size
	//appendRow( ITEM("Point size"), PERSISTENT_EDITOR(OBJECT_CLOUD_POINT_SIZE), true );
	appendRow( ITEM("点的大小"), PERSISTENT_EDITOR(OBJECT_CLOUD_POINT_SIZE), true );

	fillSFWithPointCloud(_obj);
}

void ccPropertiesTreeDelegate::fillSFWithPointCloud(ccGenericPointCloud* _obj)
{
	assert(m_model);

	//for "real" point clouds only
	ccPointCloud* cloud = ccHObjectCaster::ToPointCloud(_obj);
	if (!cloud)
		return;

	//Scalar fields
	unsigned sfCount = cloud->getNumberOfScalarFields();
	if (sfCount != 0)
	{
		addSeparator(sfCount > 1 ? "Scalar Fields" : "Scalar Field");

		//fields number
		appendRow( ITEM("Count"), ITEM(QString::number(sfCount)) );

		//fields list combo
		appendRow( ITEM("Active"), PERSISTENT_EDITOR(OBJECT_CURRENT_SCALAR_FIELD), true );

		//no need to go any further if no SF is currently active
		CCLib::ScalarField* sf = cloud->getCurrentDisplayedScalarField();
		if (sf)
		{
			addSeparator("Color Scale");

			//color scale selection combo box
			appendRow( ITEM("Current"), PERSISTENT_EDITOR(OBJECT_CURRENT_COLOR_RAMP), true );

			//color scale steps
			appendRow( ITEM("Steps"), PERSISTENT_EDITOR(OBJECT_COLOR_RAMP_STEPS), true );

			//scale visible?
			appendRow( ITEM("Visible"), CHECKABLE_ITEM(cloud->sfColorScaleShown(),OBJECT_SF_SHOW_SCALE) );

			addSeparator("SF display params");

			//SF edit dialog (warning: 2 columns)
			appendWideRow(PERSISTENT_EDITOR(OBJECT_CLOUD_SF_EDITOR));
		}
	}
}

void ccPropertiesTreeDelegate::fillWithPrimitive(ccGenericPrimitive* _obj)
{
	assert(_obj && m_model);

	addSeparator("Primitive");

	//type
	appendRow( ITEM("Type"), ITEM(_obj->getTypeName()) );

	//drawing steps
	if (_obj->hasDrawingPrecision())
		appendRow( ITEM("Drawing precision"), PERSISTENT_EDITOR(OBJECT_PRIMITIVE_PRECISION), true );

	if (_obj->isA(CC_TYPES::SPHERE))
	{
		appendRow( ITEM("Radius"), PERSISTENT_EDITOR(OBJECT_SPHERE_RADIUS), true );
	}
	else if (_obj->isKindOf(CC_TYPES::CONE)) //cylinders are also cones!
	{
		appendRow( ITEM("Height"), PERSISTENT_EDITOR(OBJECT_CONE_HEIGHT), true );
		if (_obj->isA(CC_TYPES::CYLINDER))
		{
			appendRow( ITEM("Radius"), PERSISTENT_EDITOR(OBJECT_CONE_BOTTOM_RADIUS), true );
		}
		else
		{
			appendRow( ITEM("Bottom radius"), PERSISTENT_EDITOR(OBJECT_CONE_BOTTOM_RADIUS), true );
			appendRow( ITEM("Top radius"), PERSISTENT_EDITOR(OBJECT_CONE_TOP_RADIUS), true );
		}
	}
}

void ccPropertiesTreeDelegate::fillWithFacet(ccFacet* _obj)
{
	assert(_obj && m_model);

	addSeparator("Facet");

	//surface
	appendRow( ITEM("Surface"), ITEM(QLocale(QLocale::English).toString(_obj->getSurface())) );

	//RMS
	appendRow( ITEM("RMS"), ITEM(QLocale(QLocale::English).toString(_obj->getRMS())) );

	//center
	appendRow( ITEM("Center"), ITEM(QString("(%1 ; %2 ; %3)").arg(_obj->getCenter().x).arg(_obj->getCenter().y).arg(_obj->getCenter().z)) );

	//normal
	appendRow( ITEM("Normal"), ITEM(QString("(%1 ; %2 ; %3)").arg(_obj->getNormal().x).arg(_obj->getNormal().y).arg(_obj->getNormal().z)) );

	//contour visibility
	if (_obj->getContour())
		appendRow( ITEM("Show contour"), CHECKABLE_ITEM(_obj->getContour()->isVisible(),OBJECT_FACET_CONTOUR) );

	//polygon visibility
	if (_obj->getPolygon())
		appendRow( ITEM("Show polygon"), CHECKABLE_ITEM(_obj->getPolygon()->isVisible(),OBJECT_FACET_MESH) );

	//normal vector visibility
	appendRow( ITEM("Show normal vector"), CHECKABLE_ITEM(_obj->normalVectorIsShown(),OBJECT_FACET_NORMAL_VECTOR) );

}

void ccPropertiesTreeDelegate::fillWithMesh(ccGenericMesh* _obj)
{
	assert(_obj && m_model);

	bool isSubMesh = _obj->isA(CC_TYPES::SUB_MESH);

	addSeparator(isSubMesh ? "Sub-mesh" : "Mesh");

	//number of facets
	appendRow( ITEM("Faces"), ITEM(QLocale(QLocale::English).toString(_obj->size())) );

	//material/texture
	if (_obj->hasMaterials())
		appendRow( ITEM("Materials/textures"), CHECKABLE_ITEM(_obj->materialsShown(),OBJECT_MATERIALS) );

	//wireframe
	appendRow( ITEM("Wireframe"), CHECKABLE_ITEM(_obj->isShownAsWire(),OBJECT_MESH_WIRE) );

	//stippling (ccMesh only)
	//if (_obj->isA(CC_TYPES::MESH)) //DGM: can't remember why?
		appendRow( ITEM("Stippling"), CHECKABLE_ITEM(static_cast<ccMesh*>(_obj)->stipplingEnabled(),OBJECT_MESH_STIPPLING) );

	//we also integrate vertices SF into mesh properties
	ccGenericPointCloud* vertices = _obj->getAssociatedCloud();
	if (vertices && (!vertices->isLocked() || _obj->isAncestorOf(vertices)))
		fillSFWithPointCloud(vertices);
}

void ccPropertiesTreeDelegate::fillWithPolyline(ccPolyline* _obj)
{
	assert(_obj && m_model);

	addSeparator("Polyline");

	//number of vertices
	appendRow( ITEM("Vertices"), ITEM(QLocale(QLocale::English).toString(_obj->size())) );

	//polyline length
	appendRow( ITEM("Length"), ITEM(QLocale(QLocale::English).toString(_obj->computeLength())) );

	//custom line width
	appendRow( ITEM("Line width"), PERSISTENT_EDITOR(OBJECT_POLYLINE_WIDTH), true );

	//global shift & scale
	fillWithShifted(_obj);
}

void ccPropertiesTreeDelegate::fillWithPointOctree(ccOctree* _obj)
{
	assert(_obj && m_model);

	addSeparator("Octree");

	//display mode
	appendRow( ITEM("Display mode"), PERSISTENT_EDITOR(OBJECT_OCTREE_TYPE), true );

	//level
	appendRow( ITEM("Display level"), PERSISTENT_EDITOR(OBJECT_OCTREE_LEVEL), true );

	addSeparator("Current level");

	//current display level
	int level = _obj->getDisplayedLevel();
	assert(level > 0 && level <= ccOctree::MAX_OCTREE_LEVEL);

	//cell size
	PointCoordinateType cellSize = _obj->getCellSize(static_cast<uchar>(level));
	appendRow( ITEM("Cell size"), ITEM(QString::number(cellSize)) );

	//cell count
	unsigned cellCount = _obj->getCellNumber(static_cast<uchar>(level));
	appendRow( ITEM("Cell count"), ITEM(QLocale(QLocale::English).toString(cellCount)) );

	//total volume of filled cells
	appendRow( ITEM("Filled volume"), ITEM(QString::number((double)cellCount*pow((double)cellSize,3.0))) );
}

void ccPropertiesTreeDelegate::fillWithPointKdTree(ccKdTree* _obj)
{
	assert(_obj && m_model);

	addSeparator("Kd-tree");

	//max error
	appendRow( ITEM("Max Error"), ITEM(QString::number(_obj->getMaxError())) );
	//max error measure
	{
		QString errorMeasure;
		switch(_obj->getMaxErrorType())
		{
		case CCLib::DistanceComputationTools::RMS:
			errorMeasure = "RMS";
			break;
		case CCLib::DistanceComputationTools::MAX_DIST_68_PERCENT:
			errorMeasure = "Max dist @ 68%";
			break;
		case CCLib::DistanceComputationTools::MAX_DIST_95_PERCENT:
			errorMeasure = "Max dist @ 95%";
			break;
		case CCLib::DistanceComputationTools::MAX_DIST_99_PERCENT:
			errorMeasure = "Max dist @ 99%";
			break;
		case CCLib::DistanceComputationTools::MAX_DIST:
			errorMeasure = "Max distance";
			break;
		default:
			assert(false);
			errorMeasure = "unknown";
			break;
		}
		appendRow( ITEM("Error measure"), ITEM(errorMeasure) );
	}
}

void ccPropertiesTreeDelegate::fillWithImage(ccImage* _obj)
{
	assert(_obj && m_model);

	//addSeparator("Image");
	addSeparator("图像");

	//image width
	//appendRow( ITEM("Width"), ITEM(QString::number(_obj->getW())) );
	appendRow( ITEM("宽度"), ITEM(QString::number(_obj->getW())) );

	//image height
	//appendRow( ITEM("Height"), ITEM(QString::number(_obj->getH())) );
	appendRow( ITEM("高度"), ITEM(QString::number(_obj->getH())) );

	//transparency
	//appendRow( ITEM("Alpha"), PERSISTENT_EDITOR(OBJECT_IMAGE_ALPHA), true );
	appendRow( ITEM("透明度"), PERSISTENT_EDITOR(OBJECT_IMAGE_ALPHA), true );

	if (_obj->getAssociatedSensor())
	{
		//addSeparator("Sensor");
		addSeparator("传感器");
		//"Set Viewport" button (shortcut to associated sensor)
		appendRow( ITEM("应用视口"), PERSISTENT_EDITOR(OBJECT_APPLY_IMAGE_VIEWPORT), true );
		//appendRow( ITEM("Apply Viewport"), PERSISTENT_EDITOR(OBJECT_APPLY_IMAGE_VIEWPORT), true );
	}
}

void ccPropertiesTreeDelegate::fillWithLabel(cc2DLabel* _obj)
{
	assert(_obj && m_model);

	addSeparator("Label");

	//Body
	//QStringList body = _obj->getLabelContent(ccGui::Parameters().displayedNumPrecision);
	//appendRow( ITEM("Body"), ITEM(body.join("\n")) );

	//Show label in 2D
	appendRow( ITEM("Show 2D label"), CHECKABLE_ITEM(_obj->isDisplayedIn2D(),OBJECT_LABEL_DISP_2D) );

	//Show label in 3D
	appendRow( ITEM("Show 3D legend(s)"), CHECKABLE_ITEM(_obj->isDisplayedIn3D(),OBJECT_LABEL_DISP_3D) );
}

void ccPropertiesTreeDelegate::fillWithViewportObject(cc2DViewportObject* _obj)
{
	assert(_obj && m_model);

	addSeparator("Viewport");

	//Name
	appendRow( ITEM("Name"), ITEM(_obj->getName().isEmpty() ? "undefined" : _obj->getName()) );

	//"Apply Viewport" button
	appendRow( ITEM("Apply Viewport"), PERSISTENT_EDITOR(OBJECT_APPLY_LABEL_VIEWPORT), true );
}

void ccPropertiesTreeDelegate::fillWithTransBuffer(ccIndexedTransformationBuffer* _obj)
{
	assert(_obj && m_model);

	addSeparator("Trans. buffer");

	//Associated positions
	appendRow( ITEM("Count"), ITEM(QString::number(_obj->size())) );

	//Show path as polyline
	appendRow( ITEM("Show path"), CHECKABLE_ITEM(_obj->isPathShonwAsPolyline(),OBJECT_SHOW_TRANS_BUFFER_PATH) );

	//Show trihedrons
	appendRow( ITEM("Show trihedrons"), CHECKABLE_ITEM(_obj->triherdonsShown(),OBJECT_SHOW_TRANS_BUFFER_TRIHDERONS) );

	//Trihedrons scale
	appendRow( ITEM("Scale"), PERSISTENT_EDITOR(OBJECT_TRANS_BUFFER_TRIHDERONS_SCALE), true );
}

void ccPropertiesTreeDelegate::fillWithSensor(ccSensor* _obj)
{
	assert(_obj && m_model);

	//Sensor drawing scale
	//appendRow( ITEM("Drawing scale"), PERSISTENT_EDITOR(OBJECT_SENSOR_DISPLAY_SCALE), true );
	appendRow( ITEM("尺度"), PERSISTENT_EDITOR(OBJECT_SENSOR_DISPLAY_SCALE), true );

	//"Apply Viewport" button
	//appendRow( ITEM("Apply Viewport"), PERSISTENT_EDITOR(OBJECT_APPLY_SENSOR_VIEWPORT), true );
	appendRow( ITEM("视口"), PERSISTENT_EDITOR(OBJECT_APPLY_SENSOR_VIEWPORT), true );

	//sensor aboslute orientation
	//addSeparator("Position/Orientation");
	addSeparator("位置/方向");
	appendWideRow(PERSISTENT_EDITOR(OBJECT_SENSOR_MATRIX_EDITOR));

	//Associated positions
	//addSeparator("Associated positions");
	addSeparator("相关位置");

	//number of positions
	//appendRow( ITEM("Count"), ITEM(QString::number(_obj->getPositions() ? _obj->getPositions()->size() : 0)) );
	appendRow( ITEM("数目"), ITEM(QString::number(_obj->getPositions() ? _obj->getPositions()->size() : 0)) );

	double minIndex,maxIndex;
	_obj->getIndexBounds(minIndex,maxIndex);
	if (minIndex != maxIndex)
	{
		//Index span
		//appendRow( ITEM("Indexes"), ITEM(QString("%1 - %2").arg(minIndex).arg(maxIndex)) );
		appendRow( ITEM("索引"), ITEM(QString("%1 - %2").arg(minIndex).arg(maxIndex)) );

		//Current index
		//appendRow( ITEM("Active index"), PERSISTENT_EDITOR(OBJECT_SENSOR_INDEX), true );
		appendRow( ITEM("激活索引"), PERSISTENT_EDITOR(OBJECT_SENSOR_INDEX), true );
	}

}

void ccPropertiesTreeDelegate::fillWithGBLSensor(ccGBLSensor* _obj)
{
	assert(_obj && m_model);

	addSeparator("GBL Sensor");

	//Uncertainty
	appendRow( ITEM("Uncertainty"), ITEM(QString::number(_obj->getUncertainty())) );

	//angles
	addSeparator("Angular viewport (degrees)");
	{
		//Angular range (yaw)
		PointCoordinateType yawMin = _obj->getMinYaw();
		PointCoordinateType yawMax = _obj->getMaxYaw();
		appendRow( ITEM("Yaw span"), ITEM(QString("[%1 ; %2]").arg(yawMin * CC_RAD_TO_DEG,0,'f',2).arg(yawMax * CC_RAD_TO_DEG,0,'f',2)) );

		//Angular steps (yaw)
		PointCoordinateType yawStep = _obj->getYawStep();
		appendRow( ITEM("Yaw step"), ITEM(QString("%1").arg(yawStep * CC_RAD_TO_DEG,0,'f',4)) );

		//Angular range (pitch)
		PointCoordinateType pitchMin = _obj->getMinPitch();
		PointCoordinateType pitchMax = _obj->getMaxPitch();
		appendRow( ITEM("Pitch span"), ITEM(QString("[%1 ; %2]").arg(pitchMin * CC_RAD_TO_DEG,0,'f',2).arg(pitchMax * CC_RAD_TO_DEG,0,'f',2)) );

		//Angular steps (pitch)
		PointCoordinateType pitchStep = _obj->getPitchStep();
		appendRow( ITEM("Pitch step"), ITEM(QString("%1").arg(pitchStep * CC_RAD_TO_DEG,0,'f',4)) );
	}

	//Positions
	fillWithSensor(_obj);
}

void ccPropertiesTreeDelegate::fillWithCameraSensor(ccCameraSensor* _obj)
{
	assert(_obj && m_model);

	//addSeparator("Camera Sensor");
	addSeparator("相机传感器");

	const ccCameraSensor::IntrinsicParameters& params = _obj->getIntrinsicParameters();

	//Focal
	//appendRow( ITEM("Focal (pix)"), ITEM(QString::number(params.focal_pix)) );
	appendRow( ITEM("焦距(像素)"), ITEM(QString::number(params.focal_pix)) );

	//Array size
	//appendRow( ITEM("Array size"), ITEM(QString("%1 x %2").arg(params.arrayWidth).arg(params.arrayHeight)) );
	appendRow( ITEM("矩阵尺寸"), ITEM(QString("%1 x %2").arg(params.arrayWidth).arg(params.arrayHeight)) );
	
	//Pixel size
	if (params.pixelSize_mm[0] != 0 || params.pixelSize_mm[1] != 0)
		//appendRow( ITEM("Pixel size"), ITEM(QString("%1 x %2").arg(params.pixelSize_mm[0]).arg(params.pixelSize_mm[1])) );
		appendRow( ITEM("像素大小"), ITEM(QString("%1 x %2").arg(params.pixelSize_mm[0]).arg(params.pixelSize_mm[1])) );

	//Field of view
	//appendRow( ITEM("F.o.v. (deg.)"), ITEM(QString::number(params.vFOV_rad * CC_RAD_TO_DEG)) );
	appendRow( ITEM("视场角 (deg.)"), ITEM(QString::number(params.vFOV_rad * CC_RAD_TO_DEG)) );

	//Skewness
	//appendRow( ITEM("Skew"), ITEM(QString::number(params.skew)) );
	appendRow( ITEM("错切"), ITEM(QString::number(params.skew)) );

	//addSeparator("Frustrum display");
	addSeparator("视锥体显示");

	//Draw frustrum
	//appendRow( ITEM("Show lines"), CHECKABLE_ITEM(_obj->frustrumIsDrawn(), OBJECT_SENSOR_DRAW_FRUSTRUM) );
	//appendRow( ITEM("Show side planes"), CHECKABLE_ITEM(_obj->frustrumPlanesAreDrawn(), OBJECT_SENSOR_DRAW_FRUSTRUM_PLANES) );
	appendRow( ITEM("显示直线"), CHECKABLE_ITEM(_obj->frustrumIsDrawn(), OBJECT_SENSOR_DRAW_FRUSTRUM) );
	appendRow( ITEM("显示侧面"), CHECKABLE_ITEM(_obj->frustrumPlanesAreDrawn(), OBJECT_SENSOR_DRAW_FRUSTRUM_PLANES) );

	//Positions
	fillWithSensor(_obj);
}

void ccPropertiesTreeDelegate::fillWithMaterialSet(ccMaterialSet* _obj)
{
	assert(_obj && m_model);

	addSeparator("Material set");

	//Count
	appendRow( ITEM("Count"), ITEM(QString::number(_obj->size())) );

	//ccMaterialSet objects are 'shareable'
	fillWithShareable(_obj);
}

void ccPropertiesTreeDelegate::fillWithShareable(CCShareable* _obj)
{
	assert(_obj && m_model);

	addSeparator("Array");

	//Link count
	unsigned linkCount = _obj->getLinkCount(); //if we display it, it means it is a member of the DB --> i.e. link is already >1
	appendRow( ITEM("Shared"), ITEM(linkCount < 3 ? QString("No") : QString("Yes (%1)").arg(linkCount-1)) );
}

template<int N, class ElementType> void ccPropertiesTreeDelegate::fillWithChunkedArray(ccChunkedArray<N,ElementType>* _obj)
{
	assert(_obj && m_model);

	addSeparator("Array");

	//Name
	appendRow( ITEM("Name"), ITEM(_obj->getName().isEmpty() ? "undefined" : _obj->getName() ) );

	//Count
	appendRow( ITEM("Elements"), ITEM(QLocale(QLocale::English).toString(_obj->currentSize()) ) );

	//Capacity
	appendRow( ITEM("Capacity"), ITEM(QLocale(QLocale::English).toString(_obj->capacity()) ) );

	//Memory
	appendRow( ITEM("Memory"), ITEM(QString("%1 Mb").arg((double)_obj->memory()/1048576.0,0,'f',2)) );

	//ccChunkedArray objects are 'shareable'
	fillWithShareable(_obj);
}

void ccPropertiesTreeDelegate::fillWithMarkedObject(MarkedObject* _obj)
{
    assert(_obj && m_model);

    addSeparator(QString::fromAscii("标记属性"));

    //标记物体的公共属性
    appendRow(ITEM(QString::fromAscii("类别")), ITEM(_obj->getMarkedType()->getName()));
    appendRow(ITEM(QString::fromAscii("名称")), ITEM(_obj->getName()));
    QColor color = _obj->getColor();
    appendRow(ITEM(QString::fromAscii("颜色")), ITEM(QString("%1,%2,%3").arg(color.red()).arg(color.green()).arg(color.blue())));

    //不同种类（点、线、区域）标记物体的特殊属性
    //const int precision = 6; //坐标显示精度
    if (MarkedPoint *markedPoint = dynamic_cast<MarkedPoint*>(_obj))
    {
        QVector3D point = markedPoint->getPoint();
        appendRow(ITEM(QString::fromAscii("坐标")), ITEM(QString("X: %1\nY: %2\nZ: %3").arg(QString::number(point.x())).arg(QString::number(point.y())).arg(QString::number(point.z()))));
        return;
    }
    else if (MarkedLine *markedLine = dynamic_cast<MarkedLine*>(_obj))
    {
        appendRow(ITEM(QString::fromAscii("长度")), ITEM(QString::number(markedLine->getLineLength())));
        
        //起点坐标
        QVector3D startPoint = markedLine->getStartPoint();
        appendRow(ITEM(QString::fromAscii("起点坐标")), ITEM(QString("X: %1\nY: %2\nZ: %3").arg(QString::number(startPoint.x())).arg(QString::number(startPoint.y())).arg(QString::number(startPoint.z()))));

        //终点坐标
        QVector3D endPoint = markedLine->getEndPoint();
        appendRow(ITEM(QString::fromAscii("终点坐标")), ITEM(QString("X: %1\nY: %2\nZ: %3").arg(QString::number(endPoint.x())).arg(QString::number(endPoint.y())).arg(QString::number(endPoint.z()))));
        return;
    }
    else if (MarkedArea *markedArea = dynamic_cast<MarkedArea*>(_obj))
    {
        appendRow(ITEM(QString::fromAscii("周长")), ITEM(QString::number(markedArea->getBoundaryLength())));

        appendRow(ITEM(QString::fromAscii("面积")), ITEM(QString::number(markedArea->getArea())));

        //中心点坐标
        QVector3D centerPoint = markedArea->getCenterPoint();
        appendRow(ITEM(QString::fromAscii("中心点坐标")), ITEM(QString("X: %1\nY: %2\nZ: %3").arg(QString::number(centerPoint.x())).arg(QString::number(centerPoint.y())).arg(QString::number(centerPoint.z()))));

        //拟合平面法向量
        QVector3D projectPlaneNormal = markedArea->getProjectPlaneNormal();
        appendRow(ITEM(QString::fromAscii("拟合平面法向量")), ITEM(QString("X: %1\nY: %2\nZ: %3").arg(QString::number(projectPlaneNormal.x())).arg(QString::number(projectPlaneNormal.y())).arg(QString::number(projectPlaneNormal.z()))));
        return;
    }
    else if (MarkedObjectBag *markedObjectBag = dynamic_cast<MarkedObjectBag*>(_obj))
    {
        //子物体类型
        QString typeString;
        switch(markedObjectBag->getType())
        {
        case MarkedObjectBag::POINT:
            typeString = "点";
            break;
        case MarkedObjectBag::LINE:
            typeString = "线";
            break;
        case MarkedObjectBag::AREA:
            typeString = "面";
            break;
        }
        appendRow(ITEM(QString::fromAscii("子物体类型")), ITEM(typeString));

        appendRow(ITEM(QString::fromAscii("子物体数量")), ITEM(QString::number(markedObjectBag->size())));
        return;
    }
}

bool ccPropertiesTreeDelegate::isWideEditor(int itemData) const
{
	switch (itemData)
	{
	case OBJECT_CLOUD_SF_EDITOR:
	case OBJECT_SENSOR_MATRIX_EDITOR:
	case OBJECT_HISTORY_MATRIX_EDITOR:
	case TREE_VIEW_HEADER:
		return true;
	default:
		break;
	}

	return false;
}

QWidget* ccPropertiesTreeDelegate::createEditor(QWidget *parent,
												const QStyleOptionViewItem &option,
												const QModelIndex &index) const
{
	if (!m_model || !m_currentObject)
		return NULL;

	QStandardItem* item = m_model->itemFromIndex(index);

	if (!item || !item->data().isValid())
		return NULL;

	int itemData = item->data().toInt();
	if (item->column() == 0 && !isWideEditor(itemData))
	{
		//on the first column, only editors spanning on 2 columns are allowed
		return NULL;
	}

	QWidget* outputWidget = 0;

	switch (itemData)
	{
	case OBJECT_CURRENT_DISPLAY:
		{
			QComboBox *comboBox = new QComboBox(parent);

			std::vector<ccGLWindow*> glWindows;
			MainWindow::GetGLWindows(glWindows);

			comboBox->addItem(c_noneString);

			for (unsigned i=0; i<glWindows.size(); ++i)
				comboBox->addItem(glWindows[i]->windowTitle());

			connect(comboBox, SIGNAL(currentIndexChanged(const QString)), this, SLOT(objectDisplayChanged(const QString&)));

			outputWidget = comboBox;
		}
		break;
	case OBJECT_CURRENT_SCALAR_FIELD:
		{
			ccPointCloud* cloud = ccHObjectCaster::ToPointCloud(m_currentObject);
			assert(cloud);

			QComboBox *comboBox = new QComboBox(parent);

			comboBox->addItem(QString("None"));
			int nsf = cloud->getNumberOfScalarFields();
			for (int i=0; i<nsf; ++i)
				comboBox->addItem(QString(cloud->getScalarFieldName(i)));

			connect(comboBox, SIGNAL(activated(int)), this, SLOT(scalarFieldChanged(int)));

			outputWidget = comboBox;
		}
		break;
	case OBJECT_CURRENT_COLOR_RAMP:
		{
			ccColorScaleSelector* selector = new ccColorScaleSelector(ccColorScalesManager::GetUniqueInstance(),parent,QString::fromUtf8(":/CC/images/ccGear.png"));
			//fill combobox box with Color Scales Manager
			selector->init();
			connect(selector, SIGNAL(colorScaleSelected(int)), this, SLOT(colorScaleChanged(int)));
			connect(selector, SIGNAL(colorScaleEditorSummoned()), this, SLOT(spawnColorRampEditor()));

			outputWidget = selector;
		}
		break;
	case OBJECT_COLOR_RAMP_STEPS:
		{
			QSpinBox *spinBox = new QSpinBox(parent);
			spinBox->setRange(ccColorScale::MIN_STEPS,ccColorScale::MAX_STEPS);
			spinBox->setSingleStep(4);

			connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(colorRampStepsChanged(int)));

			outputWidget = spinBox;
		}
		break;
	case OBJECT_CLOUD_SF_EDITOR:
		{
			sfEditDlg* sfd = new sfEditDlg(parent);

			//DGM: why does this widget can't follow its 'policy' ?!
			//QSizePolicy pol = sfd->sizePolicy();
			//QSizePolicy::Policy hpol = pol.horizontalPolicy();
			//sfd->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);
			//parent->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);

			connect(sfd, SIGNAL(entitySFHasChanged()), this, SLOT(updateDisplay()));

			outputWidget = sfd;
		}
		break;
	case OBJECT_HISTORY_MATRIX_EDITOR:
	case OBJECT_SENSOR_MATRIX_EDITOR:
		{
			MatrixDisplayDlg* mdd = new MatrixDisplayDlg(parent);

			//no signal connection, it's a display-only widget

			outputWidget = mdd;
		}
		break;
	case TREE_VIEW_HEADER:
		{
			QLabel* headerLabel = new QLabel(parent);
			headerLabel->setStyleSheet(SEPARATOR_STYLESHEET);

			//no signal connection, it's a display-only widget

			outputWidget = headerLabel;
		}
		break;
	case OBJECT_OCTREE_TYPE:
		{
			QComboBox *comboBox = new QComboBox(parent);

			for (unsigned char i=0; i<OCTREE_DISPLAY_TYPE_NUMBERS; ++i)
				comboBox->addItem(COCTREE_DISPLAY_TYPE_TITLES[i]);

			connect(comboBox, SIGNAL(activated(int)), this, SLOT(octreeDisplayTypeChanged(int)));

			outputWidget = comboBox;
		}
		break;
	case OBJECT_OCTREE_LEVEL:
		{
			QSpinBox *spinBox = new QSpinBox(parent);
			spinBox->setRange(1,CCLib::DgmOctree::MAX_OCTREE_LEVEL);

			connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(octreeDisplayedLevelChanged(int)));

			outputWidget = spinBox;
		}
		break;
	case OBJECT_PRIMITIVE_PRECISION:
		{
			QSpinBox *spinBox = new QSpinBox(parent);
			spinBox->setRange(4,360);
			spinBox->setSingleStep(4);

			connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(primitivePrecisionChanged(int)));

			outputWidget = spinBox;
		}
		break;
	case OBJECT_SPHERE_RADIUS:
		{
			QDoubleSpinBox *spinBox = new QDoubleSpinBox(parent);
			spinBox->setDecimals(6);
			spinBox->setRange(0,1.0e6);
			spinBox->setSingleStep(1.0);

			connect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(sphereRadiusChanged(double)));

			outputWidget = spinBox;
		}
		break;
	case OBJECT_CONE_HEIGHT:
		{
			QDoubleSpinBox *spinBox = new QDoubleSpinBox(parent);
			spinBox->setDecimals(6);
			spinBox->setRange(0,1.0e6);
			spinBox->setSingleStep(1.0);

			connect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(coneHeightChanged(double)));

			outputWidget = spinBox;
		}
		break;
	case OBJECT_CONE_BOTTOM_RADIUS:
		{
			QDoubleSpinBox *spinBox = new QDoubleSpinBox(parent);
			spinBox->setDecimals(6);
			spinBox->setRange(0,1.0e6);
			spinBox->setSingleStep(1.0);

			connect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(coneBottomRadiusChanged(double)));

			outputWidget = spinBox;
		}
		break;
	case OBJECT_CONE_TOP_RADIUS:
		{
			QDoubleSpinBox *spinBox = new QDoubleSpinBox(parent);
			spinBox->setDecimals(6);
			spinBox->setRange(0,1.0e6);
			spinBox->setSingleStep(1.0);

			connect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(coneTopRadiusChanged(double)));

			outputWidget = spinBox;
		}
		break;
	case OBJECT_IMAGE_ALPHA:
		{
			QSlider* slider = new QSlider(Qt::Horizontal,parent);
			slider->setRange(0,255);
			slider->setSingleStep(1);
			slider->setPageStep(16);
			slider->setTickPosition(QSlider::NoTicks);
			connect(slider, SIGNAL(valueChanged(int)), this, SLOT(imageAlphaChanged(int)));

			outputWidget = slider;
		}
		break;
	case OBJECT_SENSOR_INDEX:
		{
			ccSensor* sensor = ccHObjectCaster::ToSensor(m_currentObject);
			assert(sensor);

			double minIndex, maxIndex;
			sensor->getIndexBounds(minIndex, maxIndex);

			QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
			spinBox->setRange(minIndex, maxIndex);
			spinBox->setSingleStep((maxIndex-minIndex)/1000.0);

			connect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(sensorIndexChanged(double)));

			outputWidget = spinBox;
		}
		break;
	case OBJECT_TRANS_BUFFER_TRIHDERONS_SCALE:
		{
			QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
			spinBox->setRange(1.0e-3, 1.0e6);
			spinBox->setDecimals(3);
			spinBox->setSingleStep(1.0);

			connect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(trihedronsScaleChanged(double)));

			outputWidget = spinBox;
		}
		break;
	case OBJECT_APPLY_IMAGE_VIEWPORT:
		{
			QPushButton* button = new QPushButton("Apply",parent);
			connect(button, SIGNAL(clicked()), this, SLOT(applyImageViewport()));

			button->setMinimumHeight(30);
			
			outputWidget = button;
		}
		break;
	case OBJECT_APPLY_SENSOR_VIEWPORT:
		{
			QPushButton* button = new QPushButton("Apply",parent);
			connect(button, SIGNAL(clicked()), this, SLOT(applySensorViewport()));

			button->setMinimumHeight(30);

			outputWidget = button;
		}
		break;
	case OBJECT_APPLY_LABEL_VIEWPORT:
		{
			QPushButton* button = new QPushButton("Apply",parent);
			connect(button, SIGNAL(clicked()), this, SLOT(applyLabelViewport()));

			button->setMinimumHeight(30);
			outputWidget = button;
		}
		break;
	case OBJECT_SENSOR_DISPLAY_SCALE:
		{
			QDoubleSpinBox *spinBox = new QDoubleSpinBox(parent);
			spinBox->setRange(1.0e-3,1.0e6);
			spinBox->setDecimals(3);
			spinBox->setSingleStep(1.0e-1);

			connect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(sensorScaleChanged(double)));

			outputWidget = spinBox;
		}
		break;
	case OBJECT_CLOUD_POINT_SIZE:
		{
			QComboBox *comboBox = new QComboBox(parent);

			comboBox->addItem(c_defaultPointSizeString); //size = 0
			for (unsigned i=1;i<=10;++i)
				comboBox->addItem(QString::number(i));

			connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(cloudPointSizeChanged(int)));

			outputWidget = comboBox;
		}
		break;
	case OBJECT_POLYLINE_WIDTH:
		{
			QComboBox *comboBox = new QComboBox(parent);

			comboBox->addItem(c_defaultPolyWidthSizeString); //size = 0
			for (unsigned i=1; i<=10; ++i)
				comboBox->addItem(QString::number(i));

			connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(polyineWidthChanged(int)));

			outputWidget = comboBox;
		}
		break;
	case OBJECT_COLOR_SOURCE:
		{
			QComboBox *comboBox = new QComboBox(parent);

			comboBox->addItem(c_noneString);
			if (m_currentObject)
			{
				if (m_currentObject->hasColors())
					comboBox->addItem(s_rgbColor);
				if (m_currentObject->hasScalarFields())
					comboBox->addItem(s_sfColor);
				connect(comboBox, SIGNAL(currentIndexChanged(const QString)), this, SLOT(colorSourceChanged(const QString&)));
			}

			outputWidget = comboBox;
		}
		break;
	default:
		return QStyledItemDelegate::createEditor(parent, option, index);
	}

	if (outputWidget)
	{
		outputWidget->setFocusPolicy(Qt::StrongFocus); //Qt doc: << The returned editor widget should have Qt::StrongFocus >>
	}
	else
	{
		//shouldn't happen
		assert(false);
	}

	return outputWidget;
}

void ccPropertiesTreeDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyledItemDelegate::updateEditorGeometry(editor, option, index);

	if (!m_model || !editor)
		return;

	QStandardItem* item = m_model->itemFromIndex(index);

	if (item &&	item->data().isValid() && item->column() == 0)
	{
		if (isWideEditor(item->data().toInt()))
		{
			QWidget* widget = qobject_cast<QWidget*>(editor);
			if (!widget)
				return;
			//we must resize the SF edit widget so that it spans on both columns!
			QRect rect = m_view->visualRect(m_model->index(item->row(),1)); //second column width
			widget->resize(option.rect.width()+rect.width(),widget->height());
		}
	}
}

void SetDoubleSpinBoxValue(QWidget *editor, double value, bool keyboardTracking = false)
{
	QDoubleSpinBox* spinBox = qobject_cast<QDoubleSpinBox*>(editor);
	if (!spinBox)
	{
		assert(false);
		return;
	}
	spinBox->setKeyboardTracking(keyboardTracking);
	spinBox->setValue(value);
}

void SetSpinBoxValue(QWidget *editor, int value, bool keyboardTracking = false)
{
	QSpinBox* spinBox = qobject_cast<QSpinBox*>(editor);
	if (!spinBox)
	{
		assert(false);
		return;
	}
	spinBox->setKeyboardTracking(keyboardTracking);
	spinBox->setValue(value);
}

void SetComboBoxIndex(QWidget *editor, int index)
{
	QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
	if (!comboBox)
	{
		assert(false);
		return;
	}
	assert(index < 0 || index < comboBox->maxCount());
	comboBox->setCurrentIndex(index);
}

void ccPropertiesTreeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	if (!m_model || !m_currentObject)
		return;

	QStandardItem* item = m_model->itemFromIndex(index);

	if (!item || !item->data().isValid() || (item->column()==0 && !isWideEditor(item->data().toInt())))
		return;

	switch (item->data().toInt())
	{
	case OBJECT_CURRENT_DISPLAY:
		{
			QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
			if (!comboBox)
			{
				assert(false);
				return;
			}

			ccGLWindow* win = static_cast<ccGLWindow*>(m_currentObject->getDisplay());
			int pos = (win ? comboBox->findText(win->windowTitle()) : 0);

			comboBox->setCurrentIndex(std::max(pos,0)); //0 = "NONE"
			break;
		}
	case OBJECT_CURRENT_SCALAR_FIELD:
		{
			ccPointCloud* cloud = ccHObjectCaster::ToPointCloud(m_currentObject);
			assert(cloud);

			int pos = cloud->getCurrentDisplayedScalarFieldIndex();
			SetComboBoxIndex(editor,pos+1);
			break;
		}
	case OBJECT_CURRENT_COLOR_RAMP:
		{
			QFrame *selectorFrame = qobject_cast<QFrame*>(editor);
			if (!selectorFrame)
				return;
			ccColorScaleSelector* selector = static_cast<ccColorScaleSelector*>(selectorFrame);

			ccPointCloud* cloud = ccHObjectCaster::ToPointCloud(m_currentObject);
			assert(cloud);

			ccScalarField* sf = cloud->getCurrentDisplayedScalarField();
			if (sf)
			{
				if (sf->getColorScale())
					selector->setSelectedScale(sf->getColorScale()->getUuid());
				else
					selector->setSelectedScale(QString());
			}
			break;
		}
	case OBJECT_COLOR_RAMP_STEPS:
		{
			ccPointCloud* cloud = ccHObjectCaster::ToPointCloud(m_currentObject);
			assert(cloud);
			ccScalarField* sf = cloud ? cloud->getCurrentDisplayedScalarField() : 0;
			if (sf)
				SetSpinBoxValue(editor,sf->getColorRampSteps(),true);
			break;
		}
	case OBJECT_CLOUD_SF_EDITOR:
		{
			sfEditDlg *sfd = qobject_cast<sfEditDlg*>(editor);
			if (!sfd)
				return;

			ccPointCloud* cloud = ccHObjectCaster::ToPointCloud(m_currentObject);
			assert(cloud);

			ccScalarField* sf = cloud->getCurrentDisplayedScalarField();
			if (sf)
				sfd->fillDialogWith(sf);
			break;
		}
	case OBJECT_HISTORY_MATRIX_EDITOR:
		{
			MatrixDisplayDlg *mdd = qobject_cast<MatrixDisplayDlg*>(editor);
			if (!mdd)
				return;

			mdd->fillDialogWith(m_currentObject->getGLTransformationHistory());
			break;
		}
	case OBJECT_SENSOR_MATRIX_EDITOR:
		{
			MatrixDisplayDlg* mdd = qobject_cast<MatrixDisplayDlg*>(editor);
			if (!mdd)
				return;

			ccSensor* sensor = ccHObjectCaster::ToSensor(m_currentObject);
			assert(sensor);

			ccIndexedTransformation trans;
			if (sensor->getActiveAbsoluteTransformation(trans))
			{
				mdd->fillDialogWith(trans);
			}
			else
			{
				mdd->clear();
				mdd->setEnabled(false);
			}
			break;
		}
	case TREE_VIEW_HEADER:
		{
			QLabel* label = qobject_cast<QLabel*>(editor);
			if (label)
				label->setText(item->accessibleDescription());
			break;
		}
	case OBJECT_OCTREE_TYPE:
		{
			ccOctree* octree = ccHObjectCaster::ToOctree(m_currentObject);
			assert(octree);
			SetComboBoxIndex(editor,static_cast<int>(octree->getDisplayType()));
			break;
		}
	case OBJECT_OCTREE_LEVEL:
		{
			ccOctree* octree = ccHObjectCaster::ToOctree(m_currentObject);
			assert(octree);
			SetSpinBoxValue(editor,octree ? octree->getDisplayedLevel() : 0);
			break;
		}
	case OBJECT_PRIMITIVE_PRECISION:
		{
			ccGenericPrimitive* primitive = ccHObjectCaster::ToPrimitive(m_currentObject);
			assert(primitive);
			SetSpinBoxValue(editor,primitive ? primitive->getDrawingPrecision() : 0);
			break;
		}
	case OBJECT_SPHERE_RADIUS:
		{
			ccSphere* sphere = ccHObjectCaster::ToSphere(m_currentObject);
			assert(sphere);
			SetDoubleSpinBoxValue(editor,sphere ? sphere->getRadius() : 0.0);
			break;
		}
	case OBJECT_CONE_HEIGHT:
		{
			ccCone* cone = ccHObjectCaster::ToCone(m_currentObject);
			assert(cone);
			SetDoubleSpinBoxValue(editor,cone ? cone->getHeight() : 0.0);
			break;
		}
	case OBJECT_CONE_BOTTOM_RADIUS:
		{
			ccCone* cone = ccHObjectCaster::ToCone(m_currentObject);
			assert(cone);
			SetDoubleSpinBoxValue(editor,cone ? cone->getBottomRadius() : 0.0);
			break;
		}
	case OBJECT_CONE_TOP_RADIUS:
		{
			ccCone* cone = ccHObjectCaster::ToCone(m_currentObject);
			assert(cone);
			SetDoubleSpinBoxValue(editor,cone ? cone->getTopRadius() : 0.0);
			break;
		}
	case OBJECT_IMAGE_ALPHA:
		{
			QSlider *slider = qobject_cast<QSlider*>(editor);
			if (!slider)
				return;

			ccImage* image = ccHObjectCaster::ToImage(m_currentObject);
			assert(image);
			slider->setValue(static_cast<int>(image->getAlpha()*255.0f));
			//slider->setTickPosition(QSlider::NoTicks);
			break;
		}
	case OBJECT_SENSOR_INDEX:
		{
			ccSensor* sensor = ccHObjectCaster::ToSensor(m_currentObject);
			assert(sensor);
			SetDoubleSpinBoxValue(editor,sensor ? sensor->getActiveIndex() : 0.0);
			break;
		}
	case OBJECT_SENSOR_DISPLAY_SCALE:
		{
			ccSensor* sensor = ccHObjectCaster::ToSensor(m_currentObject);
			assert(sensor);
			SetDoubleSpinBoxValue(editor,sensor ? sensor->getGraphicScale() : 0.0);
			break;
		}
	case OBJECT_TRANS_BUFFER_TRIHDERONS_SCALE:
		{
			ccIndexedTransformationBuffer* buffer = ccHObjectCaster::ToTransBuffer(m_currentObject);
			assert(buffer);
			SetDoubleSpinBoxValue(editor,buffer ? buffer->triherdonsDisplayScale() : 0.0);
			break;
		}
	case OBJECT_CLOUD_POINT_SIZE:
		{
			ccGenericPointCloud* cloud = ccHObjectCaster::ToGenericPointCloud(m_currentObject);
			assert(cloud);
			SetComboBoxIndex(editor,static_cast<int>(cloud->getPointSize()));
			break;
		}
	case OBJECT_COLOR_SOURCE:
		{
			int currentIndex = 0; //no color
			int lastIndex = currentIndex;
			if (m_currentObject->hasColors())
			{
				++lastIndex;
				if (m_currentObject->colorsShown())
					currentIndex = lastIndex;
			}
			if (m_currentObject->hasScalarFields())
			{
				++lastIndex;
				if (m_currentObject->sfShown())
					currentIndex = lastIndex;
			}
			SetComboBoxIndex(editor,currentIndex);
			break;
		}
	default:
		QStyledItemDelegate::setEditorData(editor, index);
		break;
	}
}
//====================================================updateItem=====================================================//
void ccPropertiesTreeDelegate::updateItem(QStandardItem * item)
{
	if (!m_currentObject || item->column() == 0 || !item->data().isValid())
		return;

    bool redraw = false;
	switch (item->data().toInt())
	{
	case OBJECT_NAME:
		m_currentObject->setName(item->text());
		emit ccObjectPropertiesChanged(m_currentObject);
		break;
	case OBJECT_VISIBILITY:
		{
			bool objectWasDisplayed = m_currentObject->isDisplayed();
			m_currentObject->setVisible(item->checkState() == Qt::Checked);
			bool objectIsDisplayed = m_currentObject->isDisplayed();
			if (objectWasDisplayed != objectIsDisplayed)
			{
				if (m_currentObject->isGroup())
					emit ccObjectAndChildrenAppearanceChanged(m_currentObject);
				else
					emit ccObjectAppearanceChanged(m_currentObject);
			}
		}
		break;
	case OBJECT_NORMALS_SHOWN:
		m_currentObject->showNormals(item->checkState() == Qt::Checked);
		redraw = true;
		break;
	case OBJECT_MATERIALS:
		{
			ccGenericMesh* mesh = ccHObjectCaster::ToGenericMesh(m_currentObject);
			assert(mesh);
			mesh->showMaterials(item->checkState() == Qt::Checked);
		}
		redraw = true;
		break;
	case OBJECT_SF_SHOW_SCALE:
		{
			ccPointCloud* cloud = ccHObjectCaster::ToPointCloud(m_currentObject);
			assert(cloud);
			cloud->showSFColorsScale(item->checkState() == Qt::Checked);
		}
		redraw = true;
		break;
	case OBJECT_FACET_CONTOUR:
		{
			ccFacet* facet = ccHObjectCaster::ToFacet(m_currentObject);
			assert(facet);
			if (facet && facet->getContour())
				facet->getContour()->setVisible(item->checkState() == Qt::Checked);
		}
		redraw = true;
		break;
	case OBJECT_FACET_MESH:
		{
			ccFacet* facet = ccHObjectCaster::ToFacet(m_currentObject);
			assert(facet);
			if (facet && facet->getPolygon())
				facet->getPolygon()->setVisible(item->checkState() == Qt::Checked);
		}
		redraw = true;
		break;
	case OBJECT_FACET_NORMAL_VECTOR:
		{
			ccFacet* facet = ccHObjectCaster::ToFacet(m_currentObject);
			assert(facet);
			if (facet)
				facet->showNormalVector(item->checkState() == Qt::Checked);
		}
		redraw = true;
		break;
	case OBJECT_MESH_WIRE:
		{
			ccGenericMesh* mesh = ccHObjectCaster::ToGenericMesh(m_currentObject);
			assert(mesh);
			mesh->showWired(item->checkState() == Qt::Checked);
		}
		redraw = true;
		break;
	case OBJECT_MESH_STIPPLING:
		{
			ccGenericMesh* mesh = ccHObjectCaster::ToGenericMesh(m_currentObject);
			assert(mesh);
			mesh->enableStippling(item->checkState() == Qt::Checked);
		}
		redraw = true;
		break;
	case OBJECT_LABEL_DISP_2D:
		{
			cc2DLabel* label = ccHObjectCaster::To2DLabel(m_currentObject);
			assert(label);
			label->setDisplayedIn2D(item->checkState() == Qt::Checked);
		}
		redraw = true;
		break;
	case OBJECT_LABEL_DISP_3D:
		{
			cc2DLabel* label = ccHObjectCaster::To2DLabel(m_currentObject);
			assert(label);
			label->setDisplayedIn3D(item->checkState() == Qt::Checked);
		}
		redraw = true;
		break;
	case OBJECT_NAME_IN_3D:
		m_currentObject->showNameIn3D(item->checkState() == Qt::Checked);
		redraw = true;
		break;
	case OBJECT_SHOW_TRANS_BUFFER_PATH:
		{
			ccIndexedTransformationBuffer* buffer = ccHObjectCaster::ToTransBuffer(m_currentObject);
			assert(buffer);
			buffer->showPathAsPolyline(item->checkState() == Qt::Checked);
		}
		redraw = true;
		break;
	case OBJECT_SHOW_TRANS_BUFFER_TRIHDERONS:
		{
			ccIndexedTransformationBuffer* buffer = ccHObjectCaster::ToTransBuffer(m_currentObject);
			assert(buffer);
			buffer->showTriherdons(item->checkState() == Qt::Checked);
		}
		redraw = true;
		break;
	case OBJECT_SENSOR_DRAW_FRUSTRUM:
		{
			ccCameraSensor* sensor = ccHObjectCaster::ToCameraSensor(m_currentObject);
			sensor->drawFrustrum(item->checkState() == Qt::Checked);
		}
		redraw = true;
		break;
	case OBJECT_SENSOR_DRAW_FRUSTRUM_PLANES:
		{
			ccCameraSensor* sensor = ccHObjectCaster::ToCameraSensor(m_currentObject);
			sensor->drawFrustrumPlanes(item->checkState() == Qt::Checked);
		}
		redraw = true;
		break;
	}

	if (redraw)
		updateDisplay();
}

void ccPropertiesTreeDelegate::updateDisplay()
{
	ccHObject* object = m_currentObject;
	if (!object)
		return;

	bool objectIsDisplayed = object->isDisplayed();
	if (!objectIsDisplayed)
	{
		//DGM: point clouds may be mesh vertices of meshes which may depend on several of their parameters
		if (object->isKindOf(CC_TYPES::POINT_CLOUD))
		{
			ccHObject* parent = object->getParent();
			if (parent && parent->isKindOf(CC_TYPES::MESH) && parent->isDisplayed()) //specific case: vertices
			{
				object = parent;
				objectIsDisplayed = true;
			}
		}
	}

	if (objectIsDisplayed)
	{
		if (object->isGroup())
			emit ccObjectAndChildrenAppearanceChanged(m_currentObject);
		else
			emit ccObjectAppearanceChanged(m_currentObject);
	}
}

void ccPropertiesTreeDelegate::updateModel()
{
	//simply re-fill model!
	fillModel(m_currentObject);
}

void ccPropertiesTreeDelegate::scalarFieldChanged(int pos)
{
	if (!m_currentObject)
		return;

	ccPointCloud* cloud = ccHObjectCaster::ToPointCloud(m_currentObject);
	if (cloud && cloud->getCurrentDisplayedScalarFieldIndex()+1 != pos)
	{
		cloud->setCurrentDisplayedScalarField(pos-1);
		cloud->showSF(pos>0);

		updateDisplay();
		//we must also reset the properties display!
		updateModel();
	}
}

void ccPropertiesTreeDelegate::spawnColorRampEditor()
{
	if (!m_currentObject)
		return;

	ccPointCloud* cloud = ccHObjectCaster::ToPointCloud(m_currentObject);
	assert(cloud);
	ccScalarField* sf = (cloud ? static_cast<ccScalarField*>(cloud->getCurrentDisplayedScalarField()) : 0);
	if (sf)
	{
		ccColorScaleEditorDialog* editorDialog = new ccColorScaleEditorDialog(ccColorScalesManager::GetUniqueInstance(),sf->getColorScale(),static_cast<ccGLWindow*>(cloud->getDisplay()));
		editorDialog->setAssociatedScalarField(sf);
		if (editorDialog->exec())
		{
			if (editorDialog->getActiveScale())
			{
				sf->setColorScale(editorDialog->getActiveScale());
				updateDisplay();
			}

			//save current scale manager state to persistent settings
			ccColorScalesManager::GetUniqueInstance()->toPersistentSettings();

			updateModel();
		}
	}
}

void ccPropertiesTreeDelegate::colorScaleChanged(int pos)
{
	if (!m_currentObject)
		return;

	if (pos < 0)
	{
		assert(false);
		return;
	}

	ccColorScaleSelector* selector = dynamic_cast<ccColorScaleSelector*>(QObject::sender());
	if (!selector)
		return;

	ccColorScale::Shared colorScale = selector->getScale(pos);
	if (!colorScale)
	{
		ccLog::Error("Internal error: color scale doesn't seem to exist anymore!");
		return;
	}

	//get current SF
	ccPointCloud* cloud = ccHObjectCaster::ToPointCloud(m_currentObject);
	assert(cloud);
	ccScalarField* sf = cloud ? static_cast<ccScalarField*>(cloud->getCurrentDisplayedScalarField()) : 0;
	if (sf && sf->getColorScale() != colorScale)
	{
		sf->setColorScale(colorScale);
		updateDisplay();
		updateModel();
	}
}

void ccPropertiesTreeDelegate::colorRampStepsChanged(int pos)
{
	if (!m_currentObject)
		return;

	ccPointCloud* cloud = ccHObjectCaster::ToPointCloud(m_currentObject);
	assert(cloud);
	ccScalarField* sf = static_cast<ccScalarField*>(cloud->getCurrentDisplayedScalarField());
	if (sf)
	{
		sf->setColorRampSteps(pos);
		updateDisplay();
	}
}

void ccPropertiesTreeDelegate::octreeDisplayTypeChanged(int pos)
{
	if (!m_currentObject)
		return;

	ccOctree* octree = ccHObjectCaster::ToOctree(m_currentObject);
	assert(octree);

	octree->setDisplayType(OCTREE_DISPLAY_TYPE_ENUMS[pos]);
	updateDisplay();
}

void ccPropertiesTreeDelegate::octreeDisplayedLevelChanged(int val)
{
	if (!m_currentObject)
		return;

	ccOctree* octree = ccHObjectCaster::ToOctree(m_currentObject);
	assert(octree);

	if (octree->getDisplayedLevel() != val) //to avoid infinite loops!
	{
		octree->setDisplayedLevel(val);
		updateDisplay();
		//we must also reset the properties display!
		updateModel();
	}
}

void ccPropertiesTreeDelegate::primitivePrecisionChanged(int val)
{
	if (!m_currentObject)
		return;

	ccGenericPrimitive* primitive = ccHObjectCaster::ToPrimitive(m_currentObject);
	assert(primitive);

	if (static_cast<unsigned int>(val) == primitive->getDrawingPrecision())
		return;

	bool wasVisible = primitive->isVisible();
	primitive->setDrawingPrecision(val);
	primitive->setVisible(wasVisible);

	updateDisplay();
	//we must also reset the properties display!
	updateModel();
}

void ccPropertiesTreeDelegate::sphereRadiusChanged(double val)
{
	if (!m_currentObject)
		return;

	ccSphere* sphere = ccHObjectCaster::ToSphere(m_currentObject);
	assert(sphere);

	PointCoordinateType radius = static_cast<PointCoordinateType>(val);
	if (radius == sphere->getRadius())
		return;

	bool wasVisible = sphere->isVisible();
	sphere->setRadius(radius);
	sphere->setVisible(wasVisible);

	updateDisplay();
	//we must also reset the properties display!
	updateModel();
}

void ccPropertiesTreeDelegate::coneHeightChanged(double val)
{
	if (!m_currentObject)
		return;

	ccCone* cone = ccHObjectCaster::ToCone(m_currentObject);
	assert(cone);

	PointCoordinateType height = static_cast<PointCoordinateType>(val);
	if (height == cone->getHeight())
		return;

	bool wasVisible = cone->isVisible();
	cone->setHeight(height);
	cone->setVisible(wasVisible);

	updateDisplay();
	//we must also reset the properties display!
	updateModel();
}

void ccPropertiesTreeDelegate::coneBottomRadiusChanged(double val)
{
	if (!m_currentObject)
		return;

	ccCone* cone = ccHObjectCaster::ToCone(m_currentObject);
	assert(cone);

	PointCoordinateType radius = static_cast<PointCoordinateType>(val);
	if (radius == cone->getBottomRadius())
		return;

	bool wasVisible = cone->isVisible();
	cone->setBottomRadius(radius); //works for both the bottom and top radii for cylinders!
	cone->setVisible(wasVisible);

	updateDisplay();
	//we must also reset the properties display!
	updateModel();
}

void ccPropertiesTreeDelegate::coneTopRadiusChanged(double val)
{
	if (!m_currentObject)
		return;

	ccCone* cone = ccHObjectCaster::ToCone(m_currentObject);
	assert(cone);

	PointCoordinateType radius = static_cast<PointCoordinateType>(val);
	if (radius == cone->getTopRadius())
		return;

	bool wasVisible = cone->isVisible();
	cone->setTopRadius(radius); //works for both the bottom and top radii for cylinders!
	cone->setVisible(wasVisible);

	updateDisplay();
	//we must also reset the properties display!
	updateModel();
}

void ccPropertiesTreeDelegate::imageAlphaChanged(int val)
{
	ccImage* image = ccHObjectCaster::ToImage(m_currentObject);
	if (!image)
		return;
	image->setAlpha(static_cast<float>(val)/255.0f);

	updateDisplay();
}

void ccPropertiesTreeDelegate::applyImageViewport()
{
	if (!m_currentObject)
		return;

	ccImage* image = ccHObjectCaster::ToImage(m_currentObject);
	assert(image);

	if (image->getAssociatedSensor() && image->getAssociatedSensor()->applyViewport())
	{
		ccLog::Print("[ApplyImageViewport] Viewport applied");
	}
}

void ccPropertiesTreeDelegate::applySensorViewport()
{
	if (!m_currentObject)
		return;

	ccSensor* sensor = ccHObjectCaster::ToSensor(m_currentObject);
	assert(sensor);

	if (sensor->applyViewport())
	{
		ccLog::Print("[ApplySensorViewport] Viewport applied");
	}
}

void ccPropertiesTreeDelegate::applyLabelViewport()
{
	if (!m_currentObject)
		return;

	cc2DViewportObject* viewport = ccHObjectCaster::To2DViewportObject(m_currentObject);
	assert(viewport);

	ccGLWindow* win = MainWindow::GetActiveGLWindow();
	if (!win)
		return;

	win->setViewportParameters(viewport->getParameters());
	win->redraw();
}

void ccPropertiesTreeDelegate::sensorScaleChanged(double val)
{
	if (!m_currentObject)
		return;

	ccSensor* sensor = ccHObjectCaster::ToSensor(m_currentObject);
	assert(sensor);

	sensor->setGraphicScale(static_cast<PointCoordinateType>(val));
	updateDisplay();
}

void ccPropertiesTreeDelegate::sensorIndexChanged(double val)
{
	if (!m_currentObject)
		return;

	ccSensor* sensor = ccHObjectCaster::ToSensor(m_currentObject);
	assert(sensor);

	sensor->setActiveIndex(val);
	updateDisplay();
}

void ccPropertiesTreeDelegate::trihedronsScaleChanged(double val)
{
	if (!m_currentObject)
		return;

	ccIndexedTransformationBuffer* buffer = ccHObjectCaster::ToTransBuffer(m_currentObject);
	assert(buffer);

	buffer->setTriherdonsDisplayScale(static_cast<float>(val));
	if (buffer->triherdonsShown())
		updateDisplay();
}

void ccPropertiesTreeDelegate::cloudPointSizeChanged(int size)
{
	if (!m_currentObject)
		return;

	ccGenericPointCloud* cloud = ccHObjectCaster::ToGenericPointCloud(m_currentObject);
	assert(cloud);

	cloud->setPointSize(size);
	updateDisplay();
}

void ccPropertiesTreeDelegate::polyineWidthChanged(int size)
{
	if (!m_currentObject)
		return;

	ccPolyline* pline = ccHObjectCaster::ToPolyline(m_currentObject);
	assert(pline);

	if (pline)
		pline->setWidth(static_cast<PointCoordinateType>(size));

	updateDisplay();
}

void ccPropertiesTreeDelegate::objectDisplayChanged(const QString& newDisplayTitle)
{
	if (!m_currentObject)
		return;

	QString actualDisplayTitle;

	ccGLWindow* win = static_cast<ccGLWindow*>(m_currentObject->getDisplay());
	if (win)
		actualDisplayTitle = win->windowTitle();
	else
		actualDisplayTitle = c_noneString;

	if (actualDisplayTitle != newDisplayTitle)
	{
		//we first mark the "old displays" before removal,
		//to be sure that they will also be redrawn!
		m_currentObject->prepareDisplayForRefresh_recursive();

		ccGLWindow* win = MainWindow::GetGLWindow(newDisplayTitle);
		m_currentObject->setDisplay_recursive(win);
		if (win)
		{
			m_currentObject->prepareDisplayForRefresh_recursive();
			win->zoomGlobal();
		}

		MainWindow::RefreshAllGLWindow(false);
	}
}

void ccPropertiesTreeDelegate::colorSourceChanged(const QString & source)
{
	if (!m_currentObject)
		return;

	if (source == c_noneString)
	{
		m_currentObject->showColors(false);
		m_currentObject->showSF(false);
	}
	else if (source == s_rgbColor)
	{
		m_currentObject->showColors(true);
		m_currentObject->showSF(false);
	}
	else if (source == s_sfColor)
	{
		m_currentObject->showColors(false);
		m_currentObject->showSF(true);
	}
	else
	{
		assert(false);
	}

	updateDisplay();
}