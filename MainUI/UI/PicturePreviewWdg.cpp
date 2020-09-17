#include "PicturePreviewWdg.h"
#include "VhallNetWorkInterface.h"
#include "ConfigSetting.h"
#include "pathManager.h"
#include "pathManage.h"
#include <QApplication>
#include "DebugTrace.h"
#include "VhallUI_define.h"
#include <QPicture>
#include <QDir>
#include <QMovie>
#include <QDesktopWidget>

#define IMAGE_SAVE_FLODER  QString("image")
#define FIX_BORADER_WIDTH  0 
#define FIX_BORADER_HEIGHT 40

static int MAX_WIDTH = 720;
static int MAX_HEIGHT = 430;

#define LOADING_GIF        QString("border-image: url(:/sysButton/img/sysButton/spin.gif);")
#define LOAD_ERR           QString("border-image: url(:/sysButton/img/sysButton/load_err.png);")
#define LOAD_ERR_LABAL_SIZE      QSize(50,40)
#define LOADING_GIF_SIZE         QSize(16,16)
#define LOADING_ERROR_NOTICE     QString::fromWCharArray(L"加载失败")
#define LOADING_NOTICE           QString::fromWCharArray(L"加载中...")
#define NORMAL_SIZE              QSize(MAX_WIDTH,MAX_HEIGHT)

bool IsDirExist(QString fullPath)
{
   QDir dir(fullPath);
   if (dir.exists()) {
      return true;
   }
   else {
      bool ok = dir.mkpath(fullPath);//创建多级目录
      return ok;
   }
}

bool IsFileExist(QString fullFilePath) {
   QFileInfo fileInfo(fullFilePath);
   if (fileInfo.exists())
   {
      return true;
   }
   return false;
}

ImagePreviewWdg::ImagePreviewWdg(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
    ui.pushButton_close->loadPixmap(":/sysButton/close_button");
    connect(ui.pushButton_close, SIGNAL(sigClicked()), this, SLOT(slot_OnClose()));
    this->setWindowTitle(IMAGE);
    mLeftBtn = new QPushButton(this);
    mRightBtn = new QPushButton(this);
    connect(mLeftBtn, SIGNAL(clicked()), this, SLOT(slot_OnLeft()));
    connect(mRightBtn, SIGNAL(clicked()), this, SLOT(slot_OnRight()));
    mLeftBtn->setStyleSheet("border-image:url(:/sysButton/img/sysButton/left_page.png);");
    mRightBtn->setStyleSheet("border-image:url(:/sysButton/img/sysButton/next_page.png);");
    mLeftBtn->setFixedWidth(60);
    mLeftBtn->setFixedHeight(60);
    mRightBtn->setFixedWidth(60);
    mRightBtn->setFixedHeight(60);
    mLeftBtn->hide();
    mRightBtn->hide();
    QString gif_dir = QApplication::applicationDirPath() + "\\spin.gif";
    mGifMovie = new QMovie(gif_dir);
   this->setFixedSize(NORMAL_SIZE);

}

ImagePreviewWdg::~ImagePreviewWdg()
{
}

void ImagePreviewWdg::slot_OnClose() {
   close();
   setFixedSize(NORMAL_SIZE);
   if (mGifMovie) {
      mGifMovie->stop();
   }
}

void ImagePreviewWdg::customEvent(QEvent *event) {
   if (event) {
      int typeValue = event->type();
      if (CustomEvent_DownLoadFile == typeValue) {
         ShowImage(event);
      }

   }
}

void ImagePreviewWdg::slot_OnLeft() {
   if (mFileIndex > 0) {
      mFileIndex--;
      std::map<int, QString>::iterator iter = mDownLoadFileMap.find(mFileIndex);
      if (iter != mDownLoadFileMap.end()) {
         LoadImageToLabel(iter->second);
      } 
      ShowBtn();
   }
}

void ImagePreviewWdg::slot_OnRight() {
   if (mFileIndex < mPicFileList.size() - 1) {
      mFileIndex++;
      std::map<int, QString>::iterator iter = mDownLoadFileMap.find(mFileIndex);
      if (iter != mDownLoadFileMap.end()) {
         LoadImageToLabel(iter->second);
      }   
      ShowBtn();
   }
}

