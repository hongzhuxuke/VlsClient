#pragma once
#include <QPushButton>
#include <QTimer>

class CTimeButton : public QPushButton
{
	Q_OBJECT

public:
	CTimeButton(QWidget *parent = Q_NULLPTR);
	~CTimeButton();

	void setHoverDisplayText(const QString& strText);
    long GetClassTime();

	void updateLiveStatus(bool liveStatus);
	bool GetLiveStatus();

	void InitStartTime(int initLiveTime);
	void Clear();
	//int GetDisplayDateTime();
	//void setObjName(const QString& name);
protected:
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
	//void paintEvent(QPaintEvent *event);
private slots:
	void slot_UpdateClassTime();
private:

	void startClassTime();
	void dispalyTime();

	int miHour;
	int miBranch;
	int miSecond;

	QTimer* mpClassTime = nullptr;//上课时间计时器

	long miClassingTime = 0;

	bool mbMouseHoverOn = false;
	QString mstrDisplayText = "";

	bool mIsStarting;
	//QString ObjName = "";
	int mLiveTime = 0;
   int mLiveTotalTime = 0;
   uint mLastLiveTime = 0;
};