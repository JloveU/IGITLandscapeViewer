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

#ifndef CC_DB_ROOT_HEADER
#define CC_DB_ROOT_HEADER

//Qt
#include <QAbstractItemModel>
#include <QItemSelection>
#include <QPoint>
#include <QTreeView>

//CCLib
#include <CCConst.h>

//qCC_db
#include <ccObject.h>
#include <ccHObject.h>
#include <ccDrawableObject.h>

//System
#include <string.h>
#include <set>

class QStandardItemModel;
class QAction;
class ccPropertiesTreeDelegate;
class ccHObject;
class ccGLWindow;

//! Precise statistics about current selection
struct dbTreeSelectionInfo{
	int selCount;

	int sfCount;// scalar field�ĸ���
	int colorCount; // ��ɫ�ĸ���
	int normalsCount;//����������
	int octreeCount; //octree����
	int cloudCount; // ���Ƹ���
	int groupCount; // group �ĸ���
	int polylineCount; //polyline �ĸ���
	int meshCount; // mesh �ĸ���
	int imageCount; // ͼ��ĸ���
	int sensorCount; // �������ĸ���
	int gblSensorCount; 
	int cameraSensorCount; // ����������ĸ���
	int kdTreeCount; // kdtree�ĸ���

	void reset(){
		memset(this,0,sizeof(dbTreeSelectionInfo)); //���в������ó�0
	}
};

//! Custom QTreeView widget (for advanced selection behavior)
class ccCustomQTreeView : public QTreeView
{
public:

	//! Default constructor
	ccCustomQTreeView(QWidget* parent) : QTreeView(parent) {}

protected:

	//inherited from QTreeView //ѡ����
	virtual QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex& index, const QEvent* event=0) const;
};

//data Base Root
//! GUI database tree root
class ccDBRoot : public QAbstractItemModel
{
	Q_OBJECT

public:

	//! Default constructor
	/** \param dbTreeWidget widget for DB tree display //��ʾ�����ι�ϵ
		\param propertiesTreeWidget widget for selected entity's properties tree display //��ʾ����
		\param parent widget QObject parent
	**/
	ccDBRoot(ccCustomQTreeView* dbTreeWidget, QTreeView* propertiesTreeWidget, QObject* parent = 0);

	//! Destructor
	virtual ~ccDBRoot();

	//! Returns associated root object //���ظ��ڵ�
	ccHObject* getRootEntity();

	//! Hides properties view   //����������ͼ
	void hidePropertiesView();

	//! Updates properties view //����������ͼ
	void updatePropertiesView();

	//! Adds an element to the DB tree  //������嵽DB Tree
	void addElement(ccHObject* anObject, bool autoExpand = true);

	//! Removes an element from the DB tree  //ɾ��Ԫ��
	void removeElement(ccHObject* anObject);

	//! Finds an element in DB     //DB Tree�в�������
	ccHObject* find(int uniqueID) const;

	//! Returns the number of selected entities in DB tree (optionally with a given type) //����DBTree��ѡ�е�����ĸ���
	int countSelectedEntities(CC_CLASS_ENUM filter = CC_TYPES::OBJECT); 

	//! Returns selected entities in DB tree (optionally with a given type and additional information)
	int getSelectedEntities(ccHObject::Container& selEntities,
							CC_CLASS_ENUM filter = CC_TYPES::OBJECT,
							dbTreeSelectionInfo* info = NULL); //����ѡ�񵽵�����(�����̶�����������)

	//! Expands tree at a given node// ������������
	void expandElement(ccHObject* anObject, bool state);

	//! Selects a given entity
	/** If ctrl is pressed by the user at the same time,
		previous selection will be simply updated accordingly.
		\param obj entity to select
		\param forceAdditiveSelection whether to force additive selection (just as if CTRL key is pressed) or not
	**/
	//ѡ������
	void selectEntity(ccHObject* obj, bool forceAdditiveSelection = false);

