#include "MarkedObjectBag.h"
#include "MarkedPoint.h"
#include "MarkedLine.h"
#include "MarkedArea.h"
#include <QDateTime>
#include <QSqlQuery>

const QStringList MarkedObjectBag::TABLE_TYPE_STRINGS = QString("POINTZ,POLYLINEZ,POLYGONZ").split(",");
const QStringList MarkedObjectBag::TABLE_READONLY_FIELD_NAMES = QString("OBJECT_ID,NAME,SHAPE,LENGTH,BOUNDARY_LENGTH,AREA").split(",");

MarkedObjectBag::MarkedObjectBag(ccGLWindow *contextWin, const Type type, const QString &name)
    : MarkedObject(name)
    , mContextWin(contextWin)
    , mType(type)
    , mTableModel(NULL)
    , mCurrentPositionInHistory(-1)
{
    //�������ݿ��
    mTableName = name + QDateTime::currentDateTime().toString("_yyyy_MM_dd_hh_mm_ss_zzz");
    QSqlQuery query;
    QString queryString = QString("create table %1 (%2 INTERGER(9) PRIMARY KEY, %3 varchar(20), %4 varchar(20)")
        .arg(mTableName)
        .arg(TABLE_READONLY_FIELD_NAMES[0])
        .arg(TABLE_READONLY_FIELD_NAMES[1])
        .arg(TABLE_READONLY_FIELD_NAMES[2]);
    switch(type)
    {
    case POINT:
        break;
    case LINE:
        queryString += QString(", LENGTH float(32)").arg(TABLE_READONLY_FIELD_NAMES[3]);
        break;
    case AREA:
        queryString += QString(", BOUNDARY_LENGTH float(32)").arg(TABLE_READONLY_FIELD_NAMES[4]);
        queryString += QString(", AREA float(32)").arg(TABLE_READONLY_FIELD_NAMES[5]);
        break;
    }
    queryString += ")";
    query.exec(queryString);

    mTableModel = new QSqlTableModel();
    mTableModel->setTable(mTableName);
}

// void MarkedObjectBag::setType(const Type type)
// {
//     if (!mObjects.empty()) //����Ѿ���ӹ�object���������ٸ�������
//     {
//         assert(false);
//         return;
//     }
// 
//     mType = type;
// }

MarkedObjectBag::Type MarkedObjectBag::getType() const
{
    return mType;
}

void MarkedObjectBag::setName(const QString &name)
{
    MarkedObject::setName(name);

    const unsigned objectNum = mObjects.size();
    for (unsigned i = 0; i < objectNum; i++)
    {
        mObjects[i]->setName(name);
    }

    mContextWin->redraw();
}

void MarkedObjectBag::setColor(const QColor &color)
{
    MarkedObject::setColor(color);

    const unsigned objectNum = mObjects.size();
    for (unsigned i = 0; i < objectNum; i++)
    {
        mObjects[i]->setColor(color);
    }

    mContextWin->redraw();
}

