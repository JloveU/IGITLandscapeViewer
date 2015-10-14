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

    //���ó�ʼֵ
    setMarkedTypeItems(QStringList());
    setMarkedTypeName(QString());
    setMarkedPointName(QString());
    setMarkedPointColor(QColor(0, 0, 255));

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
    connect(typeComboBox, SIGNAL(editTextChanged(const QString &)), this, SLOT(autoSetMarkedPointName(const QString &)));

    //���������������ݸı�ʱ����ɫ��Ӧ�ı�
    connect(typeComboBox, SIGNAL(editTextChanged(const QString &)), this, SLOT(autoSetMarkedPointColor(const QString &)));

    //��ȷ������ť����¼�
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
        QMessageBox::information(this, QString::fromAscii("����"), QString::fromAscii("��ǰ��ɫ���Բ��ɸ��ģ���Ϊͬһ���͵ı�ǵ������ɫ��ͬ��"));
        return;
    }
    QColor color = QColorDialog::getColor(Qt::white, this);
    setMarkedPointColor(color);
}

void SetMarkedPointPropertyDlg::show()
{
    assert(mMarkedPointTreeRoot);

    ////���ø��ؼ���״̬������
    //��ȡ��ǵ����ǩ�������ӱ�ǩ�����ƣ�����ǵ�������𣩣������ڵ����Ի�������ʾ��ѡ��
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

        //���Ԥ������ɫ�б����Ƿ���δʹ�õ���ɫ
        bool hasUnusedColorInList = false;
        for (unsigned colorIndex = 0; colorIndex < mColorList.size(); colorIndex++)
        {
            color = mColorList.at(colorIndex);

            //����Ƿ���ʹ�ù��˴���ɫ
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
            setMarkedPointColor(QColor(0, 0, 0)); //��Ҫ�ֶ�������ɫ
        }

        mColorSelectorLocked = false;
        return;
    }
}

void SetMarkedPointPropertyDlg::checkInputAndCommit()
{
    assert(mMarkedPointTreeRoot);

    //ȷ�����������Ϊ��
    QString markedTypeName = getMarkedTypeName();
    if (markedTypeName.isEmpty())
    {
        QMessageBox::information(this, QString::fromAscii("����"), QString::fromAscii("����ı�ǵ��������Ʋ���Ϊ�գ����������룡"));
        return;
    }

    //ȷ�����������Ϊ��
    QString markedPointName = getMarkedPointName();
    if (markedPointName.isEmpty())
    {
        QMessageBox::information(this, QString::fromAscii("����"), QString::fromAscii("����ı�ǵ����Ʋ���Ϊ�գ����������룡"));
        return;
    }

    //��֤ͬһ������û������
    ccHObject *markedType = mMarkedPointTreeRoot->findDirectChild(markedTypeName);
    if (markedType)
    {
        if (markedType->findDirectChild(markedPointName))
        {
            QMessageBox::information(this, QString::fromAscii("����"), QString::fromAscii("����ı�ǵ������Ѵ��ڣ����������룡"));
            return;
        }
    }

    //��֤��ͬ���͵���ɫ��ͬ
    const QColor markedPointColor = getMarkedPointColor();
    for (unsigned i = 0; i < mMarkedPointTreeRoot->getChildrenNumber(); i++)
    {
        ccHObject *markedType = mMarkedPointTreeRoot->getChild(i);
        if (markedType->getName() != markedTypeName && markedType->getChildrenNumber() > 0)
        {
            QColor currentTypeColor = static_cast<MarkedPoint*>(markedType->getChild(0))->getColor();
            if (currentTypeColor == markedPointColor)
            {
                QMessageBox::information(this, QString::fromAscii("����"), QString::fromAscii("ѡ�����ɫ�ѱ���������ռ�ã�������ѡ��"));
                return;
            }
        }
    }

    //���汾������״̬���Թ��´���ʾ
    mMarkedTypeName = getMarkedTypeName();
    mMarkedPointColor = getMarkedPointColor();

    hide();
    emit accepted();
    emit committed(getMarkedTypeName(), getMarkedPointName(), getMarkedPointColor());
}

QColor SetMarkedPointPropertyDlg::gray2PseudoColor(float grayValue)
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
