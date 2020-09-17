#pragma once
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QObject>

class Event;

class MemberParam : public QObject
{
    Q_OBJECT

public:
    MemberParam(QObject *parent = nullptr);
    ~MemberParam();

    void ParamToMemberInfo(int libCurlCode, const QString& msg, int memberListType, int& outHttpCode,QString &outMsg, Event* memberInfo);
    void ParamToFlashMemberInfo(int libCurlCode, const QString& msg, int memberListType, int& outHttpCode, QString &outMsg, Event* memberInfo);
private:
    int ParamToDetailMember(int msgType,QJsonObject obj, Event* memberInfo);
};
