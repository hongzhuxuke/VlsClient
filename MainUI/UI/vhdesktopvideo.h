#ifndef VHDESKTOPVIDEO_H
#define VHDESKTOPVIDEO_H

#include <QWidget>
#include <QMenu>
#include <QAction>
namespace Ui {
class VHDesktopVideo;
}

#define ScaleHight (float)9
#define ScaleWidth (float)16

#define SCALE_NEW_HEIGHT(w)   ScaleHight / ScaleWidth * (float)w
#define SCALE_NEW_WIDTH(h) ScaleWidth / ScaleHight * (float)h

#define INIT_POS_OFF_WIDTH             240    //初始位置窗口便宜位置

#define RENDER_WND_MIN_WIDTH           200
#define RENDER_WND_MIN_HEIGHT          120

#define MAIN_WND_MIN_WIDTH                204             
#define MAIN_WND_MIN_HEIGHT_ONE_VIEW      124
#define MAIN_WND_MIN_HEIGHT_DOUBLE_VIEW   244

class VHDesktopVideo : public QWidget
{
   enum MOUSE_INSIDE {
      MOUSE_INSIDE_UNKNOWN = 0,
      MOUSE_INSIDE_LEFT = 1,
      MOUSE_INSIDE_RIGHT = 2,
      MOUSE_INSIDE_TOP = 3,
      MOUSE_INSIDE_DOWN = 4,
      MOUSE_INSIDE_LEFT_DOWN = 5,
      MOUSE_INSIDE_LEFT_TOP = 6,
      MOUSE_INSIDE_RIGHT_DOWN = 7,
      MOUSE_INSIDE_RIGHT_TOP = 8,
      MOUSE_INSIDE_VIDEO0 = 9,
      MOUSE_INSIDE_VIDEO1 = 10
   };

    Q_OBJECT

public:
    explicit VHDesktopVideo(QWidget *parent = 0);
    ~VHDesktopVideo();
    HWND LockVideo(QString deviceID = QString(), int index = -1);
    void UnlockVideo(HWND);
    void Show();
    void ResetLockVideo();
    void Hide();
    void InitPos();
    void SetDesktopShare(bool start);

signals:
   void SigResizeVideo();
public slots:
   void SlotResizeVideo();
protected:
    bool eventFilter(QObject *, QEvent *);
    MOUSE_INSIDE GetMouseInsideType();
    void UpdateWindowsSize(MOUSE_INSIDE type, QEvent *event);
    void MoveWindows();
private:
    void ResizeVideo();

private slots:
   void SlotResetInitSize(bool);
   void SlotCloseCamera(bool);

private:
    Ui::VHDesktopVideo *ui;
    HWND videos[2];
	bool videoType[2] = {0};
    QString videoDeviceID[2];
    QPoint pressPoint;
    QPoint startPoint;
    bool bSwap = false;

    QPoint m_PressPoint;
    QPoint m_StartPoint;
    QRect m_startRect;
    QPoint m_startPoint;


    QPoint m_leftTopPos;
    QPoint m_leftBottomPos;
    QPoint m_rightTopPos;
    QPoint m_rightBottomPos;

    bool m_mousePressed = false;		// 鼠标是否按下
    MOUSE_INSIDE m_insideType;
    int nInitX;
    int nInitY;

    bool m_bInitSize;
    QPoint m_LastPos;
    QRect m_lastWndRect;
};

#endif // VHDESKTOPVIDEO_H
