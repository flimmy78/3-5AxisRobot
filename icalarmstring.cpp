#include "icalarmstring.h"

#include <QTextStream>
#include <QStringList>

#include "icparameterssave.h"

#include <QDebug>

ICAlarmString * ICAlarmString::instance_ = NULL;

ICAlarmString::ICAlarmString()
    : AlarmChineseInfoPathName(":/sysconfig/alarminfomation-ch"),
      AlarmEnglishInfoPathName(":/sysconfig/alarminfomation-en"),
      AlarmSpanishInfoPathName(":/sysconfig/alarminfomation-esp"),
      AlarmNederlandInfoPathName(":/sysconfig/alarminfomation-nl"),
      AlarmFranceInfoPathName(":/sysconfig/alarminfomation-fr"),
      AlarmKoreaInfoPathName(":/sysconfig/alarminfomation-kr"),
      HintChInfoPathName(":/sysconfig/hintinfomation-ch"),
      HintEnInfoPathName(":/sysconfig/hintinfomation-en"),
      HintEspInfoPathName(":/sysconfig/hintinfomation-esp"),
      HintNederlandInfoPathName(":/sysconfig/hintinfomation-nl"),
      HintFranceInfoPathName(":/sysconfig/hintinfomation-fr"),
      HintKoreaInfoPathName(":/sysconfig/hintinfomation-kr"),
      priorAlarmNum_(-1)
{
    //    OnCurrentLanguageChanged(ICParameterSaves::Instance()->Language());
    OnCurrentLanguageChanged();

    connect(ICParametersSave::Instance(),
            SIGNAL(CurrentLanguageChanged()),
            this,
            SLOT(OnCurrentLanguageChanged()));
}

void ICAlarmString::OnCurrentLanguageChanged()
{
    QFile hintFile;
    switch(ICParametersSave::Instance()->Language())
    {
    case QLocale::Chinese:
        alarmChInfoFile.setFileName(AlarmChineseInfoPathName);
        hintFile.setFileName(HintChInfoPathName);
        break;
    case QLocale::English:
        alarmChInfoFile.setFileName(AlarmEnglishInfoPathName);
        hintFile.setFileName(HintEnInfoPathName);
        break;
    case QLocale::Spanish:
        alarmChInfoFile.setFileName(AlarmSpanishInfoPathName);
        hintFile.setFileName(HintEspInfoPathName);
        break;
    case QLocale::Dutch:
        alarmChInfoFile.setFileName(AlarmNederlandInfoPathName);
        hintFile.setFileName(HintNederlandInfoPathName);
        break;
    case QLocale::French:
        alarmChInfoFile.setFileName(AlarmFranceInfoPathName);
        hintFile.setFileName(HintFranceInfoPathName);
        break;
    case QLocale::Korean:
        alarmChInfoFile.setFileName(AlarmKoreaInfoPathName);
        hintFile.setFileName(HintKoreaInfoPathName);
        break;
    default:
        return;
    }

    alarmInfoMap_.clear();

    QTextStream alarmStream(&alarmChInfoFile);
    alarmStream.setCodec("UTF-8");

    QString alarms;
    if(alarmChInfoFile.open(QIODevice::ReadOnly))
    {
        alarms = alarmStream.readAll();
    }
    alarmChInfoFile.close();

    QStringList alarmsList = alarms.split('\n', QString::SkipEmptyParts);
    QStringList alarmInfos;
    foreach(const QString &alarm, alarmsList)
    {
        alarmInfos = alarm.split(QChar(':'));
        if(alarmInfos.count() == 2)
        {
            alarmInfoMap_.insert(alarmInfos.at(0).toInt(), alarmInfos.at(1));
        }
    }
    if(hintFile.open(QIODevice::ReadOnly))
    {
        alarmStream.setDevice(&hintFile);
        alarmStream.setCodec("UTF-8");
        alarms = alarmStream.readAll();
    }
    hintFile.close();
    alarmsList = alarms.split('\n', QString::SkipEmptyParts);
    for(int i = 0; i != alarmsList.size(); ++i)
    {
        alarmInfos = alarmsList.at(i).split(QChar(':'));
        if(alarmInfos.count() == 2)
        {
            hintInfoMap_.insert(alarmInfos.at(0).toInt(), alarmInfos.at(1));
        }
    }


    emit CurrentLanguageChanged();
}
