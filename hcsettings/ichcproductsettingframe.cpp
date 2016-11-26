#include "ichcproductsettingframe.h"
#include "ui_ichcproductsettingframe.h"

#include "iclineeditwrapper.h"
#include "icmold.h"
#include "icvirtualhost.h"
#include "icvirtualkey.h"
#include "icparameterssave.h"
#include "icconfigstring.h"


ICHCProductSettingFrame::ICHCProductSettingFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::ICHCProductSettingFrame)
{
    ui->setupUi(this);
    buttongroup_ = ui->buttonGroup ;
    InitCheckBox();
    ui->productLineEdit->setValidator(new QIntValidator(0, 65535, ui->productLineEdit));
    ui->alarmTimesEdit->setValidator(new QIntValidator(0, 65535, ui->alarmTimesEdit));
    ui->tryProductEdit->setValidator(new QIntValidator(0, 65535, ui->tryProductEdit));
    ui->samplingEdit->setValidator(new QIntValidator(0, 65535, ui->samplingEdit));
    ui->samplingEdit_3->setValidator(new QIntValidator(0, 65535, ui->samplingEdit_3));
    ui->waitTimeEdit->SetDecimalPlaces(1);
    ui->waitTimeEdit->setValidator(new QIntValidator(0, 32760, ui->waitTimeEdit));
    ui->recycleTimeEdit->SetDecimalPlaces(1);
    ui->recycleTimeEdit->setValidator(new QIntValidator(0, 30000, ui->recycleTimeEdit));
    ui->realRecycleTimeEdit->SetDecimalPlaces(1);
    ui->realRecycleTimeEdit->setValidator(new QIntValidator(0, 3000, ui->realRecycleTimeEdit));
    ICLineEditWrapper *wrapper = new ICLineEditWrapper(ui->productLineEdit,
                                                       ICMold::Product,
                                                       ICLineEditWrapper::Mold,
                                                       ICLineEditWrapper::Integer);
    wrappers_.append(wrapper);

    wrapper = new ICLineEditWrapper(ui->tryProductEdit,
                                                       ICMold::TryProduct,
                                                       ICLineEditWrapper::Mold,
                                                       ICLineEditWrapper::Integer);
    wrappers_.append(wrapper);

    wrapper = new ICLineEditWrapper(ui->samplingEdit,
                                                       ICMold::Sampling,
                                                       ICLineEditWrapper::Mold,
                                                       ICLineEditWrapper::Integer);
    wrappers_.append(wrapper);

    wrapper = new ICLineEditWrapper(ui->recycleTimeEdit,
                                                       ICMold::CheckClip8,
                                                       ICLineEditWrapper::Mold,
                                                       ICLineEditWrapper::OneFraction);
    wrappers_.append(wrapper);

    wrapper = new ICLineEditWrapper(ui->waitTimeEdit,
                                    ICVirtualHost::SM_WaitMoldOpenLimit,
                                    ICLineEditWrapper::System,
                                    ICLineEditWrapper::OneFraction);
    wrappers_.append(wrapper);

    wrapper = new ICLineEditWrapper(ui->alarmTimesEdit,
                                    ICVirtualHost::SM_ACCTIME,
                                    ICLineEditWrapper::System,
                                    ICLineEditWrapper::Integer);
    wrappers_.append(wrapper);

//    int currentPos = ICMold::CurrentMold()->MoldParam(ICMold::PosMainDown);
//    if(currentPos > 1)
//    {
//        currentPos = 1;
//    }
//    buttongroup_->setId(ICVirtualHost::GlobalVirtualHost()->FixtureDefine());
    ui->reversedCheckBox->blockSignals(true);
    ui->positiveCheckBox->blockSignals(true);
    if(ICVirtualHost::GlobalVirtualHost()->FixtureDefine() == 0)
        ui->reversedCheckBox->click();
    if(ICVirtualHost::GlobalVirtualHost()->FixtureDefine() == 1)
        ui->positiveCheckBox->click();
    ui->reversedCheckBox->blockSignals(false);
    ui->positiveCheckBox->blockSignals(false);
//    int v = ICVirtualHost::GlobalVirtualHost()->SystemParameter(ICVirtualHost::SYS_Config_Fixture).toInt();
//    v &= 0xFFFF;
//    v >>= 15;
//    ui->fixtureComboBox->setCurrentIndex((ICVirtualHost::GlobalVirtualHost()->SystemParameter(ICVirtualHost::SYS_Config_Fixture).toInt() >> 15) & 1);
//    ui->fixtureComboBox->setCurrentIndex(v);
    ui->getFailWay->setCurrentIndex(ICVirtualHost::GlobalVirtualHost()->GetFailAlarmWay());

    connect(ICMold::CurrentMold(),
            SIGNAL(MoldNumberParamChanged()),
            this,
            SLOT(OnMoldNumberParamChanged()));

    connect(ui->productClearButton,SIGNAL(clicked()), SLOT(OnProductClearButtonClicked()), Qt::UniqueConnection);
    ui->countUnitBox->setCurrentIndex(ICMold::CurrentMold()->MoldParam(ICMold::CountUnit));
    ui->productSave->blockSignals(true);
    ui->productSave->setChecked(ICParametersSave::Instance()->IsProductSave());
    ui->productSave->blockSignals(false);
    ui->recycleMode->blockSignals(true);
    ui->recycleMode->setCurrentIndex(ICVirtualHost::GlobalVirtualHost()->RecycleMode());
    ui->recycleMode->blockSignals(false);
    ui->realRecycleTimeEdit->blockSignals(true);
    ui->realRecycleTimeEdit->SetThisIntToThisText(ICVirtualHost::GlobalVirtualHost()->RecycleTime());
    ui->realRecycleTimeEdit->blockSignals(false);

    editorToConfigIDs_.insert(ui->productLineEdit, ICConfigString::kCS_PRD_Number);
    editorToConfigIDs_.insert(ui->waitTimeEdit, ICConfigString::kCS_PRD_Wait_OM_Limit);
    editorToConfigIDs_.insert(ui->alarmTimesEdit, ICConfigString::kCS_PRD_Alarm_Time);
    editorToConfigIDs_.insert(ui->recycleTimeEdit, ICConfigString::kCS_PRD_Cycle_Time);
    editorToConfigIDs_.insert(ui->buttonGroup, ICConfigString::kCS_PRD_Fixture_Define);
    editorToConfigIDs_.insert(ui->countUnitBox, ICConfigString::kCS_PRD_Transport_Count_Way);
    editorToConfigIDs_.insert(ui->productSave, ICConfigString::kCS_PRD_Save_Count);
    editorToConfigIDs_.insert(ui->getFailWay, ICConfigString::kCS_PRD_Alarm_Occasion_When_Get_Fail);
    editorToConfigIDs_.insert(ui->tryProductEdit, ICConfigString::kCS_PRD_Try_number);
    editorToConfigIDs_.insert(ui->samplingEdit, ICConfigString::kCS_PRD_Sample_cycle);
    editorToConfigIDs_.insert(ui->recycleMode, ICConfigString::kCS_PRD_Reclcle_Mode);
    editorToConfigIDs_.insert(ui->realRecycleTimeEdit, ICConfigString::kCS_PRD_Reclcle_Time);
    ICLogInit;

    this->hide();
    ui->label_9->hide();
    ui->samplingEdit_3->hide();

#ifdef Compatible6410
    ui->productSave->hide();
#endif


}

