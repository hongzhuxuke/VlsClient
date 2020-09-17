#include <windows.h>
#include <comdef.h>//初始化一下com口
#include "GdiPlus.h"
#include <gdiplus.h>
#include <gdiplusgraphics.h>
#include <qtwinextras/qwinfunctions.h>

#include "VideoRenderWdg.h"
#include <QStyleOption>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDir>

#include "IInteractionClient.h"
#include "MainUIIns.h"
#include "Msg_VhallRightExtraWidget.h"
#include "DebugTrace.h"

#define VIEW_BKG_COLOR 0x60	//默认背景颜色值
#define FPS_Count 10 //每渲染10次，更新一次fps

#define SafeDelete(p) {delete p; p = 0;}
#define SafeDeleteArr(pArr) {delete[] pArr; pArr = 0;}
void _CopyBits2Tex_None_0(unsigned char* pDst, unsigned char* pSrc, unsigned int uLen, const SIZE & size) {
   memcpy(pDst, pSrc, uLen);
}

void _CopyBits2Tex_None_90(unsigned char* pDst, unsigned char* pSrc, unsigned int uLen, const SIZE & size) {
   int n = 0;
   int linesize = size.cx * 3;

   for (int x = 0; x < size.cx; x++) {
      for (int y = size.cy - 1; y >= 0; y--) {
         memcpy(pDst + n, pSrc + linesize * y + 3 * x, 3);
         n += 3;
      }
   }
}

void _CopyBits2Tex_None_180(unsigned char* pDst, unsigned char* pSrc, unsigned int uLen, const SIZE & size) {
   int n = 0;
   int linesize = size.cx * 3;

   for (int x = 0; x < size.cx; x++) {
      for (int y = 0; y < size.cy; y++) {
         memcpy(pDst + linesize * (size.cy - y - 1) + 3 * x, pSrc + linesize * y + 3 * (size.cx - x - 1), 3);
      }
   }
}

void _CopyBits2Tex_None_270(unsigned char* pDst, unsigned char* pSrc, unsigned int uLen, const SIZE & size) {
   int n = 0;
   int linesize = size.cx * 3;

   for (int x = size.cx - 1; x >= 0; x--) {
      for (int y = 0; y < size.cy; y++) {
         memcpy(pDst + n, pSrc + linesize * y + 3 * x, 3);
         n += 3;
      }
   }
}
bool _ResizeWithMendBlack(unsigned char* pDst, unsigned char* pSrc, unsigned int uDstLen, unsigned int uSrcLen, const SIZE & dstSize, const SIZE & srcSize, unsigned int bpp) {
   if (!pDst || !pSrc)
      return false;

   if (uDstLen == 0 || uDstLen != dstSize.cx * dstSize.cy * bpp)
      return false;

   if (uSrcLen == 0 || uSrcLen != srcSize.cx * srcSize.cy * bpp)
      return false;

   if (dstSize.cx < srcSize.cx)
      return false;

   if (dstSize.cy < srcSize.cy)
      return false;

   UINT dstLineblockSize = dstSize.cx * bpp;
   UINT srcLineblockSize = srcSize.cx * bpp;

   int mendCxLeftEnd = dstSize.cx > srcSize.cx ? (dstSize.cx - srcSize.cx) / 2 : 0;
   int mendCyToEnd = dstSize.cy > srcSize.cy ? (dstSize.cy - srcSize.cy) / 2 : 0;

   for (int y = 0; y < dstSize.cy; y++) {
      if (y >= mendCyToEnd && y < mendCyToEnd + srcSize.cy) {
         if (mendCxLeftEnd > 0)
            memcpy(pDst + (bpp * mendCxLeftEnd), pSrc, srcLineblockSize);
         else
            memcpy(pDst, pSrc, srcLineblockSize);
         pSrc += srcLineblockSize;
      }
      pDst += dstLineblockSize;
   }
   return true;
}

