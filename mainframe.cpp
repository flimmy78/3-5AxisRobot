/* If win32 and no cygwin, suppose it's MinGW or any other native windows compiler. */
#if defined(WIN32) && !defined(__CYGWIN__)
#define NATIVE_WIN32
#endif /* win32 and no cygwin */

#ifndef NATIVE_WIN32
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#endif

#include <QBoxLayout>
#include <QButtonGroup>
#include <QKeyEvent>
#include <QMessageBox>
#include <QRunnable>
#include <QStackedLayout>
#include <QThreadPool>
#include <QTranslator>
#include <QDir>


#include "ui_mainframe.h"

#include "icactioncommand.h"
#include "icalarmdescriptiondialog.h"
#include "icalarmframe.h"
#include "iccycletimeandfinishedframe.h"
#include "icfunctionpageframe.h"
#include "ichcinstructionpageframe.h"
#include "ichcmanualoperationpageframe.h"
#include "ichcprogrammonitorframe.h"
//#include "ichctimeframe.h"
#include "icinitialframe.h"
#include "iciomonitor.h"
#include "ickeyboard.h"
#include "icmonitorpageframe.h"
#include "icorigindialog.h"
#include "icparameterssave.h"
#include "icprogramheadframe.h"
#include "icreturnpage.h"
#include "icsystemstatusframe.h"
#include "icvirtualhost.h"
#include "mainframe.h"
#include "moldinformation.h"
#include "icactiondialog.h"
#include "ictimerpool.h"
#include "ichostcomparepage.h"
#include "icupdatesystem.h"
#if defined(Q_WS_WIN32) || defined(Q_WS_X11)
#include "simulateknob.h"
#endif

#include <QDebug>

//class ICScreenSaverMonitor: public QRunnable
//{
//public:
//    ICScreenSaverMonitor(MainFrame* mainFrame) { mainFrame_ = mainFrame;}
//    void run()
//    {
//        int fd = open("/dev/screensaver", O_RDONLY);
//        if(fd < 0)
//        {
//            perror("Open screensaver fail \n");
//            return;
//        }
//        char readStr[30];
//        while(1)
//        {
//            memset(readStr, '\0', 30);
//            read(fd, readStr, 30);
//            qDebug(readStr);
//            if(strcmp(readStr, "BackLight Off") == 0)
//            {
//                mainFrame_->SetBackLightOff();
//                qDebug("set backLightOFF");
//            }
//        }
//    }
//private:
//    MainFrame* mainFrame_;
//};


