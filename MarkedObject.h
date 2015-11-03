#ifndef MARKEDOBJECT_H
#define MARKEDOBJECT_H


#include "cc2DLabel.h"
#include "ccColorTypes.h"
#include "ccPointCloud.h"
#include <QColor>

class MarkedObject : public cc2DLabel
{

protected:

    //所属类别，即该点在树状列表中的父节点
    ccHObject *mMarkedType;

    //名称（直接使用ccObject的m_name, setName() 和 getName()）
    //QString mMarkedName;
    static const QString DEFAULT_NAME;

    //颜色
    QColor mColor;

public:

    MarkedObject();

    MarkedObject(QString name);

    virtual void setMarkedType(ccHObject *markedType);

    ccHObject* getMarkedType();

    virtual void setColor(const QColor &color); //前面加virtual关键字修饰可以使得由子类指针强制转换得到的父类指针调用该方法时实际调用子类同名方法

    QColor getColor();

    //inherited from cc2DLabel
    virtual inline void setSelected(bool state);

    //inherited from cc2DLabel
    void drawMeOnly2D(CC_DRAW_CONTEXT& context);

    //inherited from cc2DLabel
    virtual QStringList getLabelContent(int precision) = 0;

    //inherited from cc2DLabel
    //virtual bool move2D(int x, int y, int dx, int dy, int screenWidth, int screenHeight);

    //撤销一步
    virtual bool undo() = 0;

    //重做一步
    virtual bool redo() = 0;
};


#endif // MARKEDOBJECT_H
