#include "SetMarkedObjectPropertyDlg.h"
#include <QColorDialog>
#include <QBrush>
#include <QMessageBox>
#include <QtAlgorithms>
#include "MarkedObject.h"

SetMarkedObjectPropertyDlg::SetMarkedObjectPropertyDlg(QWidget* parent)
    : QDialog(parent)
    , Ui::SetMarkedObjectPropertyDlg()
    , mMarkedObjectTreeRoot(NULL)
    , mColorSelectorLocked(false)
{
    setupUi(this);

    //���ó�ʼֵ
    setMarkedTypeItems(QStringList());
    setMarkedTypeName(QString());
    setMarkedObjectName(QString());
    setMarkedObjectColor(QColor(0, 0, 255));

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
    connect(colorSelectButton, SIGNAL(clicked()), this, SLOT(selectColor()));

    //���������������ݸı�ʱ�������������Ӧ�ı�
    connect(typeComboBox, SIGNAL(editTextChanged(const QString &)), this, SLOT(autoSetMarkedObjectName(const QString &)));

    //���������������ݸı�ʱ����ɫ��Ӧ�ı�
    connect(typeComboBox, SIGNAL(editTextChanged(const QString &)), this, SLOT(autoSetMarkedObjectColor(const QString &)));

    //��ȷ������ť����¼�
    connect(okButton, SIGNAL(clicked()), this, SLOT(checkInputAndCommit()));
}

void SetMarkedObjectPropertyDlg::setMarkedObjectTreeRoot(ccHObject *markedObjectTreeRoot)
{
    mMarkedObjectTreeRoot = markedObjectTreeRoot;
    autoSetMarkedObjectName(QString());
    autoSetMarkedObjectColor(QString());
}

void SetMarkedObjectPropertyDlg::setMarkedTypeItems(const QStringList &items)
{
    typeComboBox->clear();
    typeComboBox->addItems(items);
}

void SetMarkedObjectPropertyDlg::setMarkedTypeName(const QString &name)
{
    typeComboBox->setCurrentText(name);
}

QString SetMarkedObjectPropertyDlg::getMarkedTypeName()
{
    return typeComboBox->currentText();
}

void SetMarkedObjectPropertyDlg::setMarkedObjectName(const QString &name)
{
    nameLineEdit->setText(name);
}

QString SetMarkedObjectPropertyDlg::getMarkedObjectName()
{
    return nameLineEdit->text();
}

void SetMarkedObjectPropertyDlg::setMarkedObjectColor(const QColor &color)
{
    mMarkedObjectColor = color;
    colorSelectButton->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(color.red()).arg(color.green()).arg(color.blue()));
}

QColor SetMarkedObjectPropertyDlg::getMarkedObjectColor()
{
    return mMarkedObjectColor;
}

void SetMarkedObjectPropertyDlg::selectColor()
{
    if (mColorSelectorLocked)
    {
        QMessageBox::information(this, QString::fromAscii("����"), QString::fromAscii("��ǰ��ɫ���Բ��ɸ��ģ���Ϊͬһ���͵ı�ǵ������ɫ��ͬ��"));
        return;
    }
    QColor color = QColorDialog::getColor(Qt::white, this);
    if (color.isValid())
    {
        setMarkedObjectColor(color);
    }
}

void SetMarkedObjectPropertyDlg::show()
{
    assert(mMarkedObjectTreeRoot);

    ////���ø��ؼ���״̬������
    //��ȡ��ǵ����ǩ�������ӱ�ǩ�����ƣ�����ǵ�������𣩣������ڵ����Ի�������ʾ��ѡ��
    QStringList markTypeItems;
    for (int i = 0; i < mMarkedObjectTreeRoot->getChildrenNumber(); i++)
    {
        markTypeItems.push_back(mMarkedObjectTreeRoot->getChild(i)->getName());
    }
    setMarkedTypeItems(markTypeItems);
    setMarkedTypeName(mMarkedTypeName);
    autoSetMarkedObjectName(mMarkedTypeName);
    setMarkedObjectColor(mMarkedObjectColor);

    QDialog::show();
}

void SetMarkedObjectPropertyDlg::autoSetMarkedObjectName(const QString &markedTypeName)
{
    assert(mMarkedObjectTreeRoot);

    if (markedTypeName.isEmpty())
    {
        setMarkedObjectName("");
        return;
    }

    ccHObject *markedType = mMarkedObjectTreeRoot->findDirectChild(markedTypeName);
    if (markedType)
    {
        int nameSuffixNumber = 1;
        ccHObject *markedObject = markedType->findDirectChild(markedTypeName + QString::number(nameSuffixNumber));
        while (markedObject)
        {
            nameSuffixNumber++;
            markedObject = markedType->findDirectChild(markedTypeName + QString::number(nameSuffixNumber));
        }
        setMarkedObjectName(markedTypeName + QString::number(nameSuffixNumber));
        return;
    }
    else
    {
        setMarkedObjectName(markedTypeName + "1");
        return;
    }
}

