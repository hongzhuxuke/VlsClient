#ifndef PLUGINDATAPROCESSOR_H
#define PLUGINDATAPROCESSOR_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QList>
#include <QTimer>

#define PLUGIN_DOCDATA_LIST   "doc_table"
#define PLUGIN_WBDATA_LIST   "wb_table"

class CPluginDataProcessor : public QObject
{
   Q_OBJECT

public:
    CPluginDataProcessor(QObject *parent = NULL);
    ~CPluginDataProcessor();
    /*
    * 清空数据队列，当关闭活动之后需要调用此接口。
    */
    void ClearAllData(); 
    void SetPushStreamState(bool bStart);
    bool HasPushStreamState();
    int OnRecvHttpData(const char* data, int dataLen);

	 void SetEnableRecvMsg(bool enable);
	 bool GetEnableRecvMsg();
private slots:
   void Slot_PushStream();
private:
   void PostPluginData(const char* data,int dataLen);

   void InsertDocData(QString page_id, const char* data, int dataLen);
   void InsertWBData(const char* data, int dataLen);

   void SendWBPluginData();
   void SendDocPluginData();
   bool isDocPluginData(const char* data, const int dataLen, QString &pageid);
   bool isWBPluginData(const char* data, const int dataLen);
private:
   QMutex m_mutexStream;
   bool m_bHasPushStream = false;

   QList<std::string*> m_WbDataList;
   QList<std::string*>  m_DocDataList;
   QString m_nCurrentPage;

   QTimer*  m_pPushTimer = NULL;

	QMutex m_enableMutex;
	bool mbIsEnableRecvMsg = true;
};

#endif // PLUGINDATAPROCESSOR_H