void MarkedObjectBag::addObject(MarkedObject *object)
{
    switch(mType)
    {
    case POINT:
        assert(dynamic_cast<MarkedPoint*>(object));
        break;
    case LINE:
        assert(dynamic_cast<MarkedLine*>(object));
        break;
    case AREA:
        assert(dynamic_cast<MarkedArea*>(object));
        break;
    default:
        assert(false);
    }

    const unsigned objectIndex = mObjects.size() + 1;
    object->setName(getName() + "_" + QString::number(objectIndex));
    object->setColor(getColor());
    object->setMarkedType(NULL);
    object->setSelected(true);

    //��ӵ����ݿ�
    /*QSqlQuery query;
    QString queryString;
    switch(mType)
    {
    case POINT:
        queryString = QString("insert into %1 values(%2, '%3', '%4'")
            .arg(mTableName)
            .arg(objectIndex)
            .arg(object->getName())
            .arg(TABLE_TYPE_STRINGS[mType]);
        queryString += ")";
        query.exec(queryString);
        break;
    case LINE: //��Ϊ����ߺ���ʱ������mObjectsĩβ���һ���յ��߻���Ȼ�����޸�֮����˴˴���ӵ����ݿ����Ҫ�����һλ�������ν��ϴ���ӵ���������ӵ����ݿ�
        if (objectIndex > 1)
        {
            MarkedLine *markedLine = dynamic_cast<MarkedLine*>(mObjects.back());
            queryString = QString("insert into %1 values(%2, '%3', '%4'")
                .arg(mTableName)
                .arg(objectIndex - 1)
                .arg(object->getName())
                .arg(TABLE_TYPE_STRINGS[mType]);
            queryString += QString(", %1").arg(markedLine->getLineLength());
            queryString += ")";
            query.exec(queryString);
        }
        break;
    case AREA: //��Ϊ����ߺ���ʱ������mObjectsĩβ���һ���յ��߻���Ȼ�����޸�֮����˴˴���ӵ����ݿ����Ҫ�����һλ�������ν��ϴ���ӵ���������ӵ����ݿ�
        if (objectIndex > 1)
        {
            MarkedArea *markedArea = dynamic_cast<MarkedArea*>(mObjects.back());
            queryString = QString("insert into %1 values(%2, '%3', '%4'")
                .arg(mTableName)
                .arg(objectIndex - 1)
                .arg(object->getName())
                .arg(TABLE_TYPE_STRINGS[mType]);
            queryString += QString(", %1").arg(markedArea->getBoundaryLength());
            queryString += QString(", %1").arg(markedArea->getArea());
            queryString += ")";
            query.exec(queryString);
        }
        break;
    }*/

    //��֮ǰ������object��2D��ǩ����
    const unsigned objectNum = mObjects.size();
    for (unsigned i = 0; i < objectNum; i++)
    {
        mObjects[i]->setSelected(false);
    }

    mObjects.push_back(object);

    //��ʾ��OpenGL����
    mContextWin->addToOwnDB(object);
    mContextWin->redraw();

    //��ʷ��¼
    if (!mHistory.isEmpty() && mCurrentPositionInHistory < mHistory.size() - 1) //������ʷ��¼�б������Ĳ���
    {
        mHistory.remove(mCurrentPositionInHistory + 1, mHistory.size() - mCurrentPositionInHistory- 1);
    }
    mHistory.resize(mHistory.size() + 1);
    mHistory.back().objects = mObjects;
    mCurrentPositionInHistory = mHistory.size() - 1;

    refreshBBox();
}

unsigned MarkedObjectBag::size()
{
    return mObjects.size();
}

MarkedObject* MarkedObjectBag::getLatestObject()
{
    if (mObjects.size() == 0)
    {
        return NULL;
    }
    else
    {
        return mObjects.back();
    }
}

static ccColor::Rgb staticBBoxColor(255, 255, 0); //Bounding-box��ɫ

void MarkedObjectBag::draw(CC_DRAW_CONTEXT& context)
{
    ccHObject::draw(context);

    const unsigned objectNum = mObjects.size();
    for (unsigned i = 0; i < objectNum; i++)
    {
        mObjects[i]->draw(context);
    }

    //��Bounding-box
    if (isSelected())
    {
        mBBox.draw(staticBBoxColor);
    }
}

inline void MarkedObjectBag::setSelected(bool state)
{
    cc2DLabel::setSelected(state);

    const unsigned objectNum = mObjects.size();
    for (unsigned i = 0; i < objectNum; i++)
    {
        mObjects[i]->setSelected(false);
    }

    mContextWin->redraw();
}

QStringList MarkedObjectBag::getLabelContent(int precision)
{
    return QStringList();
}

void MarkedObjectBag::clear()
{
    const unsigned objectNum = mObjects.size();
    for (unsigned i = 0; i < objectNum; i++)
    {
        mContextWin->removeFromOwnDB(mObjects[i]);
    }
    mObjects.clear();

    mContextWin->redraw();
}

bool MarkedObjectBag::undo()
{
    switch(mType)
    {
    case LINE:
    case AREA:
        if (mObjects.size() > 0 && mObjects.back()->undo())
        {
            refreshBBox();
            return true;
        }
    case POINT:
        if (mCurrentPositionInHistory > 0)
        {
            mCurrentPositionInHistory--;
            mContextWin->removeFromOwnDB(mObjects.back());
            mObjects = mHistory.at(mCurrentPositionInHistory).objects;
            refreshBBox();
            return true;
        }
        else if (mCurrentPositionInHistory == 0)
        {
            mCurrentPositionInHistory--;
            mContextWin->removeFromOwnDB(mObjects.back());
            mObjects.clear();
            refreshBBox();
            return true;
        }
        else
        {
            refreshBBox();
            return false;
        }
        break;
    default:
        refreshBBox();
        return false;
    }
}

