#include "vhallnetworkimagelabel.h"
#include "ui_vhallnetworkimagelabel.h"
#include <QPainter>
#include <QBitmap>
#include <QtNetwork/QNetworkAccessManager>
#include "VhallNetWorkInterface.h"
#include <QDir>
#include "VhallUI_define.h"
#include <QDebug>

VhallNetworkImageLabel::VhallNetworkImageLabel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VhallNetworkImageLabel),
    m_manager(NULL) {
   ui->setupUi(this);
   this->setMinimumSize(30,30);
   this->setMaximumSize(30,30);
}

VhallNetworkImageLabel::~VhallNetworkImageLabel() {
    delete ui;
}
void VhallNetworkImageLabel::SetImage(QString url) {
   int index = url.lastIndexOf("/");
   QString pic_name = url.mid(index, url.length());
   mOutFile = QDir::tempPath() + pic_name;
   QImage privew_image;
   bool bRet = privew_image.load(mOutFile);
   if (bRet) {
      QApplication::postEvent(this, new QEvent(CustomEvent_DownLoadSharePic));
   }
   else {
      HTTP_GET_REQUEST httpRequest(url.toStdString());
      httpRequest.SetHttpPost(true);
      httpRequest.SetEnableDownLoadFile(true, mOutFile.toStdString());
      httpRequest.mbIsNeedSyncWork = true;
      QObject* obj = this;
      GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, obj](const std::string& msg, int code, const std::string userData) {
         if (code == 0) {
            QApplication::postEvent(obj, new QEvent(CustomEvent_DownLoadSharePic));
         }
      });
   }
}

void VhallNetworkImageLabel::customEvent(QEvent* event) {
   if (event && event->type() == CustomEvent_DownLoadSharePic) {
      QImage privew_image;
      bool bRet = privew_image.load(mOutFile);
      if (bRet) {
         QPixmap pixmap = QPixmap::fromImage(privew_image);
         if (pixmap.isNull()) {
            pixmap = QPixmap(300, 300);
            pixmap.fill(Qt::transparent);
         }
         else {
            m_userImage = pixmap;
            qDebug() << "VhallNetworkImageLabel::finished " << m_userImage.size();
            emit SigUserImageFinished(m_userImage);
         }

         pixmap = pixmap.scaled(QSize(300, 300), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
         QImage pixImg = pixmap.toImage();

         QPixmap mask = QPixmap(300, 300);
         mask.fill(QColor(0, 0, 0, 0));
         QPen pen;
         pen.setWidth(0);

         QPainter p(&mask);
         p.setRenderHint(QPainter::SmoothPixmapTransform);
         p.setPen(pen);
         p.setBrush(Qt::black);
         p.drawEllipse(0, 0, 290, 290);

         QImage img = mask.toImage();
         for (int i = 0; i < img.width(); i++) {
            for (int j = 0; j < img.height(); j++) {
               if (img.pixel(i, j) != 0) {
                  img.setPixel(i, j, pixImg.pixel(i, j));
               }
            }
         }
         ui->label->setPixmap(QPixmap::fromImage(img.scaled(QSize(30, 30), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
      }
   }
}

void VhallNetworkImageLabel::finished(QNetworkReply *r) {
   QByteArray d = r->readAll(); 
   QPixmap pixmap; 
   pixmap.loadFromData(d); 
   if(pixmap.isNull()) {
      pixmap=QPixmap(300,300);
      pixmap.fill(Qt::transparent);
   }
   else {
      m_userImage = pixmap;
      qDebug()<<"VhallNetworkImageLabel::finished "<<m_userImage.size();
      emit SigUserImageFinished(m_userImage);
   }
   
   pixmap=pixmap.scaled(QSize(300,300),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
   QImage pixImg=pixmap.toImage();
   
   QPixmap mask=QPixmap(300,300);
   mask.fill(QColor(0,0,0,0));
   QPen pen;
   pen.setWidth(0);
   
   QPainter p(&mask);
   p.setRenderHint(QPainter::SmoothPixmapTransform);
   p.setPen(pen);
   p.setBrush(Qt::black); 
   p.drawEllipse(0, 0, 290, 290);

   QImage img=mask.toImage();
   for(int i=0;i<img.width();i++) {
      for(int j=0;j<img.height();j++) {
         if(img.pixel(i,j)!=0) {
            img.setPixel(i,j,pixImg.pixel(i,j));
         }
      }
   }
   ui->label->setPixmap(QPixmap::fromImage(img.scaled(QSize(30,30),Qt::IgnoreAspectRatio,Qt::SmoothTransformation)));
   r->deleteLater();
}