MainFrame *icMainFrame = NULL;
MainFrame::MainFrame(QSplashScreen *splashScreen, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainFrame),
    monitorPage_(NULL),
    centerStackedLayout_(new QStackedLayout),
    ledFlags_(0),
    errCode_(0),
    oldRunnigStatus_(0),
    oldXPos_(-1),
    oldYPos_(-1),
    oldZPos_(-1),
    oldX2Pos_(-1),
    oldY2Pos_(-1),
    oldAPos_(-1),
    oldBPos_(-1),
    oldCPos_(-1),
    isOriginShown_(false),
    isReturnShown_(false),
    oldFinishCount_(0),
    cycleTime_(0),
    oldCycleTime_(0),
    oldStep_(0),
    screenSaver_(new ICScreenSaver()),
    isBackLightOff_(false),
    isOrigined_(false),
    isDoAction_(false),
    isXPosChanged_(false),
    isYPosChanged_(false),
    isZPosChanged_(false),
    isX2PosChanged_(false),
    isY2PosChanged_(false),
    isAPosChanged_(false),
    isBPosChanged_(false),
    isCPosChanged_(false),
    axisDefine_(-1),
    registe_timer(new QTimer),
    reboot_timer(new QTimer)
{
    connect(this,
            SIGNAL(LoadMessage(QString)),
            splashScreen,
            SLOT(showMessage(QString)));
    emit LoadMessage("Connected");
    ui->setupUi(this);


    InitSpareTime();
    connect(ICUpdateSystem::Instance(),
            SIGNAL(RegisterSucceed()),
            this,
            SLOT(InitSpareTime()));

    connect(registe_timer,SIGNAL(timeout()),this,SLOT(CountRestTime()));
    connect(reboot_timer,SIGNAL(timeout()),this,SLOT(Register()));

    ui->systemStatusFrame->SetOriginStatus(StatusLabel::OFFSTATUS);
    QDir configDir("./sysconfig");
    configDir.setFilter(QDir::Files);
    QStringList backupFiles = configDir.entryList(QStringList()<<"*~");
    if(!backupFiles.isEmpty())
    {
        for(int i = 0; i != backupFiles.size(); ++i)
        {
            configDir.remove(backupFiles.at(i).left(backupFiles.at(i).size() - 1));
            configDir.rename(backupFiles.at(i), backupFiles.at(i).left(backupFiles.at(i).size() - 1));
        }
        //        QMessageBox::critical(this, tr("Warning"), tr("System Configs has been recover, please check the configs first!"));
    }
    configDir.cd("../records/");
    backupFiles = configDir.entryList(QStringList()<<"*~");
    if(!backupFiles.isEmpty())
    {
        for(int i = 0; i != backupFiles.size(); ++i)
        {
            configDir.remove(backupFiles.at(i).left(backupFiles.at(i).size() - 1));
            configDir.rename(backupFiles.at(i), backupFiles.at(i).left(backupFiles.at(i).size() - 1));
        }
        //        QMessageBox::critical(this, tr("Warning"), tr("Record has been recover, please check the record first!"));
    }
    configDir.cd("../subs/");
    backupFiles = configDir.entryList(QStringList()<<"*~");
    if(!backupFiles.isEmpty())
    {
        for(int i = 0; i != backupFiles.size(); ++i)
        {
            configDir.remove(backupFiles.at(i).left(backupFiles.at(i).size() - 1));
            configDir.rename(backupFiles.at(i), backupFiles.at(i).left(backupFiles.at(i).size() - 1));
        }
        //        QMessageBox::critical(this, tr("Warning"), tr("Sub has been recover, please check the sub first!"));
    }
    icMainFrame = this;
    screenSaver_->hide();
    buttonGroup_ = new QButtonGroup();
    nullButton_ = new ICPageSwitch();
    nullButton_->hide();
    buttonGroup_->addButton(ui->functionPageButton);
    buttonGroup_->addButton(ui->monitorPageButton);
    buttonGroup_->addButton(ui->alarmPageButton);
    buttonGroup_->addButton(ui->recordPageButton);
    buttonGroup_->addButton(nullButton_);
    buttonGroup_->setExclusive(true);
    foreach(QAbstractButton* button, buttonGroup_->buttons())
    {
        button->setCheckable(true);
    }
    emit LoadMessage("MainFrame UI Loaded");
#ifndef Q_WS_X11
#ifndef Q_WS_WIN32
    this->setWindowFlags(Qt::FramelessWindowHint);
#endif
#endif
    emit LoadMessage("Reset the window hint");
    //    connect(ICVirtualHost::GlobalVirtualHost(),
    //            SIGNAL(StatusRefreshed()),
    //            this,
    //            SLOT(StatusRefreshed()));
//    timerID_ = ICTimerPool::Instance()->Start(ICTimerPool::RefreshTime, this, SLOT(StatusRefreshed()));
    connect(&timer_,
            SIGNAL(timeout()),
            SLOT(StatusRefreshed()));
    timer_.start(ICTimerPool::RefreshTime);
    emit LoadMessage("Ready to Refresh");
    InitCategoryPage();
    InitInterface();
    UpdateTranslate();
    emit LoadMessage("Translation Loaded");
    InitSignal();
    emit LoadMessage("Signals is ready");
#ifndef Q_WS_WIN32
    ledFD_ = open("/dev/szhc_leds", O_WRONLY);
#else
    ledFD_ = 0;
#endif

    //    QThreadPool::globalInstance()->start(new ICScreenSaverMonitor(this));
    //    connect(screenSaver_.data(),
    //            SIGNAL(Unlock()),
    //            this,
    //            SLOT(SetBackLightOn()));

    //    ui->xPosLabel->hide();
    //    ui->label_3->hide();
    //    ui->label_5->hide();
    actionDialog_ = new ICActionDialog(this);
    axisWidgets_.append(QList<QWidget*>()<<ui->x1Label<<ui->x1mmLabel<<ui->xPosLabel);
    axisWidgets_.append(QList<QWidget*>()<<ui->y1Label<<ui->y1mmLabel<<ui->yPosLabel);
    axisWidgets_.append(QList<QWidget*>()<<ui->zLabel<<ui->zmmLabel<<ui->zPosLabel);
    axisWidgets_.append(QList<QWidget*>()<<ui->x2Label<<ui->x2mmLabel<<ui->pPosLabel);
    axisWidgets_.append(QList<QWidget*>()<<ui->y2Label<<ui->y2mmLabel<<ui->qPosLabel);
    axisWidgets_.append(QList<QWidget*>()<<ui->aLabel<<ui->ammLabel<<ui->aPosLabel);
    axisWidgets_.append(QList<QWidget*>()<<ui->bLabel<<ui->bmmLabel<<ui->bPosLabel);
    axisWidgets_.append(QList<QWidget*>()<<ui->cLabel<<ui->cmmLabel<<ui->cPosLabel);
    compareAlarmNums_<<134<<135<<136<<137<<182<<183<<234<<235<<236<<330<<332<<333;
    hostCompareDialog_ = new ICHostComparePage(this);
    UpdateAxisDefine_();
    ICKeyboard::Instace()->Receive();
    QTimer::singleShot(ICParametersSave::Instance()->BackLightTime() * 60000, this, SLOT(CheckedInput()));
    QTimer::singleShot(1000, this, SLOT(ClearPosColor()));

    //    QTimer::singleShot(100, this, SLOT(InitHeavyPage()));
    ICVirtualHost::GlobalVirtualHost()->SetInitFinished();

#if defined(Q_WS_WIN32) || defined(Q_WS_X11)
    simulateKnob_ = new SimulateKnob();
    simulateKnob_->show();
    connect(simulateKnob_,
            SIGNAL(manualButtonClicked()),
            SLOT(ShowManualPage()));
    connect(simulateKnob_,
            SIGNAL(stopButtonClicked()),
            SLOT(ShowStandbyPage()));
    connect(simulateKnob_,
            SIGNAL(autoButtonClicked()),
            SLOT(ShowAutoPage()));
    ICVirtualHost::GlobalVirtualHost()->SetInitFinished();
#ifdef HC_SK_5
    this->setFixedSize(640, 480);
#else
    this->setFixedSize(800, 600);
#endif

#endif
#ifdef Q_WS_X11
            ShowInstructPage();
    //       ShowManualPage();
//         ShowAutoPage();
#endif

}


MainFrame::~MainFrame()
{
//    ICTimerPool::Instance()->Stop(timerID_, this, SLOT(StatusRefreshed()));
    delete nullButton_;
    delete buttonGroup_;
    delete ui;

}


void MainFrame::closeEvent(QCloseEvent *e)
{
#ifdef Q_WS_WIN32
    simulateKnob_->close();
    delete simulateKnob_;
#endif
    QWidget::closeEvent(e);
}

void MainFrame::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
    {
        ui->retranslateUi(this);
        UpdateTranslate();
    }
        break;
    default:
        break;
    }
}

void MainFrame::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case ICKeyboard::FB_F1:
    {
        ui->functionPageButton->click();
    }
        break;
    case ICKeyboard::FB_F2:
    {
        ui->monitorPageButton->click();
    }
        break;
    case ICKeyboard::FB_F3:
    {
        ui->recordPageButton->click();
    }
        break;
    case ICKeyboard::FB_F4:
    {
        ui->alarmPageButton->click();
    }
        break;
    case ICKeyboard::FB_F5:
    {
        ui->returnPageButton->click();
    }
        break;
    default:
    {
        QWidget::keyPressEvent(e);
    }
    }
}