void ImagePreviewWdg::mousePressEvent(QMouseEvent *event) {
   if (event) {
      mPressPoint = this->pos() - event->globalPos();
      mIsMoved = true;
   }
}

void ImagePreviewWdg::mouseMoveEvent(QMouseEvent *event) {
   if (event && (event->buttons() == Qt::LeftButton) && mIsMoved) {
      this->move(event->globalPos() + mPressPoint);
      this->raise();
   }
}

void ImagePreviewWdg::mouseReleaseEvent(QMouseEvent *) {
   if (mIsMoved) {
      ShowBtn();
      mIsMoved = false;
   }
   QWidget* parent_widget = this;
   QPoint pos = parent_widget->pos();
   QRect rect = QApplication::desktop()->availableGeometry(pos);
   QRect wnd_rect = parent_widget->frameGeometry();
   if (pos.y() > rect.height() - 150) {
      pos.setY(rect.height() - 150);
      parent_widget->move(pos);
   }
   else if (pos.y() < rect.y()) {
      parent_widget->move(pos + QPoint(0, +100));
   }
   else if (wnd_rect.x() >= rect.x() + rect.width() - 100) {
      parent_widget->move(QPoint(wnd_rect.x(), wnd_rect.y()) + QPoint(-100, 0));
   }
   else if (wnd_rect.x() + this->width() < rect.x() + 100) {
      parent_widget->move(QPoint(wnd_rect.x(), wnd_rect.y()) + QPoint(100, 0));
   }
}

void ImagePreviewWdg::enterEvent(QEvent *event) {
   if (!mIsMoved) {
      ShowBtn();
   }
}

void ImagePreviewWdg::ShowBtn() {
   if (mPicFileList.size() > 1) {
      if (mLeftBtn) {
         if(mFileIndex != 0 && mPicFileList.size() >= 2) {
            mLeftBtn->show();
            mLeftBtn->move(15, this->height() / 2);
         }
         else {
            mLeftBtn->hide();
         }
      }
      if (mRightBtn) {
         if (mFileIndex != mPicFileList.size() - 1 && mPicFileList.size() >= 2){
            mRightBtn->move(this->width() - 15 - mRightBtn->width(), this->height() / 2);
            mRightBtn->show();
         }
         else {
            mRightBtn->hide();
         }
      }
   }
}

void ImagePreviewWdg::HideBtn() {
   if (mLeftBtn) {
      mLeftBtn->hide();
   }
   if (mRightBtn) {
      mRightBtn->hide();
   }
}

void ImagePreviewWdg::leaveEvent(QEvent *event) {
   HideBtn();
}

void ImagePreviewWdg::ShowImage(QEvent* event) {
   CustomDownLoadEvent* download = dynamic_cast<CustomDownLoadEvent*>(event);
   //if (download->mCode != 200 && download->mCode != 0) {
   //   return;
   //}

   if (download) {
      if (mFileIndex == download->mIndex) {
         LoadImageToLabel(download->mSavePath);
      }
      mDownLoadFileMap.insert(pair<int, QString>(download->mIndex, download->mSavePath));
   }
}

