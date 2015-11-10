#ifndef MARKEDOBJECTBAG_H
#define MARKEDOBJECTBAG_H


#include <QList>
#include "ccCustomObject.h"
#include "MarkedObject.h"
#include "ccGLWindow.h"
#include <QSqlTableModel>

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

    //数据库中显示的类型
    static const QStringList TABLE_TYPE_STRINGS;
    static const QStringList TABLE_READONLY_FIELD_NAMES;

private:

    ccGLWindow *mContextWin;

    Type mType;

    QList<MarkedObject*> mObjects;

    //数据库表
    QString mTableName;
    QSqlTableModel *mTableModel;

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

    MarkedObjectBag(ccGLWindow *contextWin, const Type type, const QString &name = "UntitledObject");

    //void setType(const Type type);

    Type getType() const;

    //Inherited from MarkedObject
    void setName(const QString &name);

    //Inherited from MarkedObject
    void setColor(const QColor &color);

    void addObject(MarkedObject *object);

    unsigned size();

    MarkedObject* getLatestObject();

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

    //添加子物体完毕
    void done();

    void refreshBBox();

    QSqlTableModel* getTableModel();

};


#endif // MARKEDOBJECTBAG_H