CVideoRenderWdg::CVideoRenderWdg(QWidget* parent/* = 0*/, WND_TYPE type /* = WND_TYPE_TOP*/, Qt::WindowFlags f/* = 0*/)
:QWidget(parent)
, m_wndType(type)
, m_colorFormat(8)
, m_frameDataBufLen(0)
, m_pFrameDataBuf(NULL)
, m_pBkgDataBuf(NULL)
, m_nRenderDataBufLen(0)
, m_pRenderDataBuf(NULL)
, m_pParentWidget(parent)
, m_bPause(false)
, m_lastClock(0)
, m_nFpsCounter(FPS_Count)
, m_nFps(0) {
   m_hwnd = (HWND)winId();
   setStyleSheet("background-color:gray;");
   QString tmpDisableMicPath = QDir::tempPath() + "/iconDisableMic.png";
   QString tmpScreenPath = QDir::tempPath() + "/iconScreen.png";
   QPixmap(":/interactivity/iconDisableMic").save(tmpDisableMicPath);
   QPixmap(":/interactivity/iconScreen").save(tmpScreenPath);
   m_iconDisableMicImagePixMap.load(tmpDisableMicPath);
   m_iconScreenPixMap.load(tmpScreenPath);
   SetMicDisableIcon(false);
   SetScreenEnableIcon(false);
   connect(&manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(DownloadImgFinished(QNetworkReply *)));
   this->setMouseTracking(false);
   mLastRenderFrameTime = 0;
}

CVideoRenderWdg::~CVideoRenderWdg() {
   m_renderTimerOut.stop();
   if (m_pBkgDataBuf) {
      SafeDeleteArr(m_pBkgDataBuf);
   }

   if (m_pFrameDataBuf) {
      SafeDeleteArr(m_pFrameDataBuf);
   }

   if (m_pRenderDataBuf != NULL) {
      SafeDeleteArr(m_pRenderDataBuf);
   }
}
void CVideoRenderWdg::DownloadImgFinished(QNetworkReply *r) {
   qDebug() << "CVideoRenderWdg::DownloadImgFinished";
   QByteArray d = r->readAll();
   m_imageUserPixMap.loadFromData(d);
   if (!m_imageUserPixMap.isNull()) {
      int index = m_headImageUrl.lastIndexOf("/");
      QString fileName = m_headImageUrl.mid(index + 1, m_headImageUrl.length());
      if (!fileName.isEmpty()) {
         QString tmpName = QDir::tempPath() + "/" + fileName;
         if (m_imageUserPixMap.save(tmpName)) {
            m_headPicName = tmpName;
            if (m_drawUserImage) {
               Clear(true);
               emit Sig_DrawUserImage(m_userid);
            }
         }
      }
   }
   r->deleteLater();
}

bool CVideoRenderWdg::DoRender(VHVideoFrame pFrameData) {

   if (m_bPause) {
      return false;
   }

   if (pFrameData.data_size == 0) {
      TRACE6("%s pFrameData.data_size == 0", __FUNCTION__);
      return false;
   }

   if (m_frameDataBufLen != pFrameData.data_size) {
      if (m_pFrameDataBuf) {
         SafeDeleteArr(m_pFrameDataBuf)
         m_pFrameDataBuf = NULL;
      }
      if (m_pBkgDataBuf) {
         SafeDeleteArr(m_pBkgDataBuf);
         m_pBkgDataBuf = NULL;
      }

      m_colorFormat = pFrameData.color_format;
      m_frameDataBufLen = pFrameData.data_size;
      m_pFrameDataBuf = new unsigned char[m_frameDataBufLen + 1];
      m_pBkgDataBuf = new unsigned char[m_frameDataBufLen + 1];
      memset(m_pFrameDataBuf, VIEW_BKG_COLOR, m_frameDataBufLen);
      memset(m_pBkgDataBuf, VIEW_BKG_COLOR, m_frameDataBufLen);
   }

   static void(*fun_table[4])(unsigned char*, unsigned char*, UINT, const SIZE &) = { &_CopyBits2Tex_None_0, &_CopyBits2Tex_None_90, &_CopyBits2Tex_None_180, &_CopyBits2Tex_None_270 };

   if (pFrameData.rotate == 0) { //0
      m_frameWidth = pFrameData.width;
      m_frameHeight = pFrameData.height;
   } else if (pFrameData.rotate == 1) {  //90
      m_frameWidth = pFrameData.height;
      m_frameHeight = pFrameData.width;
   } else if (pFrameData.rotate == 2){  //180
      m_frameWidth = pFrameData.width;
      m_frameHeight = pFrameData.height;
   } else if (pFrameData.rotate == 3){ //270
      m_frameWidth = pFrameData.height;
      m_frameHeight = pFrameData.width;
   }

   if (m_pFrameDataBuf == NULL) {
      m_frameDataBufLen = 0;
      return false;
   }
   const SIZE size = { pFrameData.width, pFrameData.height };
   memset(m_pFrameDataBuf, 0, m_frameDataBufLen);
   fun_table[pFrameData.rotate](m_pFrameDataBuf, pFrameData.data, m_frameDataBufLen, size);

   //paintPic(m_pBkgDataBuf);
   m_drawUserImage = false;
   paintPic(m_pFrameDataBuf, m_frameDataBufLen, true);
   return true;
}