void MainFrame::InitCategoryPage()
{
    emit LoadMessage("Start to Initialize category pages");
    ui->centerPageFrame->setLayout(centerStackedLayout_);
    emit LoadMessage("center page layout has been setted");

    initialPage_ = new ICInitialFrame;
    functionButtonToPage_.insert(ui->returnPageButton, initialPage_);
    centerStackedLayout_->addWidget(initialPage_);
    emit LoadMessage("Standby page has been loaded");

    alarmPage_ = ICAlarmFrame::Instance();
    functionButtonToPage_.insert(ui->alarmPageButton, alarmPage_);
    centerStackedLayout_->addWidget(alarmPage_);
    emit LoadMessage("Alarm history page has been loaded");

    functionPage_ = new ICFunctionPageBackFrame;
    functionButtonToPage_.insert(ui->functionPageButton, functionPage_);
    centerStackedLayout_->addWidget(functionPage_);
    emit LoadMessage("Function page has been loaded");

    instructPage_ = new ICHCInstructionPageFrame();
    centerStackedLayout_->addWidget(instructPage_);
    autoPage_ = new ICHCProgramMonitorFrame();
    centerStackedLayout_->addWidget(autoPage_);

    recordPage_ = MoldInformation::Instance();
    qDebug()<<"recordPage Loaded";
    //    functionButtonToPage_.insert(ui->recordPageButton, recordPage_);
    qDebug()<<"recordPage Loaded 1";
    centerStackedLayout_->addWidget(recordPage_);
    emit LoadMessage("Records page has been loaded");

    manualPage_ = new ICHCManualOperationPageFrame();
    centerStackedLayout_->addWidget(manualPage_);
    emit LoadMessage("Manual page has been loaded");

    monitorPage_ = new ICMonitorPageFrame();
    functionButtonToPage_.insert(ui->monitorPageButton, monitorPage_);
    centerStackedLayout_->addWidget(monitorPage_);
    emit LoadMessage("Monitor page has been loaded");

    originExecutingPage_ = new ICOriginDialog();
    returnExecutingPage_ = new ICReturnPage();
    //    centerStackedLayout_->addWidget(originExecutingPage_);
    emit LoadMessage("Origin page has been loaded");

    //    centerStackedLayout_->setCurrentWidget(initialPage_);
    emit LoadMessage("End of initialzing category pages");
    //    centerStackedLayout_->setCurrentWidget(instructPage_);
}

void MainFrame::InitInterface()
{
    emit LoadMessage("Start to load main frame interface");
    QLayout * programHeadLayout = new QHBoxLayout;
    programHeadLayout->setContentsMargins(0,0,0,0);
    ui->programHeadWidget->setLayout(programHeadLayout);
    programHeadLayout->addWidget(ICProgramHeadFrame::Instance());
    ICProgramHeadFrame::Instance()->SetCurrentMoldName(ICParametersSave::Instance()->MoldName("TEST.act"));
    emit LoadMessage("Header has been shown");

    //    QLayout * currentchildPageNameLayout = new QHBoxLayout;
    //    currentchildPageNameLayout->setContentsMargins(0,0,0,0);
    //    ui->currentChildPageNameWidget->setLayout(currentchildPageNameLayout);
    //    currentchildPageNameLayout->addWidget(ICCurrentChildPageNameFrame::Instance());
    emit LoadMessage("Child page name section has been shown");

    ui->functionPageButton->setIcon(QPixmap(":/resource/settings.png"));
    ui->monitorPageButton->setIcon(QPixmap(":/resource/monitor.png"));
    ui->recordPageButton->setIcon(QPixmap(":/resource/records.png"));
    ui->alarmPageButton->setIcon(QPixmap(":/resource/warning.png"));
    ui->returnPageButton->setIcon(QPixmap(":resource/return.png"));
    emit LoadMessage("page switchers pixmap has been shown");

    //    this->setStyleSheet("QFrame { border: none; }");
    //    ui->centerPageFrame->setStyleSheet("QFrame#centerPageFrame{border-top: 1px solid  #000080; border-bottom: 1px solid #000080;}");
    emit LoadMessage("End of loading main frame interface");
}

void MainFrame::InitSignal()
{
    connect(ui->functionPageButton,
            SIGNAL(clicked()),
            functionPage_,
            SLOT(ShowFunctionSelectPage()));

    connect(ui->functionPageButton,
            SIGNAL(clicked()),
            this,
            SLOT(CategoryButtonClicked()));
    connect(ui->alarmPageButton,
            SIGNAL(clicked()),
            this,
            SLOT(CategoryButtonClicked()));
    connect(ui->monitorPageButton,
            SIGNAL(clicked()),
            this,
            SLOT(CategoryButtonClicked()));
    //    connect(ui->returnPageButton,
    //            SIGNAL(clicked()),
    //            this,
    //            SLOT(CategoryButtonClicked()));
    //    connect(ui->recordPageButton,
    //            SIGNAL(clicked()),
    //            this,
    //            SLOT(CategoryButtonClicked()));
    connect(ui->recordPageButton,
            SIGNAL(clicked()),
            this,
            SLOT(RecordButtonClicked()));
    connect(ui->returnPageButton,
            SIGNAL(clicked()),
            this,
            SLOT(ReturnButtonClicked()));

    connect(ICProgramHeadFrame::Instance(),
            SIGNAL(LevelChanged(int)),
            this,
            SLOT(LevelChanged(int)));
    connect(ICVirtualHost::GlobalVirtualHost(),
            SIGNAL(StepChanged(int)),
            this,
            SLOT(StepChanged(int)));
}

void MainFrame::UpdateTranslate()
{
    ui->functionPageButton->setText(tr("Function"));
    ui->monitorPageButton->setText(tr("Monitor"));
    ui->alarmPageButton->setText(tr("Alarm"));
    ui->recordPageButton->setText(tr("Record"));
    ui->returnPageButton-> setText(tr("Return"));
    ui->cycleTimeAndFinistWidget->SetAlarmInfo(ICAlarmString::Instance()->AlarmInfo(errCode_));
    ui->cycleTimeAndFinistWidget->SetCycleTime(QString().sprintf("%.1f", cycleTime_ / 200.0));
    ui->cycleTimeAndFinistWidget->SetFinished(oldFinishCount_);
    ui->xPosLabel->setText(QString().sprintf("%.2f", oldXPos_ / 100.0));
    ui->yPosLabel->setText(QString().sprintf("%.2f", oldYPos_ / 100.0));
    ui->zPosLabel->setText(QString().sprintf("%.2f", oldZPos_ / 100.0));
    ui->pPosLabel->setText(QString().sprintf("%.2f", oldX2Pos_ / 100.0));
    ui->qPosLabel->setText(QString().sprintf("%.2f", oldY2Pos_ / 100.0));
    ui->aPosLabel->setText(QString().sprintf("%.2f", oldAPos_ / 100.0));
    ui->bPosLabel->setText(QString().sprintf("%.2f", oldBPos_ / 100.0));
    ui->cPosLabel->setText(QString().sprintf("%.2f", oldCPos_ / 100.0));
    ui->stepLabel->setText(QString::number(oldStep_));
}

