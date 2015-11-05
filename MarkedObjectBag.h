#ifndef MARKEDOBJECTBAG_H
#define MARKEDOBJECTBAG_H


#include <QList>
#include "ccCustomObject.h"
#include "MarkedObject.h"
#include "ccGLWindow.h"

class MarkedObjectBag : public MarkedObject
{

public:

    //����MarkedObject������
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

    //������ʷ��¼
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
    QVector<HistoryItem> mHistory; //��ʷ��¼
    int mCurrentPositionInHistory; //��ǰ״̬����ʷ��¼�е�λ�ã��±꣩

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
