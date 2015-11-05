#ifndef MARKEDOBJECTBAG_H
#define MARKEDOBJECTBAG_H


#include <QList>
#include "ccCustomObject.h"
#include "MarkedObject.h"
#include "ccGLWindow.h"

class MarkedObjectBag : public MarkedObject
{

public:

    //包含MarkedObject的类型
    enum Type
    {
        POINT,
        LINE,
        AREA
    };

private:

    ccGLWindow *mContextWin;

    Type mType;

    QList<MarkedObject*> mObjects;

    //操作历史纪录
    struct HistoryItem
    {
        QList<MarkedObject*> objects;

        HistoryItem()
            : objects()
        {
        }

        HistoryItem(QList<MarkedObject*> _objects)
            : objects(_objects)
        {
        }
    };
    QVector<HistoryItem> mHistory; //历史纪录
    int mCurrentPositionInHistory; //当前状态在历史记录中的位置（下标）

public:

    MarkedObjectBag(ccGLWindow *contextWin, const Type type = POINT, const QString &name = "UntitledObject");

    void setType(const Type type);

    Type getType();

    //Inherited from MarkedObject
    void setName(const QString &name);

    //Inherited from MarkedObject
    void setColor(const QColor &color);

    void addObject(MarkedObject *object);

    unsigned size();

    MarkedObject * getLatestObject();

    void removeLatestObject();

    //Inherited from ccHObject
    void draw(CC_DRAW_CONTEXT& context);

    //inherited from MarkedObject
    inline void setSelected(bool state);

    //inherited from MarkedObject
    QStringList getLabelContent(int precision);

    void clear();

    //Inherited from MarkedObject
    bool undo();

    //Inherited from MarkedObject
    bool redo();

};


#endif // MARKEDOBJECTBAG_H