void ImagePreviewWdg::LoadImageToLabel(const QString& filePath) {

   QRect screen_rect = QApplication::desktop()->availableGeometry(this);

   mGifMovie->start();
   ui.label_gif->setStyleSheet(LOADING_GIF);
   ui.label_gif->setFixedSize(LOADING_GIF_SIZE);
   ui.label_gif->setStyleSheet(LOAD_ERR);
   ui.label_notice->setText(LOADING_NOTICE);
   if (!IsFileExist(filePath)) {
      ui.stackedWidget->setCurrentIndex(1);
      ui.label_gif->clear();
      ui.label_gif->setFixedSize(LOAD_ERR_LABAL_SIZE);
      ui.label_gif->setStyleSheet(LOAD_ERR);
      ui.label_notice->setText(LOADING_ERROR_NOTICE);
      mGifMovie->stop();
      return;
   }

   QImage privew_image;
   bool bRet = privew_image.load(filePath);
   if (bRet) {
      ui.stackedWidget->setCurrentIndex(1);
      QPixmap image = QPixmap::fromImage(privew_image);
      int width = image.width();
      int height = image.height();
      QSize curSize = this->size();
      bool bResize = false;
      while (true) {
         if (width > MAX_WIDTH || height > MAX_HEIGHT - FIX_BORADER_HEIGHT) {
            width = width * 5 / 6;
            height = height * 5 / 6;
            bResize = true;
         }
         else {
            break;
         }
      }

      image = image.scaled(QSize(width, height), Qt::KeepAspectRatio);
      ui.label->setPixmap(image);
      ui.label->show();
      ui.stackedWidget->setCurrentIndex(0);
      emit sig_wndRemove();
      mGifMovie->stop();
   }
   else {
      ui.stackedWidget->setCurrentIndex(1);
      ui.label_gif->clear();
      ui.label_gif->setFixedSize(LOAD_ERR_LABAL_SIZE);
      ui.label_gif->setStyleSheet(LOAD_ERR);
      ui.label_notice->setText(LOADING_ERROR_NOTICE);
      mGifMovie->stop();
   }
}

void ImagePreviewWdg::InsertPicFiles(const std::list<QString>& pic_list, int image_index) {
   if (!isHidden()) {
      this->raise();
   } 
   ui.stackedWidget->setCurrentIndex(1);
   if (mGifMovie) {
      mGifMovie->start();
      ui.label_gif->setMovie(mGifMovie);
   }
   ui.label_gif->setFixedSize(16, 16);
   ui.label_notice->setText(LOADING_NOTICE);
   
   mFileIndex = image_index;
   IsDirExist(QString::fromStdWString(GetAppDataPath()) + IMAGE_SAVE_FLODER);
   mPicFileList = pic_list;
   mDownLoadFileMap.clear();
  
   std::list<QString>::iterator iter = mPicFileList.begin();
   int index = 0;
   while (iter != mPicFileList.end()) {
      QString fileUrl = (*iter);
      QString configPath = CPathManager::GetConfigPath();
      QString proxyHost;
      QString proxyUsr;
      QString proxyPwd;
      int proxyPort;
      int is_http_proxy = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_OPEN, 0);
      if (is_http_proxy) {
         TRACE6("%s is_http_proxy:%d\n", __FUNCTION__, is_http_proxy);
         proxyHost = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_HOST, "");
         proxyPort = ConfigSetting::ReadInt(configPath, GROUP_DEFAULT, PROXY_PORT, 80);
         proxyUsr = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_USERNAME, "");
         proxyPwd = ConfigSetting::ReadString(configPath, GROUP_DEFAULT, PROXY_PASSWORD, "");
      }
      QObject* obj = this;
      int lastIndex = fileUrl.lastIndexOf("/");
      QString fileName = fileUrl.mid(lastIndex, fileUrl.length());
      fileName = fileName.replace("/","\\");
      QString filePath = QString::fromStdWString(GetAppDataPath()) + IMAGE_SAVE_FLODER + fileName;
      TRACE6("%s fileUrl:%s\n", __FUNCTION__, fileUrl.toStdString().c_str());
      if (IsFileExist(filePath)) {
         QApplication::postEvent(obj, new CustomDownLoadEvent(CustomEvent_DownLoadFile, 0, fileUrl, filePath, QString::fromStdString(""), index));
         mDownLoadFileMap.insert(pair<int, QString>(index, filePath));
      } else {
         HTTP_GET_REQUEST request(fileUrl.toStdString());
         request.SetEnableDownLoadFile(true, filePath.toStdString());
         request.mbIsNeedSyncWork = true;
         GetHttpManagerInstance()->HttpGetRequest(request, [&, obj, fileUrl, filePath, index](const std::string& msg, int code, const std::string userData) {
            TRACE6("%s code:%d msg:%s\n", __FUNCTION__, code, msg.c_str());
            QApplication::postEvent(obj, new CustomDownLoadEvent(CustomEvent_DownLoadFile, code, fileUrl, filePath, QString::fromStdString(msg), index));
         });
      }
      iter++;
      index++;
   }
}