void MainFrame::CategoryButtonClicked()
{
    QAbstractButton *clickedButton = qobject_cast<QAbstractButton *>(sender());

    if(functionButtonToPage_.contains(clickedButton))
    {
        centerStackedLayout_->setCurrentWidget(functionButtonToPage_.value(clickedButton));
    }
    //    else if(clickedButton == ui->monitorPageButton)
    //    {
    //        monitorPage_ = new ICMonitorPageFrame();
    //        functionButtonToPage_.insert(ui->monitorPageButton, monitorPage_);
    //        centerStackedLayout_->addWidget(monitorPage_);
    //    }
    //    ICProgramHeadFrame::Instance()->SetCurrentCategoryName(clickedButton->text());
}

void MainFrame::StatusRefreshed()
{

    static ICAlarmString* alarmString = ICAlarmString::Instance();
    static ICVirtualHost* virtualHost = ICVirtualHost::GlobalVirtualHost();
    //    if(isXPosChanged_)
    //    {
    //        ui->xPosLabel->setStyleSheet("color: rgb(0, 0, 127);");
    //        isXPosChanged_ = false;
    //    }
    //    if(isYPosChanged_)
    //    {
    //        ui->yPosLabel->setStyleSheet("color: rgb(0, 0, 127);");
    //        isYPosChanged_ = false;
    //    }
    //    if(isZPosChanged_)
    //    {
    //        ui->zPosLabel->setStyleSheet("color: rgb(0, 0, 127);");
    //        isZPosChanged_ = false;
    //    }
    //    if(isX2PosChanged_)
    //    {
    //        ui->pPosLabel->setStyleSheet("color: rgb(0, 0, 127);");
    //        isX2PosChanged_ = false;
    //    }
    //    if(isY2PosChanged_)
    //    {
    //        ui->qPosLabel->setStyleSheet("color: rgb(0, 0, 127);");
    //        isY2PosChanged_ = false;
    //    }
    //    if(isAPosChanged_)
    //    {
    //        ui->aPosLabel->setStyleSheet("color: rgb(0, 0, 127);");
    //        isAPosChanged_ = false;
    //    }
    //    if(isBPosChanged_)
    //    {
    //        ui->bPosLabel->setStyleSheet("color: rgb(0, 0, 127);");
    //        isBPosChanged_ = false;
    //    }
    //    if(isCPosChanged_)
    //    {
    //        ui->cPosLabel->setStyleSheet("color: rgb(0, 0, 127);");
    //        isCPosChanged_ = false;
    //    }
    uint axisLast = virtualHost->HostStatus(ICVirtualHost::AxisLastPos1).toUInt() |
            (virtualHost->HostStatus(ICVirtualHost::AxisLastPos2).toUInt() << 16);
//    int pos = virtualHost->HostStatus(ICVirtualHost::XPos).toInt() * 10 + (axisLast1 & 0xF);
    int pos = virtualHost->GetActualPos(ICVirtualHost::ICAxis_AxisX1, axisLast);
    if(pos != oldXPos_)
    {
        oldXPos_ = pos ;
        ui->xPosLabel->setText(QString().sprintf("%.2f", oldXPos_ / 100.0));
        ui->xPosLabel->setStyleSheet("color: rgb(0, 0, 127);background-color: rgb(85, 255, 127);");
        isXPosChanged_ = true;
    }
    pos = virtualHost->GetActualPos(ICVirtualHost::ICAxis_AxisY1, axisLast);
    if(pos != oldYPos_)
    {
        oldYPos_ = pos;
        ui->yPosLabel->setText(QString().sprintf("%.2f", oldYPos_ / 100.0));
        ui->yPosLabel->setStyleSheet("color: rgb(0, 0, 127);background-color: rgb(85, 255, 127);");
        isYPosChanged_ = true;
    }

    pos = virtualHost->GetActualPos(ICVirtualHost::ICAxis_AxisZ, axisLast);
    if(pos != oldZPos_)
    {
        oldZPos_ = pos;
        ui->zPosLabel->setText(QString().sprintf("%.2f", oldZPos_ / 100.0));
        ui->zPosLabel->setStyleSheet("color: rgb(0, 0, 127);background-color: rgb(85, 255, 127);");
        isZPosChanged_ = true;
    }

    pos = virtualHost->GetActualPos(ICVirtualHost::ICAxis_AxisX2, axisLast);
    if(pos != oldX2Pos_)
    {
        oldX2Pos_ = pos;
        ui->pPosLabel->setText(QString().sprintf("%.2f", oldX2Pos_ / 100.0));
        ui->pPosLabel->setStyleSheet("color: rgb(0, 0, 127);background-color: rgb(85, 255, 127);");
        isX2PosChanged_ = true;
    }

    pos = virtualHost->GetActualPos(ICVirtualHost::ICAxis_AxisY2, axisLast);
    if(pos != oldY2Pos_)
    {
        oldY2Pos_ = pos;
        ui->qPosLabel->setText(QString().sprintf("%.2f", oldY2Pos_ / 100.0));
        ui->qPosLabel->setStyleSheet("color: rgb(0, 0, 127);background-color: rgb(85, 255, 127);");
        isY2PosChanged_ = true;
    }

    pos = virtualHost->GetActualPos(ICVirtualHost::ICAxis_AxisA, axisLast);
    if(pos != oldAPos_)
    {
        oldAPos_ = pos;
        ui->aPosLabel->setText(QString().sprintf("%.2f", oldAPos_ / 100.0));
        ui->aPosLabel->setStyleSheet("color: rgb(0, 0, 127);background-color: rgb(85, 255, 127);");
        isAPosChanged_ = true;
    }

    pos = virtualHost->GetActualPos(ICVirtualHost::ICAxis_AxisB, axisLast);
    if(pos != oldBPos_)
    {
        oldBPos_ = pos;
        ui->bPosLabel->setText(QString().sprintf("%.2f", oldBPos_ / 100.0));
        ui->bPosLabel->setStyleSheet("color: rgb(0, 0, 127);background-color: rgb(85, 255, 127);");
        isBPosChanged_ = true;
    }

    pos = virtualHost->GetActualPos(ICVirtualHost::ICAxis_AxisC, axisLast);
    if(pos != oldCPos_)
    {
        oldCPos_ = pos;
        ui->cPosLabel->setText(QString().sprintf("%.2f", oldCPos_ / 100.0));
        ui->cPosLabel->setStyleSheet("color: rgb(0, 0, 127);background-color: rgb(85, 255, 127);");
        isCPosChanged_ = true;
    }

    newLedFlags_ = 0;
    newLedFlags_ |= (virtualHost->IsInputOn(35)? 8 : 0);
    newLedFlags_ |= (virtualHost->IsInputOn(32)? 4 : 0);
    newLedFlags_ |= (virtualHost->IsOutputOn(32)? 2 : 0);
    newLedFlags_ |= (virtualHost->IsOutputOn(33)? 1 : 0);
    if(newLedFlags_ != ledFlags_)
    {
        ledFlags_ = newLedFlags_;

#ifndef Q_WS_WIN32
#ifdef HC_ARMV6
        ioctl(ledFD_, 0, ledFlags_);
#else
        ioctl(ledFD_, 2, ledFlags_);
#endif
#endif
    }
    errCode_ = virtualHost->AlarmNum();
    if(compareAlarmNums_.indexOf(errCode_) != -1)
    {
        hostCompareDialog_->move(100, 100);
        hostCompareDialog_->show();
    }
    int hintCode = virtualHost->HintNum();
    if(alarmString->PriorAlarmNum() != static_cast<int>(errCode_) || hintCode != oldHintCode_)
    {
        oldHintCode_ = hintCode;
        qDebug()<<"hint code"<<hintCode<<alarmString->HintInfo(hintCode);
        alarmString->SetPriorAlarmNum(errCode_);
        if(errCode_ != 0)
        {
            ui->cycleTimeAndFinistWidget->SetAlarmInfo("Err" + QString::number(errCode_) + ":" + alarmString->AlarmInfo(errCode_));
            QTimer::singleShot(5000, this, SLOT(checkAlarmModify()));
        }
        else if(hintCode != 0)
        {
            ui->cycleTimeAndFinistWidget->SetHintInfo(tr("Hint") + QString::number(hintCode) + ":" + alarmString->HintInfo(hintCode));
        }
        else
        {
            ui->cycleTimeAndFinistWidget->SetAlarmInfo("");
        }
    }
    finishCount_ = virtualHost->HostStatus(ICVirtualHost::DbgX1).toUInt();
    if(finishCount_ != oldFinishCount_)
    {
        ui->cycleTimeAndFinistWidget->SetFinished(virtualHost->HostStatus(ICVirtualHost::DbgX1).toUInt());
        oldFinishCount_ = finishCount_;
    }
    cycleTime_ = virtualHost->HostStatus(ICVirtualHost::Time).toUInt();
    if(cycleTime_ != oldCycleTime_)
    {
        ui->cycleTimeAndFinistWidget->SetCycleTime(QString().sprintf("%.2f", cycleTime_ / qreal(100)));
        oldCycleTime_ = cycleTime_;
    }

    bool getHostOrigin = IsOrigined();
    if(getHostOrigin != isOrigined_)
    {
        isOrigined_ = getHostOrigin;
        getHostOrigin ? ui->systemStatusFrame->SetOriginStatus(StatusLabel::ONSTATUS) :
                        ui->systemStatusFrame->SetOriginStatus(StatusLabel::OFFSTATUS);
    }
    runningStatus_ = virtualHost->CurrentStatus();

    if(runningStatus_ == ICVirtualHost::Manual)
    {
        speed_ = virtualHost->HostStatus(ICVirtualHost::DbgX0).toString();
        //        statusStr_ = tr("Manual");
    }
    else if(runningStatus_ == ICVirtualHost::Stop)
    {
        actionDialog_->hide();
        speed_ = "0";
        //        statusStr_ = tr("Stop");
#ifdef Q_WS_X11
        finishCount_ = virtualHost->FinishProductCount();
        if(finishCount_ != oldFinishCount_)
        {
            ui->cycleTimeAndFinistWidget->SetFinished(finishCount_);
            oldFinishCount_ = finishCount_;
        }
#endif
        ui->systemStatusFrame->SetProgramStatus(StatusLabel::ONSTATUS);
    }
    else if(runningStatus_ == ICVirtualHost::Auto)
    {
        if(hintCode == 15)
        {
            if(actionDialog_->isHidden())
            {
                actionDialog_->show();
            }
        }
        else
        {
            actionDialog_->hide();
        }
        finishCount_ = virtualHost->HostStatus(ICVirtualHost::DbgX1).toUInt();
        if(finishCount_ != oldFinishCount_)
        {
            ui->cycleTimeAndFinistWidget->SetFinished(finishCount_);
            virtualHost->SetFinishProductCount(finishCount_);
            oldFinishCount_ = finishCount_;
        }
        int speedVal =  virtualHost->GlobalSpeed();
        speed_ = QString::number(speedVal);
        if(virtualHost->HostStatus(ICVirtualHost::DbgX0) == ICVirtualHost::AutoReady)
        {
            runningStatus_ = ICVirtualHost::AutoReady;
        }
        else if(virtualHost->HostStatus(ICVirtualHost::DbgX0) == ICVirtualHost::AutoRunning)
        {
            runningStatus_ = ICVirtualHost::AutoRunning;
        }
        else if(virtualHost->HostStatus(ICVirtualHost::DbgX0) == ICVirtualHost::AutoSingleCycle)
        {
            runningStatus_ = ICVirtualHost::AutoSingleCycle;
        }
        else if(virtualHost->HostStatus(ICVirtualHost::DbgX0) == ICVirtualHost::AutoStopping)
        {
            runningStatus_ = ICVirtualHost::AutoStopping;
        }
        else if(virtualHost->HostStatus(ICVirtualHost::DbgX0) == ICVirtualHost::AutoOneCycle)
        {
            runningStatus_ = ICVirtualHost::AutoOneCycle;
        }
        //        statusStr_ = tr("Auto");
    }
    else if(runningStatus_ == ICVirtualHost::Teach)
    {
        speed_ = virtualHost->HostStatus(ICVirtualHost::DbgZ1).toString();
        //        statusStr_ = tr("Teach");
    }
    else
    {
        speed_ = "0";
    }
    //    else if(runningStatus_ == ICVirtualHost::Origin)
    //    {
    //        speed_ = virtualHost->HostStatus(ICVirtualHost::S).toString();
    //    }
    //    else if(runningStatus_ == ICVirtualHost::Return)
    //    {
    //        speed_ = virtualHost->HostStatus(ICVirtualHost::S).toString();
    //    }
    if(speed_ != ui->systemStatusFrame->CurrentSpeed())
    {
        //        ui->speedLabel->setText(speed_);
        ui->systemStatusFrame->SetCurrentSpeed(speed_);
    }
    if(runningStatus_ != oldRunnigStatus_)
    {
        //        ui->statusLabel->setText(statusStr_);
        oldRunnigStatus_ = runningStatus_; // must put here because the stop status want to use the new value;
        if(runningStatus_ == ICVirtualHost::Manual)
        {
            ui->systemStatusFrame->SetManualStatus(StatusLabel::ONSTATUS);
            //            LevelChanged(ICProgramHeadFrame::Instance()->CurrentLevel());
            //            ui->functionPageButton->setEnabled(false);
            //            ui->recordPageButton->setText(tr("Instru            ui->recordPageButton->setText(tr("Instruct"));ct"));
            //            ui->recordPageButton->setEnabled(false);
        }
        else if(runningStatus_ == ICVirtualHost::AutoRunning)
        {
            ui->systemStatusFrame->SetAutoStatus(ICSystemStatusFrame::Running);
            //            ui->functionPageButton->setEnabled(false);
            //            ui->recordPageButton->setEnabled(false);
        }
        else if(runningStatus_ == ICVirtualHost::AutoReady)
        {
            ui->systemStatusFrame->SetAutoStatus(ICSystemStatusFrame::Ready);
            //            ui->functionPageButton->setEnabled(false);
            //            ui->recordPageButton->setEnabled(false);
        }
        else if(runningStatus_ == ICVirtualHost::AutoSingleCycle)
        {
            ui->systemStatusFrame->SetAutoStatus(ICSystemStatusFrame::SingleCycle);
            //            ui->functionPageButton->setEnabled(false);
            //            ui->recordPageButton->setEnabled(false);
        }
        else if(runningStatus_ == ICVirtualHost::AutoStopping)
        {
            ui->systemStatusFrame->SetAutoStatus(ICSystemStatusFrame::Stopping);
            //            ui->functionPageButton->setEnabled(false);
            //            ui->recordPageButton->setEnabled(false);
        }
        else if(runningStatus_ == ICVirtualHost::AutoOneCycle)
        {
            ui->systemStatusFrame->SetAutoStatus(ICSystemStatusFrame::OneCycle);
        }
        //        else if(runningStatus_ == ICVirtualHost::Teach)
        //        {
        //            ui->systemStatusFrame->SetProgramStatus(StatusLabel::ONSTATUS);
        //        }
        else if(runningStatus_ == ICVirtualHost::Stop)
        {
            //            ui->systemStatusFrame->SetProgramStatus(virtualHost->IsCloseMoldPermit() ? StatusLabel::ONSTATUS : StatusLabel::OFFSTATUS);
            //            ui->systemStatusFrame->SetSystemStop();
            //            ui->recordPageButton->setText(tr("Records"));
            //            LevelChanged(ICProgramHeadFrame::Instance()->CurrentLevel());
            //            ui->functionPageButton->setEnabled(true);
            //            ui->recordPageButton->setEnabled(true);
            //            HideOrigin();
            HideReturn();
        }
        else if(runningStatus_ == ICVirtualHost::Origin)
        {
            ShowOrigin();
        }
        else if(runningStatus_ == ICVirtualHost::Return)
        {
            ShowReturn();
        }
    }
    LevelChanged(ICProgramHeadFrame::Instance()->CurrentLevel());
    if(mousePoint_ != QCursor::pos())
    {
        mousePoint_ = QCursor::pos();
        SetHasInput(true);
    }
}