	//! Unselects a given entity //ȡ��ѡ������
	void unselectEntity(ccHObject* obj);

	//! Unselects all entities // ȡ�����е�ѡ��
	void unselectAllEntities();

	//! Unloads all entities //ж�����е�����
	void unloadAll();

	//inherited from QAbstractItemModel
	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual QModelIndex index(int row, int column, const QModelIndex &parentIndex = QModelIndex()) const;
	virtual QModelIndex index(ccHObject* object);
	virtual QModelIndex parent(const QModelIndex &index) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	virtual Qt::DropActions supportedDropActions() const;
	virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
	virtual QMap<int,QVariant> itemData(const QModelIndex& index) const;
#ifdef CC_QT5
	virtual Qt::DropActions supportedDragActions() const { return Qt::MoveAction; }
#endif

public slots:
	void changeSelection(const QItemSelection & selected, const QItemSelection & deselected);
	void reflectObjectPropChange(ccHObject* obj);//��Ӧ�������Ա仯
	void redrawCCObject(ccHObject* anObject); // ���»�������
	void redrawCCObjectAndChildren(ccHObject* anObject);//���»���������������
	void updateCCObject(ccHObject* anObject);//��������
	void deleteSelectedEntities();//ɾ��ѡ��

	//! Shortcut to selectEntity(ccHObject*)
	void selectEntity(int uniqueID); 

	//! Selects multiple entities at once (shortcut to the other version)
	/** \param entIDs list of the IDs of the entities to select
	**/
	void selectEntities(std::set<int> entIDs);

	//! Selects multiple entities at once
	/** \param entIDs set of the entities to 'select'
		\param incremental whether to 'add' the input set to the selected entities set or to use it as replacement
	**/
	void selectEntities(const ccHObject::Container& entities, bool incremental = false);

protected slots:
	void showContextMenu(const QPoint&); //��ʾ�����˵�
	void expandBranch(); //���ŷ�֧
	void collapseBranch(); //ȡ����֧
	void gatherRecursiveInformation();//�ռ��ݹ���Ϣ
	void sortSiblingsAZ();//���������������
	void sortSiblingsZA();
	void sortSiblingsType();//�������尴����������
	void toggleSelectedEntities(); //�л�ѡ�������
	void toggleSelectedEntitiesVisibility(); //�л�ѡ������Ŀ�����
	void toggleSelectedEntitiesColor(); //�л�ѡ���������ɫ
	void toggleSelectedEntitiesNormals(); //�л�ѡ������ķ�����
	void toggleSelectedEntitiesSF(); //�л�ѡ������ı�����
	void toggleSelectedEntitiesMat(); //�л�ѡ������ľ���
	void toggleSelectedEntities3DName(); //�л�ѡ�������3D����
	void addEmptyGroup(); //��ӿյ���
	void alignCameraWithEntityDirect() { alignCameraWithEntity(false); } //????????????????
	void alignCameraWithEntityIndirect() { alignCameraWithEntity(true); } //??????????????????????//
	void enableBubbleViewMode();

signals:
	void selectionChanged(); //ѡ�����仯

protected:

	//! Aligns the camera with the currently selected entity
	/** \param reverse whether to use the entity's normal (false) or its inverse (true)
	**/
	void alignCameraWithEntity(bool reverse); //�������ѡ���������ж���

	//! Shows properties view for a given element  //��ʾ�������������
	void showPropertiesView(ccHObject* obj);

	//! Toggles a given property (enable state, visibility, normal, color, SF, etc.) on selected entities
	/** Properties are:
			0 - enable state
			1 - visibility
			2 - normal
			3 - color
			4 - SF
			5 - materials/textures
			6 - 3D name
	**/
	void toggleSelectedEntitiesProperty(unsigned prop); //�л����������ԵĿ�����

