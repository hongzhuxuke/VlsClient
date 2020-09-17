#ifndef VideoRender_h_
#define VideoRender_h_

#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <QTimer>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkProxy>
#include <QtOpenGL>
#include <QOpenGLTexture> 
#include <atomic>
 
#include "time.h"
#include "IInterActivitySDK.h"
#include "Msg_MainUI.h"

#define MAX_TIMEOUT_RENDER 5000
class CVideoRenderWdg : public QWidget
{
	Q_OBJECT
public:
   CVideoRenderWdg(QWidget* parent = 0, WND_TYPE type = WND_TYPE_TOP, Qt::WindowFlags f = 0);
   ~CVideoRenderWdg();
	bool	DoRender(VHVideoFrame pFrameData);
	void	Clear(bool bforce  = false);
   void  SetID(const QString& id);
   QString GetID();
	void	pauseRender();
	void	recoverRender();
   bool  GetPauseState();

	void	enterFullScreen();//È«ÆÁ
	void	exitFullScreen();//ÍË³öÈ«ÆÁ
   void EnableCamera(bool);

   inline bool GetEnableCamera() { return m_bEnableCamera;}
   void EnableScreen(bool);
   inline bool GetEnableScreen(){ return m_bEnableScreen;}
   void EnableAll(bool);

   void SetMicDisableIcon(bool);
   bool GetMicIconState();
   void SetScreenEnableIcon(bool);

   void SetHeadImageUrl(const QString& url);
   void SetUserName(QString);

   void SetDrawImage(bool bDraw);
   bool GetIsDrawImage() {return m_drawUserImage;};
   QString GetUserImageUrl() {return m_headImageUrl;};
   QString GetUserImageFilePath();
   bool IsShowFull();
   void SetFullState(bool full);
   bool IsFrameRender();
   unsigned int GetLastRenderTime();
   void SetLastRenderTime();

   void SetMixStream(bool bMix);
   bool IsMixStream();
public slots:
   void DownloadImgFinished(QNetworkReply *r);
   void Slot_renderTimeOut();
signals:
	void	applyFullScreen(CVideoRenderWdg* pRender);
	void	exitFullScreen(CVideoRenderWdg* pRender);
   void  Sig_DrawUserImage(QString);
   void Sig_MouseMoveInCenterRender();
protected:
	void	paintEvent(QPaintEvent * event) override;
	void	mouseDoubleClickEvent( QMouseEvent * event ) override;
   virtual void contextMenuEvent(QContextMenuEvent *);
   virtual void mouseMoveEvent(QMouseEvent *);

private:
   void	paintPic(unsigned char* pData, int frameLen , bool bPaintFromRender = false );
   bool LocalFileExist(QString &url);
private:
	int m_colorFormat;
	unsigned int		m_frameDataBufLen;	//Ö¡Êý¾Ý³¤¶È
	unsigned char*		m_pFrameDataBuf;	//Ö¡Êý¾Ý»º´æ
	unsigned char*		m_pBkgDataBuf;		//±³¾°»º´æ

	unsigned int		m_frameWidth;		//Ö¡¿í¶È
	unsigned int		m_frameHeight;		//Ö¡¸ß¶È

	unsigned int		m_nRenderDataBufLen;//äÖÈ¾»º´æ´óÐ¡
	unsigned char*		m_pRenderDataBuf;	//äÖÈ¾»º´æ

	QWidget*	m_pParentWidget;
	QRect		m_Rect;
	bool		m_bPause;//ÊÇ·ñÔÝÍ£äÖÈ¾	

	clock_t		m_lastClock;
	int			m_nFpsCounter;
	int			m_nFps;
   HWND        m_hwnd = NULL;
   bool m_bEnableCamera = false;
   bool m_bEnableScreen = false;
   bool m_bEnableAll = false;

   QString m_userid;
   bool m_bIconDisableMicImageShow = false;
   bool m_bIconScreenShow = false;

   QString m_headImageUrl;
   QNetworkAccessManager manager;

   QString m_userName;
   int m_wndType;

   qint64 m_lastTime;
   bool m_drawUserImage = true;
   QString m_headPicName;

   bool m_bIsFullWnd = false;
   QTimer m_renderTimerOut;
   bool m_bIsFrameRender = false;

   QPixmap m_VideoPaintPixMap;
   QPixmap m_iconDisableMicImagePixMap;
   QPixmap m_iconScreenPixMap;
   QPixmap m_imageUserPixMap;
   QPixmap m_nameUserPixMap;
   bool m_bPaintFromRender = false;
   std::atomic_uint mLastRenderFrameTime;
   bool mIsMixStream = true;

};

#endif //VideoRender_h_
