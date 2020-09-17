#include "vhallsharedqr.h"
#include "ui_vhallsharedqr.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QImage>
#include <QDir>
#include <QClipboard>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DebugTrace.h"
#include "pub.Const.h"
#include "MainUIIns.h"
#include "httpnetwork.h"
#include "pathmanager.h"
#include "ConfigSetting.h"
#include "VhallNetWorkInterface.h"

VhallSharedQr::VhallSharedQr(QWidget *parent) :
CBaseDlg(parent),
ui(new Ui::VhallSharedQr) {
   ui->setupUi(this);
   setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog /*|Qt::WindowStaysOnTopHint*/);
   setAttribute(Qt::WA_TranslucentBackground);

   ui->label_qr->installEventFilter(this);
   ui->widget_title->installEventFilter(this);

   //mBackgroundPixmap.load(":/shared/img/shared/background.png");
   m_pBtnClose = new TitleButton(this);
   m_pBtnClose->loadPixmap(":/sysButton/close_button");
   ui->horizontalLayout->addWidget(m_pBtnClose);
   connect(m_pBtnClose, SIGNAL(clicked()), this, SLOT(close()));
   TRACE6("%s VhallSharedQr::VhallSharedQr end\n", __FUNCTION__);
   //mHttpNetWork = new HttpNetWork(this);
   //if (mHttpNetWork) {
   //   connect(mHttpNetWork, SIGNAL(HttpNetworkGetFinished(QByteArray, int)), this, SLOT(Slot_OnCheckMixStreamServerFinished(QByteArray, int)));
   //}
   connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(Slot_OnCopyShareUrl()));
   mLoadSharePicTimer = new QTimer(this);
   connect(mLoadSharePicTimer, SIGNAL(timeout()), this, SLOT(Slot_OnLoadSharePic()));
}

VhallSharedQr::~VhallSharedQr() {
   //if (mHttpNetWork) {
   //   delete mHttpNetWork;
   //   mHttpNetWork = NULL;
   //}
   delete ui;
}

void VhallSharedQr::Shared(QString domain, QString streamID) {
   TRACE6("%s\n", __FUNCTION__);
   QString shareUrl = domain + "/" + streamID;
   if (domain.isEmpty() || streamID.isEmpty()) {
      return;
   }

   mOutFile = QDir::tempPath() + "/" + streamID + ".png";
   if (QFile::exists(mOutFile)) {
      ui->label_qr->setPixmap(QPixmap::fromImage(QImage(mOutFile)).scaled(ui->label_qr->size()));
      QFile::remove(mOutFile);
   }
   else {
      ui->label_qr->setPixmap(QPixmap());
   }

   QString toolConfPath = CPathManager::GetAppDataPath() + QString::fromStdWString(VHALL_TOOL_CONFIG);
   QString downloadUrl = ConfigSetting::ReadString(toolConfPath, GROUP_DEFAULT, KEY_VHALL_SHARE_LIVE_PIC, "http://aliqr.e.vhall.com/qr.png?t=");
   downloadUrl = downloadUrl + domain + "/" + streamID;

   HTTP_GET_REQUEST httpRequest(downloadUrl.toStdString());
   httpRequest.SetHttpPost(true);
   httpRequest.SetEnableDownLoadFile(true, mOutFile.toStdString());
   httpRequest.mbIsNeedSyncWork = true;
   QObject* obj = this;
   GetHttpManagerInstance()->HttpGetRequest(httpRequest, [&, obj](const std::string& msg, int code, const std::string userData) {
      TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
      //QApplication::postEvent(obj,new QEvent(CustomEvent_DownLoadSharePic));
   });
   if (mLoadSharePicTimer) {
      mLoadSharePicTimer->start(2000);
   }
   shareUrl = shareUrl.replace("http", "https");
   ui->lineEdi_url->setText(shareUrl);
   show();
}

