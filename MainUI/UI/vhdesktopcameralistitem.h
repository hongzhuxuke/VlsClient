#ifndef VHDESKTOPCAMERALISTITEM_H
#define VHDESKTOPCAMERALISTITEM_H
#include "VH_ConstDeff.h"
#include <QWidget>

namespace Ui {
class VHDesktopCameraListItem;
}

class VHDesktopCameraListItem : public QWidget
{
    Q_OBJECT

public:
    explicit VHDesktopCameraListItem(QWidget *parent = 0);
    ~VHDesktopCameraListItem();
    void SetData(QString,DeviceInfo,int);
    bool SetChecked(DeviceInfo &dInfo);
    void SetChecked(bool bCheck);
    bool IsChecked();
    void SetEnabled(bool );
    void Toggle(bool);
    QString GetDeviceID();
    void SetToggle(bool b){m_bToToggle = b;}
	 DeviceInfo* GetDeviceInfo();
signals:
    void SigClicked(bool,DeviceInfo);
private slots:
    void checkBox_toggled(bool checked);
protected:
    void paintEvent(QPaintEvent *);
    bool eventFilter(QObject *, QEvent *);
private:
    Ui::VHDesktopCameraListItem *ui;
    QString txt ;
    DeviceInfo deviceInfo;
    int itemIndex;
    bool m_bToToggle = true;
};

#endif // VHDESKTOPCAMERALISTITEM_H