void MainFrame::ShowManualPage()
{
    functionPage_->ShowFunctionSelectPage();
    centerStackedLayout_->setCurrentWidget(manualPage_);
    //    ICProgramHeadFrame::Instance()->SetCurrentCategoryName(tr("Manual"));
    ICProgramHeadFrame::Instance()->StopAutoTime();
    nullButton_->click();
    //    if(!IsOrigined())
    //    {
    //        QMessageBox::warning(this, tr("Warning"), tr("Need to origin!"));
    //    }
    ui->recordPageButton->setText(tr("Instruct"));
    //    ui->recordPageButton->setEnabled(true);
}

void MainFrame::ShowAutoPage()
{
    resetTime = ICParametersSave::Instance()->RestTime(0);

    if(resetTime <= 7*24 )
    {
        if(resetTime > 0)
        {
            QMessageBox::information(this,tr("tips"),tr("Spear Time %1 Hour").arg(resetTime));
        }
        else if(resetTime < 0)
        {
            QMessageBox::information(this,tr("tips"),tr("No Register"));
        }
    }
    functionPage_->ShowFunctionSelectPage();
    centerStackedLayout_->setCurrentWidget(autoPage_);
    //    ICProgramHeadFrame::Instance()->SetCurrentCategoryName(tr("Auto")) ;
    ICProgramHeadFrame::Instance()->StartAutoTime();
    nullButton_->click();
    //    if(!IsOrigined())
    //    {
    //        QMessageBox::warning(this, tr("Warning"), tr("Need to origin!"));
    //    }
}

