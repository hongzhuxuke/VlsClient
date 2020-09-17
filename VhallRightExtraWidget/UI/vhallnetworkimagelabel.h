#ifndef VHALLNETWORKIMAGELABEL_H
#define VHALLNETWORKIMAGELABEL_H

#include <QWidget>

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QUrl>
#include <QPixmap>

namespace Ui {
class VhallNetworkImageLabel;
}

class VhallNetworkImageLabel : public QWidget
{
    Q_OBJECT

public:
    explicit VhallNetworkImageLabel(QWidget *parent = 0);
    ~VhallNetworkImageLabel();
    void SetImage(QString url);

protected:
   virtual void customEvent(QEvent*);
signals:
    void SigUserImageFinished(QPixmap &);

public slots:
    void finished(QNetworkReply *); 
private:
    Ui::VhallNetworkImageLabel *ui;
    QNetworkAccessManager *m_manager;    
    QPixmap m_userImage;
    QString mOutFile;
};

#endif // VHALLNETWORKIMAGELABEL_H
