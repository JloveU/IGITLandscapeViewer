#include "SetMarkedPointPropertyDlg.h"
#include <QColorDialog>
#include <QBrush>
#include <QMessageBox>
#include <QtAlgorithms>
#include "MarkedPoint.h"

SetMarkedPointPropertyDlg::SetMarkedPointPropertyDlg(QWidget* parent)
    : QDialog(parent)
    , Ui::SetMarkedPointPropertyDlg()
    , mMarkedPointTreeRoot(NULL)
    , mColorSelectorLocked(false)
{
    setupUi(this);

    //设置初始值
    setMarkedTypeItems(QStringList());
    setMarkedTypeName(QString());
    setMarkedPointName(QString());
    setMarkedPointColor(QColor(0, 0, 255));

    //为不同类别预生成颜色列表
    //颜色分配方案说明：按灰度从0.0到1.0对应到伪彩色（从蓝到红），灰度依次为0.0、1.0、0.5、0.25、0.75、0.125、0.375、0.625、0.875……
    const unsigned PRE_CREATE_COLOR_NUM = 100; //默认预分配100个类型的颜色
    QList<float> grayValueList;
    grayValueList.reserve(PRE_CREATE_COLOR_NUM);
    grayValueList.push_back(0.0);
    grayValueList.push_back(1.0);
    unsigned level = 1;
    while (grayValueList.size() < PRE_CREATE_COLOR_NUM)
    {
        const float grayValueStep = 1.0 / (1 << level);
        for (unsigned colorIndexPerLevel = 1; colorIndexPerLevel < (1 << level); colorIndexPerLevel += 2)
        {
            grayValueList.push_back(grayValueStep * colorIndexPerLevel);
            if (grayValueList.size() >= PRE_CREATE_COLOR_NUM)
            {
                break;
            }
        }
        level++;
    }
    //qSort(grayValueList);
    mColorList.reserve(PRE_CREATE_COLOR_NUM);
    for (unsigned colorIndex = 0; colorIndex < PRE_CREATE_COLOR_NUM; colorIndex++)
    {
        mColorList.push_back(gray2PseudoColor(grayValueList.at(colorIndex)));
    }

    //设置选择颜色按钮响应
    connect(colorSelectButton, SIGNAL(clicked()), this, SLOT(selectColor()));

    //标记类型输入框内容改变时，名称输入框相应改变
    connect(typeComboBox, SIGNAL(editTextChanged(const QString &)), this, SLOT(autoSetMarkedPointName(const QString &)));

    //标记类型输入框内容改变时，颜色相应改变
    connect(typeComboBox, SIGNAL(editTextChanged(const QString &)), this, SLOT(autoSetMarkedPointColor(const QString &)));

    //“确定”按钮点击事件
    connect(okButton, SIGNAL(clicked()), this, SLOT(checkInputAndCommit()));
}

void SetMarkedPointPropertyDlg::setMarkedPointTreeRoot(ccHObject *markedPointTreeRoot)
{
    mMarkedPointTreeRoot = markedPointTreeRoot;
    autoSetMarkedPointName(QString());
    autoSetMarkedPointColor(QString());
}

void SetMarkedPointPropertyDlg::setMarkedTypeItems(const QStringList &items)
{
    typeComboBox->clear();
    typeComboBox->addItems(items);
}

void SetMarkedPointPropertyDlg::setMarkedTypeName(const QString &name)
{
    typeComboBox->setCurrentText(name);
}

QString SetMarkedPointPropertyDlg::getMarkedTypeName()
{
    return typeComboBox->currentText();
}

void SetMarkedPointPropertyDlg::setMarkedPointName(const QString &name)
{
    nameLineEdit->setText(name);
}

QString SetMarkedPointPropertyDlg::getMarkedPointName()
{
    return nameLineEdit->text();
}