void MainFrame::ShowInstructPage()
{
    centerStackedLayout_->setCurrentWidget(instructPage_);
    //    ICProgramHeadFrame::Instance()->SetCurrentCategoryName(tr("Instruct"));
    nullButton_->click();
}

void MainFrame::ShowStandbyPage()
{
    if(manualPage_ != NULL)
    {
        manualPage_->AdjustFrameTransfer();
    }
    functionPage_->ShowFunctionSelectPage();
    centerStackedLayout_->setCurrentWidget(initialPage_);
    //    ICProgramHeadFrame::Instance()->SetCurrentCategoryName(tr("Standby"));
    ICProgramHeadFrame::Instance()->StopAutoTime();
    nullButton_->click();
    ui->recordPageButton->setText(tr("Records"));
    //    qApp->processEvents();
}

void MainFrame::ShowFunctionPage()
{
    centerStackedLayout_->setCurrentWidget(functionPage_);
    //    ICProgramHeadFrame::Instance()->SetCurrentCategoryName(ui->functionPageButton->text());
}

void MainFrame::ShowOrigin()
{
    if(!originExecutingPage_->isVisible())
    {
        //        ui->systemStatusFrame->SetOriginStatus(StatusLabel::ONSTATUS);
        //        isOriginShown_ = true;
        originExecutingPage_->open();
    }
}