void CVideoRenderWdg::Clear(bool bforce) {
   if (!bforce) {
      if (QDateTime::currentDateTime().toMSecsSinceEpoch() - m_lastTime < 500) {
         return;
      }
   }

   m_frameWidth = width();
   m_frameHeight = height();
   if (m_pBkgDataBuf) {
      SafeDeleteArr(m_pBkgDataBuf);
      m_pBkgDataBuf = NULL;
   }

   if (m_pRenderDataBuf) {
      SafeDeleteArr(m_pRenderDataBuf);
      m_pRenderDataBuf = NULL;
   }

   m_nRenderDataBufLen = m_frameWidth * m_frameHeight * 3;
   m_pBkgDataBuf = new unsigned char[m_nRenderDataBufLen];
   m_pRenderDataBuf = new unsigned char[m_nRenderDataBufLen];

   memset(m_pBkgDataBuf, VIEW_BKG_COLOR, m_nRenderDataBufLen);
   memset(m_pRenderDataBuf, VIEW_BKG_COLOR, m_nRenderDataBufLen);

   paintPic(m_pBkgDataBuf, m_nRenderDataBufLen, false);
   m_lastClock = 0;
}

void CVideoRenderWdg::pauseRender() {
   m_bPause = true;
   //m_pParentWidget->setEnabled(false);
}

void CVideoRenderWdg::recoverRender() {
   m_bPause = false;
   //m_pParentWidget->setEnabled(true);
}

void CVideoRenderWdg::enterFullScreen() {
   if (!isFullScreen()) {
      m_pParentWidget = parentWidget();
      m_Rect = geometry();
      this->setParent(NULL);
      this->showFullScreen();
   }
}

void CVideoRenderWdg::exitFullScreen() {
   if (isFullScreen()) {
      this->setParent(m_pParentWidget);
      this->setGeometry(m_Rect);
      this->showNormal();
   }
}

void CVideoRenderWdg::EnableCamera(bool bEnable) {
   m_bEnableCamera = bEnable;
}
void CVideoRenderWdg::EnableScreen(bool bEnable) {
   m_bEnableScreen = bEnable;
}
void CVideoRenderWdg::EnableAll(bool bEnable) {
   m_bEnableAll = bEnable;
}

void CVideoRenderWdg::paintEvent(QPaintEvent * event) {
   QPainter painter(this);
   painter.drawPixmap(0, 0, m_VideoPaintPixMap);
   if (m_bIconDisableMicImageShow || m_bIconScreenShow) {
      int iconScreenWidth = 0;
      int iconScreenHeight = 0;
      int iconMicDisableWidth = 0;
      int iconMicDisableHeight = 0;
      const int bottom = 5;
      const int right = 5;
      const int space = 5;
      if (m_bIconScreenShow) {
         iconScreenWidth = m_iconScreenPixMap.width();
         iconScreenHeight = m_iconScreenPixMap.height();
         painter.drawPixmap(this->width() - iconScreenWidth - right,
                            this->height() - iconScreenHeight - bottom,
                            iconScreenWidth, iconScreenHeight, m_iconScreenPixMap);
      }

      if (m_bIconDisableMicImageShow) {
         iconMicDisableWidth = m_iconDisableMicImagePixMap.width();
         iconMicDisableHeight = m_iconDisableMicImagePixMap.height();
         painter.drawPixmap(this->width() - iconScreenWidth - right - space - iconMicDisableWidth,
                            this->height() - iconMicDisableHeight - bottom,
                            iconMicDisableWidth, iconMicDisableHeight, m_iconDisableMicImagePixMap);
      }
   }

   if (m_bEnableCamera == false && m_bPaintFromRender == false) {
      int x = 0, y = 0;
      QRect rect = this->rect();
      x = (rect.width() - 50) / 2;
      y = (rect.height() - 50) / 2;
      painter.drawPixmap(x, y, 50, 50, m_imageUserPixMap);
   }
   int imageWidth = m_nameUserPixMap.width();
   int imageHeigth = m_nameUserPixMap.height();
   int x = 0, y = 0;
   y = this->height() - imageHeigth - 5;
   painter.drawPixmap(x, y, imageWidth, imageHeigth, m_nameUserPixMap);
}

