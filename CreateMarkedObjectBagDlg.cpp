#include "CreateMarkedObjectBagDlg.h"
#include <QColorDialog>
#include <QBrush>
#include <QMessageBox>
#include <QtAlgorithms>
#include "MarkedObjectBag.h"

CreateMarkedObjectBagDlg::CreateMarkedObjectBagDlg(QWidget* parent)
    : QDialog(parent)
    , Ui::CreateMarkedObjectBagDlg()
    , mMarkedObjectBagDBTreeParent(NULL)
    , mMarkedObjectBagColor()
    , mColorSelectorLocked(false)
    , mColorList()
    , mCreatedObjectCount(0)
{
    setupUi(this);

    //ѡ�����������͵�RadioButtonGroup
    mRadioButtonGroupObjectBagType = new QButtonGroup(this);
    mRadioButtonGroupObjectBagType->addButton(radioButtonMarkedObjectTypePoint, MarkedObjectBag::POINT);
    mRadioButtonGroupObjectBagType->addButton(radioButtonMarkedObjectTypeLine,  MarkedObjectBag::LINE);
    mRadioButtonGroupObjectBagType->addButton(radioButtonMarkedObjectTypeArea,  MarkedObjectBag::AREA);
    radioButtonMarkedObjectTypePoint->setChecked(true);

    //���ó�ʼֵ
//     setMarkedObjectBagDBTreeParent(NULL);
//     setMarkedObjectBagName("UntitledObject");
//     setMarkedObjectBagType(MarkedObjectBag::POINT);
//     setMarkedObjectBagColor(QColor(0, 0, 0));

    //Ϊ��ͬ���Ԥ������ɫ�б�
    //��ɫ���䷽��˵�������Ҷȴ�0.0��1.0��Ӧ��α��ɫ���������죩���Ҷ�����Ϊ0.0��1.0��0.5��0.25��0.75��0.125��0.375��0.625��0.875����
    const unsigned PRE_CREATE_COLOR_NUM = 100; //Ĭ��Ԥ����100�����͵���ɫ
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

    //����ѡ����ɫ��ť��Ӧ
    connect(buttonSelectColor, SIGNAL(clicked()), this, SLOT(selectColor()));

    //��ȷ������ť����¼�
    connect(okButton, SIGNAL(clicked()), this, SLOT(checkInputAndCommit()));
}

void CreateMarkedObjectBagDlg::setMarkedObjectBagDBTreeParent(ccHObject *parent)
{
    mMarkedObjectBagDBTreeParent = parent;
    autoSetMarkedObjectBagName();
    autoSetMarkedObjectBagType();
    autoSetMarkedObjectBagColor();
}

// ccHObject* CreateMarkedObjectBagDlg::getMarkedObjectBagDBTreeParent()
// {
//     return mMarkedObjectBagDBTreeParent;
// }

void CreateMarkedObjectBagDlg::setMarkedObjectBagType(const MarkedObjectBag::Type type)
{
    mRadioButtonGroupObjectBagType->button(type)->setChecked(true);
}

MarkedObjectBag::Type CreateMarkedObjectBagDlg::getMarkedObjectBagType()
{
    return static_cast<MarkedObjectBag::Type>(mRadioButtonGroupObjectBagType->checkedId());
}

void CreateMarkedObjectBagDlg::setMarkedObjectBagName(const QString &name)
{
    lineEditName->setText(name);
}

QString CreateMarkedObjectBagDlg::getMarkedObjectBagName()
{
    return lineEditName->text();
}

void CreateMarkedObjectBagDlg::setMarkedObjectBagColor(const QColor &color)
{
    mMarkedObjectBagColor = color;
    buttonSelectColor->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(color.red()).arg(color.green()).arg(color.blue()));
}

QColor CreateMarkedObjectBagDlg::getMarkedObjectBagColor()
{
    return mMarkedObjectBagColor;
}

void CreateMarkedObjectBagDlg::selectColor()
{
    if (mColorSelectorLocked)
    {
        QMessageBox::information(this, QString::fromAscii("����"), QString::fromAscii("��ǰ��ɫ���Բ��ɸ��ģ���Ϊͬһ���ڵ��µ�������ɫ������ͬ��"));
        return;
    }
    QColor color = QColorDialog::getColor(Qt::white, this);
    setMarkedObjectBagColor(color);
}

