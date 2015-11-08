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
    if (!mObjects.empty()) //����Ѿ���ӹ�object���������ٸ�������
    {
        assert(false);
        return;
    }

    mType = type;
}

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

    object->setName(getName() + "_" + QString::number(mObjects.size() + 1));
    object->setColor(getColor());
    object->setMarkedType(NULL);
    object->setSelected(true);

    //��֮ǰ������object��2D��ǩ����
    const unsigned objectNum = mObjects.size();
    for (unsigned i = 0; i < objectNum; i++)
    {
        mObjects[i]->setSelected(false);
    }

    mObjects.push_back(object);

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