void VhallSharedQr::customEvent(QEvent* event) {
   if (event) {
      if (event->type() == CustomEvent_DownLoadSharePic) {
         if (QFile::exists(mOutFile)) {
            ui->label_qr->setPixmap(QPixmap::fromImage(QImage(mOutFile)).scaled(ui->label_qr->size()));
         }
      }
   }
}

void VhallSharedQr::CenterWindow(QWidget* parent) {
   int x = 0;
   int y = 0;
   if (NULL == parent) {
      const QRect rect = QApplication::desktop()->availableGeometry();
      x = rect.left() + (rect.width() - width()) / 2;
      y = rect.top() + (rect.height() - height()) / 2;
   } else {
      QPoint point(0, 0);
      point = parent->mapToGlobal(point);
      x = point.x() + (parent->width() - width()) / 2;
      y = point.y() + (parent->height() - height()) / 2;
   }
   move(x, y);
}

bool VhallSharedQr::eventFilter(QObject *obj, QEvent *ev) {
   if (obj == ui->label_qr) {
      if (ev->type() == QEvent::Resize) {
         ui->label_qr->setPixmap(QPixmap::fromImage(QImage(mOutFile)).scaled(ui->label_qr->size()));
      }
   } else if (obj == ui->widget_title) {
      if (ev->type() == QEvent::MouseButtonPress) {
         this->pressPoint = this->cursor().pos();
         this->startPoint = this->pos();
      } 
      else if (ev->type() == QEvent::MouseMove) {
         int dx = this->cursor().pos().x() - this->pressPoint.x();
         int dy = this->cursor().pos().y() - this->pressPoint.y();
         this->move(this->startPoint.x() + dx, this->startPoint.y() + dy);
      }
      else if (ev->type() == QEvent::MouseButtonRelease) {
         QPoint pos = this->pos();
         QRect rect = QApplication::desktop()->availableGeometry(pos);
         QRect wnd_rect = this->frameGeometry();
         if (pos.y() > rect.height() - 150) {
            pos.setY(rect.height() - 150);
            this->move(pos);
         }
         else if (pos.y() < rect.y()) {
            this->move(pos + QPoint(0, +100));
         }
         else if (wnd_rect.x() >= rect.x() + rect.width() - 100) {
            this->move(QPoint(wnd_rect.x(), wnd_rect.y()) + QPoint(-100, 0));
         }
         else if (wnd_rect.x() + this->width() < rect.x() + 100) {
            this->move(QPoint(wnd_rect.x(), wnd_rect.y()) + QPoint(100, 0));
         }
      }
   }
   return QWidget::eventFilter(obj, ev);
}

void VhallSharedQr::Slot_OnClose() {
   close();
   ui->label_qr->setPixmap(QPixmap());
}

void VhallSharedQr::Slot_OnCopyShareUrl() {
    QApplication::clipboard()->setText(ui->lineEdi_url->text());
    if (this->parent()) {
        QApplication::postEvent(this->parent(), new CustomOnHttpResMsgEvent(CustomEvent_CopyShareUrl));
    }
}

void VhallSharedQr::Slot_OnLoadSharePic() {
   if (QFile::exists(mOutFile)) {
      ui->label_qr->setPixmap(QPixmap::fromImage(QImage(mOutFile)).scaled(ui->label_qr->size()));
      if (mLoadSharePicTimer) {
         mLoadSharePicTimer->stop();
      }
   }
}

void VhallSharedQr::Slot_OnCheckMixStreamServerFinished(QByteArray data, int errCode) {
   int dataLen = data.length();
   QByteArray fileArray = data;
   QFile file(mOutFile);//文件的名字
   if (file.open(QFile::WriteOnly)) {
      file.write(fileArray);
      file.close();
      TRACE6("%s download pic suc\n", __FUNCTION__);
   }
   ui->label_qr->setPixmap(QPixmap::fromImage(QImage(mOutFile)).scaled(ui->label_qr->size()));
}