void MainFrame::HideOrigin()
{
    //    if(isOriginShown_)
    //    {
    //        ui->systemStatusFrame->SetSystemStop();
    //        isOriginShown_ = false;
    if(originExecutingPage_->isVisible())
    {
        originExecutingPage_->accept();
        //        oldRunnigStatus_ = ICVirtualHost::Stop;
    }
}

void MainFrame::ShowReturn()
{
    if(!isReturnShown_)
    {
        isReturnShown_ = true;
        returnExecutingPage_->open();
    }
}

void MainFrame::HideReturn()
{
    if(isReturnShown_)
    {
        isReturnShown_ = false;
        returnExecutingPage_->accept();
    }
}

void MainFrame::ReturnButtonClicked()
{
    int status = ICKeyboard::Instace()->CurrentSwitchStatus();
    if(status == ICKeyboard::KS_ManualStatu)
    {
        ShowManualPage();
    }
    else if(status == ICKeyboard::KS_AutoStatu)
    {
        if(centerStackedLayout_->currentWidget() != autoPage_)
        {
            ShowAutoPage();
        }
    }
    else
    {
        ShowStandbyPage();
    }

}

void MainFrame::RecordButtonClicked()
{
    //    int status = ICVirtualHost::GlobalVirtualHost()->CurrentStatus();
#if !defined(Q_WS_WIN32) &&  !defined(Q_WS_X11)
    if(ICKeyboard::Instace()->CurrentSwitchStatus() == ICKeyboard::KS_ManualStatu)
#else
    if(!manualPage_->isHidden())
#endif
    {
        centerStackedLayout_->setCurrentWidget(instructPage_);
    }
    else
    {
        centerStackedLayout_->setCurrentWidget(recordPage_);
    }
}

void MainFrame::LevelChanged(int level)
{

    switch(level)
    {
    case ICParametersSave::MachineOperator:
    {
        ui->functionPageButton->setEnabled(false);
        ui->recordPageButton->setEnabled(false);
#ifndef Q_WS_X11
        if(!functionPage_->isHidden() ||
                !instructPage_->isHidden() ||
                !recordPage_->isHidden())
        {
            ui->returnPageButton->click();
        }
#endif
    }
        break;
    case ICParametersSave::MachineAdmin:
    case ICParametersSave::AdvanceAdmin:
    {
        if(ICKeyboard::Instace()->CurrentSwitchStatus() == ICKeyboard::KS_StopStatu)
        {
            ui->functionPageButton->setEnabled(true);
        }
        else
        {
            ui->functionPageButton->setEnabled(false);
        }
        if(ICKeyboard::Instace()->CurrentSwitchStatus() != ICKeyboard::KS_AutoStatu)
        {
            ui->recordPageButton->setEnabled(true);
        }
        else
        {
            ui->recordPageButton->setEnabled(false);
        }
    }
        break;
    default:
    {
        ui->functionPageButton->setEnabled(false);
        ui->recordPageButton->setEnabled(false);
    }
    }
}
void MainFrame::StepChanged(int step)
{
    ui->stepLabel->setText(QString::number(step));
    oldStep_ = step;
}


bool MainFrame::IsOrigined() const
{
    return ICVirtualHost::GlobalVirtualHost()->IsOrigined();
}

void MainFrame::ShowScreenSaver()
{
    //    screenSaver_->show();
    ICProgramHeadFrame::Instance()->SetCurrentLevel(ICParametersSave::MachineOperator);
}

bool MainFrame::IsInput() const
{
    return isDoAction_;
}

void MainFrame::SetHasInput(bool isInput)
{
    isDoAction_ = isInput;
    if(isInput && IsBackLightOff())
    {
        //        system("BackLight on");
        ICParametersSave::Instance()->SetBrightness(ICParametersSave::Instance()->Brightness());
        SetBackLightOff(false);
    }
}

bool MainFrame::IsBackLightOff() const
{
    return isBackLightOff_;
}

void MainFrame::SetBackLightOff(bool isOff)
{
    isBackLightOff_ = isOff;
}

void MainFrame::CheckedInput()
{
    if(!IsInput())
    {
        ShowScreenSaver();
        system("BackLight off");
        SetBackLightOff(true);
    }
    SetHasInput(false);
    QTimer::singleShot(ICParametersSave::Instance()->BackLightTime() * 60000, this, SLOT(CheckedInput()));
}

void MainFrame::ShowWidgets_(QList<QWidget *> &widgets)
{
    for(int i = 0; i != widgets.size(); ++i)
    {
        widgets[i]->show();
    }
}

void MainFrame::HideWidgets_(QList<QWidget *> &widgets)
{
    for(int i = 0; i != widgets.size(); ++i)
    {
        widgets[i]->hide();
    }
}

