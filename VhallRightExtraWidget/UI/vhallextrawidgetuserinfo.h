#ifndef VHALLEXTRAWIDGETUSERINFO_H
#define VHALLEXTRAWIDGETUSERINFO_H

#include <QWidget>
#include <QPixmap>
#include "vhallnetworkimagelabel.h"

namespace Ui {
class VhallExtraWidgetUserInfo;
}

class VhallExtraWidgetUserInfo : public QWidget
{
    Q_OBJECT

public:
    explicit VhallExtraWidgetUserInfo(QWidget *parent = 0);
    ~VhallExtraWidgetUserInfo();
    //void SetUserName(const QString &);
    void SetUserImage(const QString &);
	 QString GetNickName();
private:
   QString geteElidedText(QFont font, QString str, int MaxWidth);
signals:
    //void SigSetUserName(const QString &);
    void SigSetUserImage(const QString &); 
    void SigUserImageFinished(QPixmap &);
public slots:
    void SlotSetUserImage(const QString &);
    void SlotSetUserName(const QString &) ;
private:
    Ui::VhallExtraWidgetUserInfo *ui;
    VhallNetworkImageLabel *m_ImageLabel = NULL;
};

#endif // VHALLEXTRAWIDGETUSERINFO_H