	//! Entities sorting schemes
	enum SortRules { SORT_A2Z, SORT_Z2A, SORT_BY_TYPE }; //�������

	//! Sorts selected entities siblings
	void sortSelectedEntitiesSiblings(SortRules rule); //��ѡ����ֵܽڵ��������

	//! Expands or collapses hovered item
	void expandOrCollapseHoveredBranch(bool expand); //????????????????????????????????????////

	//! Associated DB root // ���ĸ��ڵ�
	ccHObject* m_treeRoot;   

	//! Associated widget for DB tree // DBTree��صĲ���
	QTreeView* m_dbTreeWidget;

	//! Associated widget for selected entity's properties tree
	QTreeView* m_propertiesTreeWidget; // ������صĲ���

	//! Selected entity's properties data model  //ѡ������������ģ��
	QStandardItemModel* m_propertiesModel;

	//! Selected entity's properties delegate  //ѡ�����������Դ���
	ccPropertiesTreeDelegate* m_ccPropDelegate;  //??????????????????????????????????????????????

	//! Context menu action: expand tree branch
	QAction* m_expandBranch; // ������
	//! Context menu action: collapse tree branch
	QAction* m_collapseBranch; // �۵���
	//! Context menu action: gather (recursive) information on selected entities
	QAction* m_gatherInformation; //�Ѽ���Ϣ
	//! Context menu action: sort siblings in alphabetical order
	QAction* m_sortSiblingsAZ; //����1
	//! Context menu action: sort siblings in reverse alphabetical order
	QAction* m_sortSiblingsZA; //����2
	//! Context menu action: sort siblings by type
	QAction* m_sortSiblingsType;  //����3
	//! Context menu action: delete selected entities
	QAction* m_deleteSelectedEntities; // ɾ��ѡ�����������
	//! Context menu action: enabled/disable selected entities
	QAction* m_toggleSelectedEntities; //�л�ѡ�����������
	//! Context menu action: hide/show selected entities
	QAction* m_toggleSelectedEntitiesVisibility; //�л�ѡ������Ŀ�����
	//! Context menu action: hide/show selected entities color
	QAction* m_toggleSelectedEntitiesColor; // �л�ѡ���������ɫ
	//! Context menu action: hide/show selected entities normals
	QAction* m_toggleSelectedEntitiesNormals; //�л�ѡ������ķ�����
	//! Context menu action: hide/show selected entities materials/textures
	QAction* m_toggleSelectedEntitiesMat; //�л�ѡ������ľ���
	//! Context menu action: hide/show selected entities SF
	QAction* m_toggleSelectedEntitiesSF; //�л�ѡ������ı�����
	//! Context menu action: hide/show selected entities 3D name
	QAction* m_toggleSelectedEntities3DName;//�л�ѡ�������3Dname
	//! Context menu action: add empty group
	QAction* m_addEmptyGroup;  //��ӿյ���
	//! Context menu action: use 3-points labels or planes to orient camera
	QAction* m_alignCameraWithEntity;  //�����������
	//! Context menu action: reverse of m_alignCameraWithEntity
	QAction* m_alignCameraWithEntityReverse; //
	//! Context menu action: enable bubble-view (on a sensor)
	QAction* m_enableBubbleViewMode;

	//! Last context menu pos
	QPoint m_contextMenuPos;  //�˵�λ��

    //Added by yuqiang on 2015/11/07
    QAction* mActionCreateMarkedObjectBag;
    QAction* mActionAddMarkedObject;
    QAction* mActionChangeMarkedObjectColor;
    QAction* mActionShowMarkedObjectProperties;
    QAction* mActionExportMarkedObjectAsShapefile;
signals:
    void actionCreateMarkedObjectBagTriggered();
    void actionAddMarkedObjectTriggered();
    void actionChangeMarkedObjectColorTriggered();
    void actionShowMarkedObjectPropertiesTriggered();
    void actionExportMarkedObjectAsShapefileTriggered();

};

#endif