void CVideoRenderWdg::mouseDoubleClickEvent(QMouseEvent * event) {
   if (event && event->button() == Qt::LeftButton) {
		QJsonObject body;
      if (m_bIsFullWnd) {//normal
         emit exitFullScreen(this);
			body["ac"] = "Normal";
      } else {//max
         emit applyFullScreen(this);
			body["ac"] = "Max";
      }
		SingletonMainUIIns::Instance().reportLog(L"Interaction_DoubleClickRender", eLogRePortK_InteractionRenderWdg_DoubleClick, body);
   }

}
void CVideoRenderWdg::SetMicDisableIcon(bool bShow) {
   m_bIconDisableMicImageShow = bShow;
}
void CVideoRenderWdg::SetScreenEnableIcon(bool bShow) {
   m_bIconScreenShow = bShow;
}

void CVideoRenderWdg::paintPic(unsigned char* pData, int frameLen, bool bPaintFromRender /*= false*/) {
   m_bPaintFromRender = bPaintFromRender;
   if (bPaintFromRender) {
      m_bIsFrameRender = true;
      m_renderTimerOut.stop();
      m_renderTimerOut.start(MAX_TIMEOUT_RENDER);
   }
   m_lastTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
   int winWidth = width();
   int winHeight = height();
   int srcWidth = m_frameWidth;
   int srcHeight = m_frameHeight;

   int dstWidth = 0;
   int dstHeight = 0;

   float fWinRate = winWidth / (float)winHeight;
   float fSrcRate = srcWidth / (float)srcHeight;
   //高不变，宽填充黑边
   if (fWinRate > fSrcRate) {
      dstWidth = (int)(srcHeight * winWidth / (float)winHeight);
      dstHeight = srcHeight;
   }
   //宽不变,高填充黑边
   else {
      dstWidth = srcWidth;
      dstHeight = (int)(srcWidth * winHeight / (float)winWidth);
   }
   dstWidth = (dstWidth + 3) / 4 * 4; //保证宽度是4的倍数。

   if (m_nRenderDataBufLen != dstWidth * dstHeight * 3) {
      if (m_pRenderDataBuf != NULL) {
         SafeDeleteArr(m_pRenderDataBuf);
      }

      m_nRenderDataBufLen = dstWidth * dstHeight * 3; //RGB24
      m_pRenderDataBuf = new unsigned char[m_nRenderDataBufLen];
   }
   memset(m_pRenderDataBuf, 0, m_nRenderDataBufLen);//清除上一次渲染

   SIZE srcSize = { srcWidth, srcHeight };
   SIZE dstSize = { dstWidth, dstHeight };

   _ResizeWithMendBlack(m_pRenderDataBuf, pData, m_nRenderDataBufLen, frameLen, dstSize, srcSize, 3);
   BITMAPINFO Bitmap;
   memset(&Bitmap, 0, sizeof(BITMAPINFO));
   Bitmap.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   Bitmap.bmiHeader.biWidth = dstWidth;
   Bitmap.bmiHeader.biHeight = -dstHeight;
   Bitmap.bmiHeader.biBitCount = 3 * 8;//COLOR_FORMAT_RGB24	
   Bitmap.bmiHeader.biPlanes = 1;
   Bitmap.bmiHeader.biCompression = BI_RGB;//COLOR_FORMAT_RGB24	
   Bitmap.bmiHeader.biSizeImage = 0;
   Bitmap.bmiHeader.biClrUsed = 0;
   Bitmap.bmiHeader.biXPelsPerMeter = 0;
   Bitmap.bmiHeader.biYPelsPerMeter = 0;
   Bitmap.bmiHeader.biClrImportant = 0;

   HWND hWnd = m_hwnd;
   HDC hDC = GetDC(hWnd);
   HDC hMemDC = CreateCompatibleDC(hDC);
   HBITMAP hMemBitmap = CreateCompatibleBitmap(hDC, winWidth, winHeight);
   SelectObject(hMemDC, hMemBitmap);
   SetBkMode(hMemDC, TRANSPARENT);
   SetStretchBltMode(hMemDC, HALFTONE);
   SetBrushOrgEx(hMemDC, 0, 0, NULL);
   StretchDIBits(hMemDC, 0, 0, winWidth, winHeight, 0, 0, dstWidth, dstHeight, m_pRenderDataBuf, &Bitmap, DIB_RGB_COLORS, SRCCOPY);

   m_VideoPaintPixMap = QtWin::fromHBITMAP(hMemBitmap);
   DeleteObject(hMemBitmap);
   DeleteObject(hMemDC);
   ReleaseDC(hWnd, hDC);

   update();
}