ICHCProductSettingFrame::~ICHCProductSettingFrame()
{
    delete ui;
    qDeleteAll(wrappers_);
    delete buttongroup_;
}

void ICHCProductSettingFrame::hideEvent(QHideEvent *e)
{
    qDebug("Product hide");
    ICVirtualHost* host = ICVirtualHost::GlobalVirtualHost();
    if(host->IsParamChanged())
    {
        ICSetAxisConfigsCommand command;
        ICCommandProcessor* process = ICCommandProcessor::Instance();
        int sum = 0;
        QVector<uint> dataBuffer(7, 0);
        dataBuffer[0] = host->SystemParameter(ICVirtualHost::SYS_Config_Signal).toUInt();
        dataBuffer[1] = host->SystemParameter(ICVirtualHost::SYS_Config_Arm).toUInt();
        dataBuffer[2] = host->SystemParameter(ICVirtualHost::SYS_Config_Out).toUInt();
        dataBuffer[3] = host->SystemParameter(ICVirtualHost::SYS_Config_Fixture).toUInt();
        dataBuffer[4] = host->SystemParameter(ICVirtualHost::SYS_Config_Resv1).toUInt();
        dataBuffer[5] = host->SystemParameter(ICVirtualHost::SYS_Config_Resv2).toUInt();
    //    dataBuffer[3] = ICVirtualHost::GlobalVirtualHost()->FixtureDefine();
        for(int i = 0; i != 6; ++i)
        {
            sum += dataBuffer.at(i);
        }
        sum = (-sum & 0XFFFF);
        dataBuffer[6] = sum;
//        qDebug()<<sum;
        command.SetSlave(process->SlaveID());
        command.SetDataBuffer(dataBuffer);
        command.SetAxis(8);
    #ifndef Q_WS_X11
        if(process->ExecuteCommand(command).toBool())
    #endif
        {
            ICVirtualHost* host = ICVirtualHost::GlobalVirtualHost();
            host->SetSystemParameter(ICVirtualHost::SYS_Config_Signal, dataBuffer.at(0));
            host->SetAxisDefine(dataBuffer.at(1));
            host->SetPeripheryOutput(dataBuffer.at(2));
            host->SetSystemParameter(ICVirtualHost::SYS_Config_Fixture, dataBuffer.at(3));
            host->SetSystemParameter(ICVirtualHost::SYS_Config_Resv1, dataBuffer.at(4));
            host->SetSystemParameter(ICVirtualHost::SYS_Config_Resv2, dataBuffer.at(5));
            host->SetSystemParameter(ICVirtualHost::SYS_Config_Xorsum, dataBuffer.at(6));
        }
        ICMold::CurrentMold()->SaveMoldParamsFile();
        ICVirtualHost::GlobalVirtualHost()->SaveSystemConfig();
        ICVirtualHost::GlobalVirtualHost()->ReConfigure();
    }
    QFrame::hideEvent(e);
}

