#include "MarkedObjectBag.h"
#include "MarkedPoint.h"
#include "MarkedLine.h"
#include "MarkedArea.h"

MarkedObjectBag::MarkedObjectBag(ccGLWindow *contextWin, const Type type, const QString &name)
    : MarkedObject(name)
    , mContextWin(contextWin)
    , mType(type)
    , mCurrentPositionInHistory(-1)
{
}

void MarkedObjectBag::setType(const Type type)
{
    if (!mObjects.empty()) //如果已经添加过object，则不允许再更改类型
    {
        assert(false);
        return;
    }

    mType = type;
}

MarkedObjectBag::Type MarkedObjectBag::getType()
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

    object->setName(getName());
    object->setColor(getColor());
    object->setMarkedType(NULL);

    //将之前的所有object的2D标签隐藏
    const unsigned objectNum = mObjects.size();
    for (unsigned i = 0; i < objectNum; i++)
    {
        mObjects[i]->setSelected(false);
    }

    mObjects.push_back(object);

    mContextWin->addToOwnDB(object);
    mContextWin->redraw();

    //历史纪录
    if (!mHistory.isEmpty() && mCurrentPositionInHistory < mHistory.size() - 1) //舍弃历史纪录中被撤销的操作
    {
        mHistory.remove(mCurrentPositionInHistory + 1, mHistory.size() - mCurrentPositionInHistory- 1);
    }
    mHistory.resize(mHistory.size() + 1);
    mHistory.back().objects = mObjects;
    mCurrentPositionInHistory = mHistory.size() - 1;
}

unsigned MarkedObjectBag::size()
{
    return mObjects.size();
}

MarkedObject * MarkedObjectBag::getLatestObject()
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

void MarkedObjectBag::removeLatestObject()
{
    mObjects.pop_back();
}

void MarkedObjectBag::draw(CC_DRAW_CONTEXT& context)
{
    ccHObject::draw(context);

    const unsigned objectNum = mObjects.size();
    for (unsigned i = 0; i < objectNum; i++)
    {
        mObjects[i]->draw(context);
    }
}

inline void MarkedObjectBag::setSelected(bool state)
{
    MarkedObject::setSelected(state);

    const unsigned objectNum = mObjects.size();
    for (unsigned i = 0; i < objectNum; i++)
    {
        mObjects[i]->setSelected(state);
    }
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
            return true;
        }
    case POINT:
        if (mCurrentPositionInHistory > 0)
        {
            mCurrentPositionInHistory--;
            mContextWin->removeFromOwnDB(mObjects.back());
            mObjects = mHistory.at(mCurrentPositionInHistory).objects;
            return true;
        }
        else if (mCurrentPositionInHistory == 0)
        {
            mCurrentPositionInHistory--;
            mContextWin->removeFromOwnDB(mObjects.back());
            mObjects.clear();
            return true;
        }
        else
        {
            return false;
        }
        break;
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
            return true;
        }
    case POINT:
        if (mCurrentPositionInHistory < mHistory.size() - 1)
        {
            mCurrentPositionInHistory++;
            mObjects = mHistory.at(mCurrentPositionInHistory).objects;
            mContextWin->addToOwnDB(mObjects.back());
            return true;
        }
        else
        {
            return false;
        }
        break;
    }
}