void CVideoRenderWdg::contextMenuEvent(QContextMenuEvent *) {
   SelectUserInfo user;
   memset(&user, 0, sizeof(UserInfo));
   wcscpy(user.userInfo.m_szUserID, m_userid.toStdWString().c_str());
   user.selectType = SELECT_FROM_RENDER_WND;
   user.renderWndType = m_wndType;
   SingletonMainUIIns::Instance().PostCRMessage(MSG_VHALLRIGHTEXTRAWIDGET_RIGHT_MOUSE_BUTTON_USER, &user, sizeof(SelectUserInfo));
}

void CVideoRenderWdg::SetID(const QString& id) {
   m_userid = id;
}

void CVideoRenderWdg::SetHeadImageUrl(const QString& url) {
   if (url == "") {
      return;
   }
   bool firstLoad = false;
   if (m_headImageUrl.isEmpty()) {
      firstLoad = true;
   }
   m_headImageUrl = url;
   if (!LocalFileExist(m_headImageUrl)) {
      manager.get(QNetworkRequest(QUrl(url)));
   }
}
void CVideoRenderWdg::SetUserName(QString name) {
   this->m_userName = name;
   QString tmpName = QDir::tempPath() + QString("/%1.png").arg(m_userid);
   qDebug() << "CVideoRenderWdg::SetUserName" << name << tmpName;
   if (name.length() > 5) {
      name = name.left(5) + "...";
   }

   int w = QPainter().fontMetrics().width(name);
   int h = QPainter().fontMetrics().height();

   QPixmap pixmap = QPixmap(w, h);
   pixmap.fill(QColor(0, 0, 0, 80));
   QPainter p(&pixmap);
   p.setPen(Qt::white);
   p.drawText(pixmap.rect(), name);
   pixmap.save(tmpName);
   m_nameUserPixMap.load(tmpName);
}

QString CVideoRenderWdg::GetID() {
   return m_userid;
}

bool CVideoRenderWdg::GetPauseState() {
   return m_bPause;
}

void CVideoRenderWdg::SetDrawImage(bool bDraw) {
   m_drawUserImage = bDraw;
}

bool CVideoRenderWdg::LocalFileExist(QString &url) {
   int index = url.lastIndexOf("/");
   QString fileName = url.mid(index + 1, url.length());
   if (!fileName.isEmpty()) {
      QString localFile = QDir::tempPath() + "/" + fileName;
      QFile file(localFile);
      if (m_imageUserPixMap.isNull()) {
         if (file.open(QIODevice::ReadOnly)) {
            file.close();
            m_headPicName = localFile;
            if (!m_imageUserPixMap.load(localFile)) {
               return false;
            }
            return true;
         } else {
            return false;
         }
      } else if (!m_imageUserPixMap.isNull() && file.open(QIODevice::ReadOnly)) {
         file.close();
         return true;
      }
   }
   return false;
}

QString CVideoRenderWdg::GetUserImageFilePath() {
   if (!m_headPicName.isEmpty()) {
      return m_headPicName;
   } else if ((m_headPicName.isEmpty() || m_imageUserPixMap.isNull()) && !m_headImageUrl.isEmpty()) {
      manager.get(QNetworkRequest(QUrl(m_headImageUrl)));
   }
   return QString();
}

bool CVideoRenderWdg::IsShowFull() {
   return m_bIsFullWnd;
}

void CVideoRenderWdg::SetFullState(bool fullWnd) {
   m_bIsFullWnd = fullWnd;
   if (m_bIsFullWnd) {
      this->setMouseTracking(true);
   }
   else {
      this->setMouseTracking(false);
   }
}

bool CVideoRenderWdg::GetMicIconState() {
   return m_bIconDisableMicImageShow;
}

void CVideoRenderWdg::Slot_renderTimeOut() {
   m_bIsFrameRender = false;
}

bool CVideoRenderWdg::IsFrameRender() {
   return m_bIsFrameRender;
}

void CVideoRenderWdg::mouseMoveEvent(QMouseEvent *) {
   if (m_bIsFullWnd && m_wndType == WND_TYPE_CENTER) {
      emit Sig_MouseMoveInCenterRender();
   }
}

unsigned int CVideoRenderWdg::GetLastRenderTime() {
   return mLastRenderFrameTime;
}

void CVideoRenderWdg::SetLastRenderTime() {
   mLastRenderFrameTime = QDateTime::currentDateTime().toTime_t();
}

void CVideoRenderWdg::SetMixStream(bool bMix) {
   mIsMixStream = bMix;
}

bool CVideoRenderWdg::IsMixStream() {
   return mIsMixStream;
}