void SetMarkedObjectPropertyDlg::autoSetMarkedObjectColor(const QString &markedTypeName)
{
    assert(mMarkedObjectTreeRoot);

    if (markedTypeName.isEmpty())
    {
        setMarkedObjectColor(QColor(0, 0, 255));
        mColorSelectorLocked = false;
        return;
    }

    ccHObject *markedType = mMarkedObjectTreeRoot->findDirectChild(markedTypeName);
    if (markedType)
    {
        if (markedType->getChildrenNumber() > 0)
        {
            MarkedObject *markedObject = dynamic_cast<MarkedObject*>(markedType->getChild(0));
            assert(markedObject);
            setMarkedObjectColor(markedObject->getColor());
            mColorSelectorLocked = true;
            return;
        }
        else
        {
            setMarkedObjectColor(QColor(0, 0, 255));
            mColorSelectorLocked = false;
            return;
        }
    }
    else
    {
        QColor color;

        //���Ԥ������ɫ�б����Ƿ���δʹ�õ���ɫ
        bool hasUnusedColorInList = false;
        for (unsigned colorIndex = 0; colorIndex < mColorList.size(); colorIndex++)
        {
            color = mColorList.at(colorIndex);

            //����Ƿ���ʹ�ù��˴���ɫ
            bool currentColorHasUsed = false;
            for (unsigned i = 0; i < mMarkedObjectTreeRoot->getChildrenNumber(); i++)
            {
                ccHObject *markedType = mMarkedObjectTreeRoot->getChild(i);
                if (markedType->getChildrenNumber() > 0)
                {
                    QColor currentTypeColor = static_cast<MarkedObject*>(markedType->getChild(0))->getColor();
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
            setMarkedObjectColor(color);
        }
        else
        {
            setMarkedObjectColor(QColor(0, 0, 0)); //��Ҫ�ֶ�������ɫ
        }

        mColorSelectorLocked = false;
        return;
    }
}

void SetMarkedObjectPropertyDlg::checkInputAndCommit()
{
    assert(mMarkedObjectTreeRoot);

    //ȷ�����������Ϊ��
    QString markedTypeName = getMarkedTypeName();
    if (markedTypeName.isEmpty())
    {
        QMessageBox::information(this, QString::fromAscii("����"), QString::fromAscii("����ı�ǵ��������Ʋ���Ϊ�գ����������룡"));
        return;
    }

    //ȷ�����������Ϊ��
    QString markedObjectName = getMarkedObjectName();
    if (markedObjectName.isEmpty())
    {
        QMessageBox::information(this, QString::fromAscii("����"), QString::fromAscii("����ı�ǵ����Ʋ���Ϊ�գ����������룡"));
        return;
    }

    //��֤ͬһ������û������
    ccHObject *markedType = mMarkedObjectTreeRoot->findDirectChild(markedTypeName);
    if (markedType)
    {
        if (markedType->findDirectChild(markedObjectName))
        {
            QMessageBox::information(this, QString::fromAscii("����"), QString::fromAscii("����ı�ǵ������Ѵ��ڣ����������룡"));
            return;
        }
    }

    //��֤��ͬ���͵���ɫ��ͬ
    const QColor markedObjectColor = getMarkedObjectColor();
    for (unsigned i = 0; i < mMarkedObjectTreeRoot->getChildrenNumber(); i++)
    {
        ccHObject *markedType = mMarkedObjectTreeRoot->getChild(i);
        if (markedType->getName() != markedTypeName && markedType->getChildrenNumber() > 0)
        {
            QColor currentTypeColor = static_cast<MarkedObject*>(markedType->getChild(0))->getColor();
            if (currentTypeColor == markedObjectColor)
            {
                QMessageBox::information(this, QString::fromAscii("����"), QString::fromAscii("ѡ�����ɫ�ѱ���������ռ�ã�������ѡ��"));
                return;
            }
        }
    }

    //���汾������״̬���Թ��´���ʾ
    mMarkedTypeName = getMarkedTypeName();
    mMarkedObjectColor = getMarkedObjectColor();

    emit committed(getMarkedTypeName(), getMarkedObjectName(), getMarkedObjectColor());
    accept();
}

QColor SetMarkedObjectPropertyDlg::gray2PseudoColor(float grayValue)
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