int CreateMarkedObjectBagDlg::exec()
{
    assert(mMarkedObjectBagDBTreeParent);

    autoSetMarkedObjectBagName();
    autoSetMarkedObjectBagType();
    autoSetMarkedObjectBagColor();

    return QDialog::exec();
}

void CreateMarkedObjectBagDlg::autoSetMarkedObjectBagName()
{
    assert(mMarkedObjectBagDBTreeParent);

    const QString dbTreeParentName = mMarkedObjectBagDBTreeParent->getName();

    int nameSuffixNumber = 1;
    ccHObject *markedObject = mMarkedObjectBagDBTreeParent->findDirectChild(dbTreeParentName + QString::number(nameSuffixNumber));
    while (markedObject)
    {
        nameSuffixNumber++;
        markedObject = mMarkedObjectBagDBTreeParent->findDirectChild(dbTreeParentName + QString::number(nameSuffixNumber));
    }
    setMarkedObjectBagName(dbTreeParentName + QString::number(nameSuffixNumber));
}

void CreateMarkedObjectBagDlg::autoSetMarkedObjectBagType()
{
    /*assert(mMarkedObjectBagDBTreeParent);

    if (mMarkedObjectBagDBTreeParent->getChildrenNumber() > 0)
    {
        MarkedObjectBag *markedObjectBag = dynamic_cast<MarkedObjectBag*>(mMarkedObjectBagDBTreeParent->getChild(0));
        assert(markedObjectBag);
        setMarkedObjectBagType(markedObjectBag->getType());
        
        //��ֹ��������������
        mRadioButtonGroupObjectBagType->button(MarkedObjectBag::POINT)->setEnabled(false);
        mRadioButtonGroupObjectBagType->button(MarkedObjectBag::LINE)->setEnabled(false);
        mRadioButtonGroupObjectBagType->button(MarkedObjectBag::AREA)->setEnabled(false);

        return;
    }
    else
    {
        //�����������������
        mRadioButtonGroupObjectBagType->button(MarkedObjectBag::POINT)->setEnabled(true);
        mRadioButtonGroupObjectBagType->button(MarkedObjectBag::LINE)->setEnabled(true);
        mRadioButtonGroupObjectBagType->button(MarkedObjectBag::AREA)->setEnabled(true);

        return;
    }*/
}

void CreateMarkedObjectBagDlg::autoSetMarkedObjectBagColor()
{
    /*assert(mMarkedObjectBagDBTreeParent);

    if (mMarkedObjectBagDBTreeParent->getChildrenNumber() > 0)
    {
        MarkedObjectBag *markedObjectBag = dynamic_cast<MarkedObjectBag*>(mMarkedObjectBagDBTreeParent->getChild(0));
        assert(markedObjectBag);
        setMarkedObjectBagColor(markedObjectBag->getColor());
        //mColorSelectorLocked = true;
        return;
    }
    else
    {
        setMarkedObjectBagColor(mCreatedObjectCount < mColorList.size() ? mColorList.at(mCreatedObjectCount) : QColor(255, 0, 0));
        //mColorSelectorLocked = false;
        return;
    }*/

    setMarkedObjectBagColor(mCreatedObjectCount < mColorList.size() ? mColorList.at(mCreatedObjectCount) : QColor(255, 0, 0));
}

void CreateMarkedObjectBagDlg::checkInputAndCommit()
{
    assert(mMarkedObjectBagDBTreeParent);

    //ȷ�����������Ϊ��
    if (getMarkedObjectBagName().isEmpty())
    {
        QMessageBox::information(this, QString::fromAscii("����"), QString::fromAscii("��������Ʋ���Ϊ�գ����������룡"));
        return;
    }

    //��֤ͬһ���ڵ���û������
    if (mMarkedObjectBagDBTreeParent->findDirectChild(getMarkedObjectBagName()))
    {
        QMessageBox::information(this, QString::fromAscii("����"), QString::fromAscii("����������Ѵ��ڣ����������룡"));
        return;
    }

    mCreatedObjectCount++;
    emit committed(getMarkedObjectBagName(), getMarkedObjectBagType(), getMarkedObjectBagColor());
    accept();
}

QColor CreateMarkedObjectBagDlg::gray2PseudoColor(float grayValue)
{
    //��grayValue�淶����0��1֮��
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