void SetMarkedPointPropertyDlg::setMarkedPointColor(const QColor &color)
{
    mMarkedPointColor = color;
    colorSelectButton->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(color.red()).arg(color.green()).arg(color.blue()));
}

QColor SetMarkedPointPropertyDlg::getMarkedPointColor()
{
    return mMarkedPointColor;
}

void SetMarkedPointPropertyDlg::selectColor()
{
    if (mColorSelectorLocked)
    {
        QMessageBox::information(this, QString::fromAscii("警告"), QString::fromAscii("当前颜色属性不可更改，因为同一类型的标记点必须颜色相同！"));
        return;
    }
    QColor color = QColorDialog::getColor(Qt::white, this);
    setMarkedPointColor(color);
}

void SetMarkedPointPropertyDlg::show()
{
    assert(mMarkedPointTreeRoot);

    ////设置各控件的状态和内容
    //获取标记点根标签下所有子标签的名称（即标记点所属类别），用以在弹出对话框中显示供选择
    QStringList markTypeItems;
    for (int i = 0; i < mMarkedPointTreeRoot->getChildrenNumber(); i++)
    {
        markTypeItems.push_back(mMarkedPointTreeRoot->getChild(i)->getName());
    }
    setMarkedTypeItems(markTypeItems);
    setMarkedTypeName(mMarkedTypeName);
    autoSetMarkedPointName(mMarkedTypeName);
    setMarkedPointColor(mMarkedPointColor);

    QDialog::show();
}

void SetMarkedPointPropertyDlg::autoSetMarkedPointName(const QString &markedTypeName)
{
    assert(mMarkedPointTreeRoot);

    if (markedTypeName.isEmpty())
    {
        setMarkedPointName("");
        return;
    }

    ccHObject *markedType = mMarkedPointTreeRoot->findDirectChild(markedTypeName);
    if (markedType)
    {
        int nameSuffixNumber = 1;
        ccHObject *markedPoint = markedType->findDirectChild(markedTypeName + QString::number(nameSuffixNumber));
        while (markedPoint)
        {
            nameSuffixNumber++;
            markedPoint = markedType->findDirectChild(markedTypeName + QString::number(nameSuffixNumber));

        }
        setMarkedPointName(markedTypeName + QString::number(nameSuffixNumber));
        return;
    }
    else
    {
        setMarkedPointName(markedTypeName + "1");
        return;
    }
}

void SetMarkedPointPropertyDlg::autoSetMarkedPointColor(const QString &markedTypeName)
{
    assert(mMarkedPointTreeRoot);

    if (markedTypeName.isEmpty())
    {
        setMarkedPointColor(QColor(0, 0, 255));
        mColorSelectorLocked = false;
        return;
    }

    ccHObject *markedType = mMarkedPointTreeRoot->findDirectChild(markedTypeName);
    if (markedType)
    {
        if (markedType->getChildrenNumber() > 0)
        {
            MarkedPoint *markedPoint = dynamic_cast<MarkedPoint*>(markedType->getChild(0));
            assert(markedPoint);
            setMarkedPointColor(markedPoint->getColor());
            mColorSelectorLocked = true;
            return;
        }
        else
        {
            setMarkedPointColor(QColor(0, 0, 255));
            mColorSelectorLocked = false;
            return;
        }
    }
    else
    {
        QColor color;

        //检查预分配颜色列表中是否有未使用的颜色
        bool hasUnusedColorInList = false;
        for (unsigned colorIndex = 0; colorIndex < mColorList.size(); colorIndex++)
        {
            color = mColorList.at(colorIndex);

            //检查是否已使用过了此颜色
            bool currentColorHasUsed = false;
            for (unsigned i = 0; i < mMarkedPointTreeRoot->getChildrenNumber(); i++)
            {
                ccHObject *markedType = mMarkedPointTreeRoot->getChild(i);
                if (markedType->getChildrenNumber() > 0)
                {
                    QColor currentTypeColor = static_cast<MarkedPoint*>(markedType->getChild(0))->getColor();
                    if (currentTypeColor == color)
                    {
                        currentColorHasUsed = true;
                        break;
                    }
                }
            }
            if (!currentColorHasUsed)
            {
                hasUnusedColorInList = true;
                break;
            }
        }
        if (hasUnusedColorInList)
        {
            setMarkedPointColor(color);
        }
        else
        {
            setMarkedPointColor(QColor(0, 0, 0)); //需要手动设置颜色
        }

        mColorSelectorLocked = false;
        return;
    }
}

