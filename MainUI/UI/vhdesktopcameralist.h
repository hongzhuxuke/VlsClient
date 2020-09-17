#ifndef VHDESKTOPCAMERALIST_H
#define VHDESKTOPCAMERALIST_H

#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include "VH_ConstDeff.h"
namespace Ui {
class VHDesktopCameraList;
}

class VHDesktopCameraList : public QWidget
{
    Q_OBJECT

public:
    explicit VHDesktopCameraList(QWidget *parent = 0);
    ~VHDesktopCameraList();
    void SetTipsLabelText(QString);
    void SetMaxCount(int m ){maxCount = m;};
    void SetPixmap(QString);
    void SetPixmap(QPixmap);
    void AddItem(QString,DeviceInfo,int);
    void AddItemInterActivityItem(QString,DeviceInfo,int);
    void Clear();
    void Resize();
    void Show();
    void SetChecked(DeviceInfo &dInfo);
    void FlushEnableStatus();
    void RemoveDevice(QString deviceID);
    void SetBodyShow(int h,int b,int t);
    void SetBodySide(int l,int r);
    QString GetCheckedDeviceID();
public slots:
    void SlotItemClicked(bool,DeviceInfo);
signals:
    void SigItemClicked(bool,DeviceInfo);    
protected:
    void paintEvent(QPaintEvent *);
    void focusOutEvent(QFocusEvent *);
private:
    Ui::VHDesktopCameraList *ui;
    QPixmap pixmap;
    QPixmap pixmap_head;
    QPixmap pixmap_body;
    QPixmap pixmap_tail;
    QList<QWidget *> mCurrentWidget;

    int mBodyHead = 33;
    int mBodyShow = 30;
    int mBodyTail = 40;

    int maxCount = 2;    
    int checkedCount = 0;
    QLabel *tipsLabel = NULL;
};

#endif // VHDESKTOPCAMERALIST_H
