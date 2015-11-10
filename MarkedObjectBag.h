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

    //����MarkedObject������
    enum Type
    {
        POINT,
        LINE,
        AREA
    };

    //���ݿ�����ʾ������
    static const QStringList TABLE_TYPE_STRINGS;
    static const QStringList TABLE_READONLY_FIELD_NAMES;

private:

    ccGLWindow *mContextWin;

    Type mType;

    QList<MarkedObject*> mObjects;

    //���ݿ��
    QString mTableName;
    QSqlTableModel *mTableModel;

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

    //������������
    void done();

    void refreshBBox();

    QSqlTableModel* getTableModel();

};


#endif // MARKEDOBJECTBAG_H