void SetMarkedPointPropertyDlg::checkInputAndCommit()
{
    assert(mMarkedPointTreeRoot);

    //确保类型输入框不为空
    QString markedTypeName = getMarkedTypeName();
    if (markedTypeName.isEmpty())
    {
        QMessageBox::information(this, QString::fromAscii("警告"), QString::fromAscii("输入的标记点类型名称不能为空，请重新输入！"));
        return;
    }

    //确保名称输入框不为空
    QString markedPointName = getMarkedPointName();
    if (markedPointName.isEmpty())
    {
        QMessageBox::information(this, QString::fromAscii("警告"), QString::fromAscii("输入的标记点名称不能为空，请重新输入！"));
        return;
    }

    //保证同一类型下没有重名
    ccHObject *markedType = mMarkedPointTreeRoot->findDirectChild(markedTypeName);
    if (markedType)
    {
        if (markedType->findDirectChild(markedPointName))
        {
            QMessageBox::information(this, QString::fromAscii("警告"), QString::fromAscii("输入的标记点名称已存在，请重新输入！"));
            return;
        }
    }

    //保证不同类型的颜色不同
    const QColor markedPointColor = getMarkedPointColor();
    for (unsigned i = 0; i < mMarkedPointTreeRoot->getChildrenNumber(); i++)
    {
        ccHObject *markedType = mMarkedPointTreeRoot->getChild(i);
        if (markedType->getName() != markedTypeName && markedType->getChildrenNumber() > 0)
        {
            QColor currentTypeColor = static_cast<MarkedPoint*>(markedType->getChild(0))->getColor();
            if (currentTypeColor == markedPointColor)
            {
                QMessageBox::information(this, QString::fromAscii("警告"), QString::fromAscii("选择的颜色已被其他类型占用，请重新选择！"));
                return;
            }
        }
    }

    //保存本次输入状态，以供下次显示
    mMarkedTypeName = getMarkedTypeName();
    mMarkedPointColor = getMarkedPointColor();

    hide();
    emit accepted();
    emit committed(getMarkedTypeName(), getMarkedPointName(), getMarkedPointColor());
}

QColor SetMarkedPointPropertyDlg::gray2PseudoColor(float grayValue)
{
    //将grayValue规范化到0～1之间
    if(grayValue < 0.0)
    {
        grayValue = 0.0;
    }
    if(grayValue > 1.0)
    {
        grayValue = 1.0;
    }

    QColor pseudoColor;

    if(grayValue < 0.25)
    {
        pseudoColor.setRedF(0.0);
        pseudoColor.setGreenF(grayValue * 4.0);
        pseudoColor.setBlueF(1.0);
    }
    else if(grayValue < 0.5)
    {
        pseudoColor.setRedF(0.0);
        pseudoColor.setGreenF(1.0);
        pseudoColor.setBlueF(1.0 - (grayValue - 0.25) * 4.0);
    }
    else if(grayValue < 0.75)
    {
        pseudoColor.setRedF((grayValue - 0.5) * 4.0);
        pseudoColor.setGreenF(1.0);
        pseudoColor.setBlueF(0.0);
    }
    else
    {
        pseudoColor.setRedF(1.0);
        pseudoColor.setGreenF(1.0 - (grayValue - 0.75) * 4.0);
        pseudoColor.setBlueF(0.0);
    }

    return pseudoColor;
}