bool MarkedObjectBag::redo()
{
    switch(mType)
    {
    case LINE:
    case AREA:
        if (mObjects.size() > 0 && mObjects.back()->redo())
        {
            refreshBBox();
            return true;
        }
    case POINT:
        if (mCurrentPositionInHistory < mHistory.size() - 1)
        {
            mCurrentPositionInHistory++;
            mObjects = mHistory.at(mCurrentPositionInHistory).objects;
            mContextWin->addToOwnDB(mObjects.back());
            refreshBBox();
            return true;
        }
        else
        {
            refreshBBox();
            return false;
        }
        break;
    default:
        refreshBBox();
        return false;
    }
}

void MarkedObjectBag::done()
{
    if (mObjects.empty())
    {
        return;
    }

    //ȥ�����Ϸ���������
    switch(mType)
    {
    case POINT:
        break;
    case LINE:
        if (!(dynamic_cast<MarkedLine*>(mObjects.back()))->isValid())
        {
            mContextWin->removeFromOwnDB(mObjects.back());
            mObjects.pop_back();
        }
        break;
    case AREA:
        if (!(dynamic_cast<MarkedArea*>(mObjects.back()))->isValid())
        {
            mContextWin->removeFromOwnDB(mObjects.back());
            mObjects.pop_back();
        }
        break;
    }

    //��������������ӵ����ݿ�
    QSqlQuery deleteAllItemsQuery;
    deleteAllItemsQuery.exec(QString("delete from %1").arg(mTableName)); //�Ƚ������
    const unsigned objectNum = mObjects.size();
    QSqlQuery insertQuery;
    switch(mType)
    {
    case POINT:
        insertQuery.prepare(QString("insert into %1 values(?, ?, ?)").arg(mTableName));
        for(unsigned i = 0; i < objectNum; i++)
        {
            insertQuery.bindValue(0, i + 1);
            insertQuery.bindValue(1, mObjects[i]->getName());
            insertQuery.bindValue(2, TABLE_TYPE_STRINGS[POINT]);
            assert(insertQuery.exec());
        }
        break;
    case LINE:
        insertQuery.prepare(QString("insert into %1 values(?, ?, ?, ?)").arg(mTableName));
        for(unsigned i = 0; i < objectNum; i++)
        {
            insertQuery.bindValue(0, i + 1);
            insertQuery.bindValue(1, mObjects[i]->getName());
            insertQuery.bindValue(2, TABLE_TYPE_STRINGS[LINE]);
            MarkedLine *markedLine = dynamic_cast<MarkedLine*>(mObjects[i]);
            insertQuery.bindValue(3, markedLine->getLineLength());
            assert(insertQuery.exec());
        }
        break;
    case AREA:
        insertQuery.prepare(QString("insert into %1 values(?, ?, ?, ?, ?)").arg(mTableName));
        for(unsigned i = 0; i < objectNum; i++)
        {
            insertQuery.bindValue(0, i + 1);
            insertQuery.bindValue(1, mObjects[i]->getName());
            insertQuery.bindValue(2, TABLE_TYPE_STRINGS[LINE]);
            MarkedArea *markedArea = dynamic_cast<MarkedArea*>(mObjects[i]);
            insertQuery.bindValue(3, markedArea->getBoundaryLength());
            insertQuery.bindValue(4, markedArea->getArea());
            assert(insertQuery.exec());
        }
        break;
    default:
        assert(false);
        break;
    }

}

void MarkedObjectBag::refreshBBox()
{
    mBBox.clear();

    const unsigned objectNum = mObjects.size();
    for (unsigned i = 0; i < objectNum; i++)
    {
        mBBox += mObjects[i]->getBBox();
    }

    mContextWin->redraw();
}

QSqlTableModel* MarkedObjectBag::getTableModel()
{
    mTableModel->clear();
    mTableModel->setTable(mTableName);
    mTableModel->select();
    return mTableModel;
}
