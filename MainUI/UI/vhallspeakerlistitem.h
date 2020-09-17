#ifndef VHALLSPEAKERLISTITEM_H
#define VHALLSPEAKERLISTITEM_H

#include <QWidget>

namespace Ui {
class VhallSpeakerListItem;
}

class VhallSpeakerListItem : public QWidget
{
    Q_OBJECT

public:
    explicit VhallSpeakerListItem(QWidget *parent = 0);
    ~VhallSpeakerListItem();
    void setId(QString id);
    void setName(QString name);
    void setMaster(bool master);
    QString GetId() {return mId;}
    void setCameraIcon(bool ok);
    void setMicIcon(bool ok);
    bool GetMicState();
    void SetOffVisiable(bool);
signals:
    void SigItemClicked(QString);
    
private:
    void RepaintIcon();
    QString geteElidedText(QFont font, QString str, int MaxWidth) ;
protected:
    bool eventFilter(QObject *o,QEvent *e);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
private:
    Ui::VhallSpeakerListItem *ui;
    QString mId;
    QString mName;
    bool mMaster = false;
    bool mCamera;
    bool mMic;
    bool mbVisable;
};

#endif // VHALLSPEAKERLISTITEM_H