void ICHCProductSettingFrame::changeEvent(QEvent *e)
{
    QFrame::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
    {
      // ui->retranslateUi();
        retranslateUi_();
    }
    break;
    default:
        break;
    }
}
void ICHCProductSettingFrame::retranslateUi_()
{
    this->setWindowTitle(tr("Frame"));
    ui->label->setText(tr("Product"));
    ui->label_2->setText(tr("Wait Mold Opened Limit Time"));
    ui->label_4->setText(tr("s"));
    ui->productClearButton->setText(tr("Product Clear"));
    ui->label_7->setText(tr("TryProduct")); //试产模数
    ui->label_8->setText(tr("Sampling Interval"));
    ui->label_9->setText(tr("Bad Product"));
    ui->label_5->setText(tr("Alarm Times"));
    ui->label_6->setText(tr("Times"));
    ui->label_17->setText(tr("Fixture"));
//    ui->fixtureSelectBox->setItemText(0,tr("Reversed Phase"));
//    ui->fixtureSelectBox->setItemText(1,tr("Positive Phase"));
    ui->reversedCheckBox->setText(tr("Reversed Phase"));
    ui->positiveCheckBox->setText(tr("Positive Phase"));
    ui->getFailWay->setItemText(0, tr("Alarm When Up"));
    ui->getFailWay->setItemText(1, tr("Alarm Once"));
    ui->countUnitBox->setItemText(0, tr("All"));
    ui->countUnitBox->setItemText(1, tr("Good"));
    ui->countUnitBox->setItemText(2, tr("Stack-1"));
    ui->countUnitBox->setItemText(3, tr("Stack-2"));
    ui->countUnitBox->setItemText(4, tr("Stack-3"));
    ui->countUnitBox->setItemText(5, tr("Stack-4"));
    ui->label_18->setText(tr("Count Ways"));
    ui->label_10->setText(tr("Get Fail"));
    ui->label_19->setText(tr("Recycle Time"));
    ui->productSave->setText(tr("Product Save"));
    ui->label_20->setText(tr("Recycle Mode"));
    ui->recycleMode->setItemText(0, tr("Recycle Mode-1"));
    ui->recycleMode->setItemText(1, tr("Recycle Mode-2"));
    ui->recycleMode->setItemText(2, tr("Recycle Mode-3"));
    ui->label_3->setText(tr("Recycle-T"));

}