void MainFrame::UpdateAxisDefine_()
{
    ICVirtualHost* host = ICVirtualHost::GlobalVirtualHost();
    int currentAxis = host->SystemParameter(ICVirtualHost::SYS_Config_Arm).toInt();
    if(axisDefine_ != currentAxis)
    {
        axisDefine_ = currentAxis;
        for(int i = 0 ; i != axisWidgets_.size(); ++i)
        {
            HideWidgets_(axisWidgets_[i]);
        }


        if(host->AxisDefine(ICVirtualHost::ICAxis_AxisX1) != ICVirtualHost::ICAxisDefine_Servo)
        {
            HideWidgets_(axisWidgets_[0]);
        }
        else
        {
            ShowWidgets_(axisWidgets_[0]);
        }

        if(host->AxisDefine(ICVirtualHost::ICAxis_AxisY1) != ICVirtualHost::ICAxisDefine_Servo)
        {
            HideWidgets_(axisWidgets_[1]);
        }
        else
        {
            ShowWidgets_(axisWidgets_[1]);
        }

        if(host->AxisDefine(ICVirtualHost::ICAxis_AxisZ) != ICVirtualHost::ICAxisDefine_Servo)
        {
            HideWidgets_(axisWidgets_[2]);
        }
        else
        {
            ShowWidgets_(axisWidgets_[2]);
        }

        if(host->AxisDefine(ICVirtualHost::ICAxis_AxisX2) != ICVirtualHost::ICAxisDefine_Servo)
        {
            HideWidgets_(axisWidgets_[3]);
        }
        else
        {
            ShowWidgets_(axisWidgets_[3]);
        }

        if(host->AxisDefine(ICVirtualHost::ICAxis_AxisY2) != ICVirtualHost::ICAxisDefine_Servo)
        {
            HideWidgets_(axisWidgets_[4]);
        }
        else
        {
            ShowWidgets_(axisWidgets_[4]);
        }

        if(host->AxisDefine(ICVirtualHost::ICAxis_AxisA) != ICVirtualHost::ICAxisDefine_Servo)
        {
            HideWidgets_(axisWidgets_[5]);
        }
        else
        {
            ShowWidgets_(axisWidgets_[5]);
        }

        if(host->AxisDefine(ICVirtualHost::ICAxis_AxisB) != ICVirtualHost::ICAxisDefine_Servo)
        {
            HideWidgets_(axisWidgets_[6]);
        }
        else
        {
            ShowWidgets_(axisWidgets_[6]);
        }

        if(host->AxisDefine(ICVirtualHost::ICAxis_AxisC) != ICVirtualHost::ICAxisDefine_Servo)
        {
            HideWidgets_(axisWidgets_[7]);
        }
        else
        {
            ShowWidgets_(axisWidgets_[7]);
        }
    }
}

void MainFrame::KeyToInstructEditor(int key)
{
    Q_UNUSED(key);
    instructPage_->ShowServoAction(key);
}

void MainFrame::ClearPosColor()
{
    if(isXPosChanged_)
    {
        ui->xPosLabel->setStyleSheet("color: rgb(0, 0, 127);");
        isXPosChanged_ = false;
    }
    if(isYPosChanged_)
    {
        ui->yPosLabel->setStyleSheet("color: rgb(0, 0, 127);");
        isYPosChanged_ = false;
    }
    if(isZPosChanged_)
    {
        ui->zPosLabel->setStyleSheet("color: rgb(0, 0, 127);");
        isZPosChanged_ = false;
    }
    if(isX2PosChanged_)
    {
        ui->pPosLabel->setStyleSheet("color: rgb(0, 0, 127);");
        isX2PosChanged_ = false;
    }
    if(isY2PosChanged_)
    {
        ui->qPosLabel->setStyleSheet("color: rgb(0, 0, 127);");
        isY2PosChanged_ = false;
    }
    if(isAPosChanged_)
    {
        ui->aPosLabel->setStyleSheet("color: rgb(0, 0, 127);");
        isAPosChanged_ = false;
    }
    if(isBPosChanged_)
    {
        ui->bPosLabel->setStyleSheet("color: rgb(0, 0, 127);");
        isBPosChanged_ = false;
    }
    if(isCPosChanged_)
    {
        ui->cPosLabel->setStyleSheet("color: rgb(0, 0, 127);");
        isCPosChanged_ = false;
    }
    QTimer::singleShot(1000, this, SLOT(ClearPosColor()));
}

void MainFrame::Register()
{
    resetTime = ICParametersSave::Instance()->RestTime(0);
    if(resetTime < 0)
    {
#ifndef Q_WS_X11
        QMessageBox::information(NULL,tr("tips"),tr("No Register. System Restart Now..."));
//        system("reboot");
#endif
    }
}

void MainFrame::CountRestTime()
{
    resetTime-- ;
    if(resetTime == 0)
    {
        resetTime = -1;
        ICParametersSave::Instance()->SetRestTime(resetTime);
        Register();
        registe_timer->stop();
        reboot_timer->start(1000*60*10);
    }
    ICParametersSave::Instance()->SetRestTime(resetTime);
}

void MainFrame::checkAlarmModify()
{
    if(errCode_ == 0)
    {
        ICAlarmFrame::Instance()->AlarmModifyTime();
    }
    else
    {
        QTimer::singleShot(5000, this, SLOT(checkAlarmModify()));
    }
}

void MainFrame::InitSpareTime()
{
    registe_timer->stop();
    reboot_timer->stop();
    resetTime = ICParametersSave::Instance()->RestTime(0);
    if(resetTime <= 7*24 )
    {
        if(resetTime > 0)
        {
            QMessageBox::information(NULL,tr("tips"),tr("Spare Time %1 Hour").arg(resetTime));
//            connect(registe_timer,SIGNAL(timeout()),this,SLOT(CountRestTime()));
//            registe_timer->start(1000*15);
            registe_timer->start(1000*3600);
        }
        else if(resetTime < 0)
        {
            QMessageBox::information(NULL,tr("tips"),tr("No Register,The System Will Reboot after 10 minutes"));
//            connect(reboot_timer,SIGNAL(timeout()),this,SLOT(Register()));
            //            registe_timer->start(1000*60*10);
            reboot_timer->start(1000*60*10);

        }
    }
    else
    {
//        connect(registe_timer,SIGNAL(timeout()),this,SLOT(CountRestTime()));
//        registe_timer->start(1000*15); //15秒减一次（方便测试）
        registe_timer->start(1000*3600);
    }

}
