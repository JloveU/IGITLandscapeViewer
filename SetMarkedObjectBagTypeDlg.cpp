#include "SetMarkedObjectBagTypeDlg.h"

SetMarkedObjectBagTypeDlg::SetMarkedObjectBagTypeDlg(QWidget* parent)
    : QDialog(parent)
    , Ui::SetMarkedObjectBagTypeDlg()
{
    setupUi(this);

    //ѡ�����͵�RadioButtonGroup
    mTypeRadioButtonGroup = new QButtonGroup(this);
    mTypeRadioButtonGroup->addButton(radioButtonPoint, MarkedObjectBag::POINT);
    mTypeRadioButtonGroup->addButton(radioButtonLine,  MarkedObjectBag::LINE);
    mTypeRadioButtonGroup->addButton(radioButtonArea,  MarkedObjectBag::AREA);
    radioButtonPoint->setChecked(true);

    //��ȷ������ť����¼�
    connect(okButton, SIGNAL(clicked()), this, SLOT(checkInputAndCommit()));

    //ȥ����ȡ������ť
    cancelButton->setVisible(false);
}

void SetMarkedObjectBagTypeDlg::setMarkedObjectBagType(const MarkedObjectBag::Type markedObjectBagType)
{
    mTypeRadioButtonGroup->button(markedObjectBagType)->setChecked(true);
}

MarkedObjectBag::Type SetMarkedObjectBagTypeDlg::getMarkedObjectBagType()
{
    return static_cast<MarkedObjectBag::Type>(mTypeRadioButtonGroup->checkedId());
}

void SetMarkedObjectBagTypeDlg::checkInputAndCommit()
{
    hide();
    emit accepted();
    emit committed(getMarkedObjectBagType());
}