void ICHCProductSettingFrame::OnMoldNumberParamChanged()
{
    for(int i = 0; i != wrappers_.size(); ++i)
    {
        wrappers_[i]->UpdateParam();
    }
    ui->countUnitBox->setCurrentIndex(ICMold::CurrentMold()->MoldParam(ICMold::CountUnit));
}

void ICHCProductSettingFrame::OnProductClearButtonClicked()
{
    ICCommandProcessor::Instance()->ExecuteVirtualKeyCommand(IC::VKEY_PRODUCT_CLEAR);
    ICVirtualHost::GlobalVirtualHost()->SetFinishProductCount(0);

    ICAlarmFrame::Instance()->OnActionTriggered(ICConfigString::kCS_PRD_Product_Clear,
                                                tr("Product clear"),
                                                "");
    ICVirtualHost::GlobalVirtualHost()->SaveSystemConfig();

}

void ICHCProductSettingFrame::FixtureBoxChange()
{
    ICVirtualHost* host = ICVirtualHost::GlobalVirtualHost();
    int v = host->SystemParameter(ICVirtualHost::SYS_Config_Fixture).toInt();
    v &= 0x8000;
    v |= host->FixtureDefineSwitch(buttongroup_->checkedId());
    host->SetSystemParameter(ICVirtualHost::SYS_Config_Fixture, v);
}

void ICHCProductSettingFrame::InitCheckBox()
{
    buttongroup_->addButton(ui->reversedCheckBox,0);
    buttongroup_->addButton(ui->positiveCheckBox,1);
//    buttongroup_->addButton(ui->positiveCheckBox,2);

    QList<QAbstractButton*> buttons = buttongroup_->buttons();
    for(int i = 0; i != buttons.size(); ++i)
    {
        buttons[i]->setCheckable(true);
        connect(buttons.at(i),
                SIGNAL(clicked()),
                this,
                SLOT(FixtureBoxChange()));
    }
    buttongroup_->setExclusive(true);
}

void ICHCProductSettingFrame::on_countUnitBox_currentIndexChanged(int index)
{
    ICMold::CurrentMold()->SetMoldParam(ICMold::CountUnit, index);
}

void ICHCProductSettingFrame::on_getFailWay_activated(int index)
{
    ICVirtualHost::GlobalVirtualHost()->SetGetFailAlarmWay(index);
}

//void ICHCProductSettingFrame::on_fixtureComboBox_currentIndexChanged(int index)
//{
//    ICVirtualHost* host = ICVirtualHost::GlobalVirtualHost();
//    int v = host->SystemParameter(ICVirtualHost::SYS_Config_Fixture).toInt();
//    v &= 0x7FFF;
//    v |= (index << 15);
//    host->SetSystemParameter(ICVirtualHost::SYS_Config_Fixture, v);
//}

void ICHCProductSettingFrame::on_productSave_toggled(bool checked)
{
    ICParametersSave::Instance()->SetProductSave(checked);
}

ICLogFunctions(ICHCProductSettingFrame)

void ICHCProductSettingFrame::on_recycleMode_currentIndexChanged(int index)
{
    ICVirtualHost::GlobalVirtualHost()->SetRecycleMode(index);
}

void ICHCProductSettingFrame::on_realRecycleTimeEdit_textChanged(const QString &arg1)
{
    ICVirtualHost::GlobalVirtualHost()->SetRecycleTime(ui->realRecycleTimeEdit->TransThisTextToThisInt());
}
