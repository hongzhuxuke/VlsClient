#pragma once

#include <QWidget>
#include "ui_MemberCtrlMaskWdg.h"
#include "CToolTips.h"

#define MIC_OPEN_STATE_CSS "QPushButton:pressed{border-image:url(:/vhallActive/img/vhallactive/mic_click.png);}\
QPushButton:hover{border-image:url(:/vhallActive/img/vhallactive/mic_hover.png);}\
QPushButton{border-image:url(:/vhallActive/img/vhallactive/mic_normal.png);}"

#define MIC_CLOSE_STATE_CSS "QPushButton:pressed{border-image:url(:/vhallActive/img/vhallactive/mic_mute_click.png);}\
QPushButton:hover{border-image:url(:/vhallActive/img/vhallactive/mic_mute_hover.png);}\
QPushButton{border-image:url(:/vhallActive/img/vhallactive/mic_mute.png);}"


#define CAMERA_OPEN_STATE_CSS "QPushButton{border-image:url(:/vhallActive/img/vhallactive/camera_normal.png);}\
QPushButton:hover{border-image:url(:/vhallActive/img/vhallactive/camera_hover.png);}\
QPushButton:pressed{border-image:url(:/vhallActive/img/vhallactive/camera_click.png);}"

#define CAMERA_CLOSE_STATE_CSS "QPushButton{border-image:url(:/vhallActive/img/vhallactive/camera_mute.png);}\
QPushButton:hover{border-image:url(:/vhallActive/img/vhallactive/camera_mute_hover.png);}\
QPushButton:pressed{border-image:url(:/vhallActive/img/vhallactive/camera_mute_click.png);}"

#define USER_TOSPEAK_STATE_CSS "QPushButton{border-image:url(:/vhallActive/img/vhallactive/speak.png);}\
QPushButton:hover{border-image:url(:/vhallActive/img/vhallactive/speak_hover.png);}\
QPushButton:pressed{border-image:url(:/vhallActive/img/vhallactive/speak_click.png);}"

#define USER_KICKOUT_STATE_CSS "QPushButton{border-image:url(:/vhallActive/img/vhallactive/off_speak.png);}\
QPushButton:hover{border-image:url(:/vhallActive/img/vhallactive/off_speak_hover.png);}\
QPushButton:pressed{border-image:url(:/vhallActive/img/vhallactive/off_speak_click.png);}"


class MemberCtrlMaskWdg : public QWidget
{
	Q_OBJECT

public:
	MemberCtrlMaskWdg(QWidget *parent = Q_NULLPTR);
	~MemberCtrlMaskWdg();

	void SetCurrentUserRole(int state);
	//�Ƿ���ʾ�����水����
	void ShowMainView(bool show);
	//�Ƿ���ʾ����Ϊ������
	void ShowMainSpeaker(bool show);
	//�����Ƿ���ʾ����
	void ShowToKickOutRoom(bool show);
	//������˷�״̬
	void SetMicState(bool open);
	//��������ͷ״̬
	void SetCameraState(bool open);
	//����������
	void SetSpeakState(bool kickout);
   //�����л�
   void ShowSwitchLayoutBtn(bool show);

   void HideDeviceCtrl(bool hide);
public slots:
	void hide();
private slots:
	void slot_OnClickedCamera();
	void slot_OnClickedMic();
	void slot_OnClickedSetMainView();
	void slot_OnClickedKickOffRoom();
	void slot_OnClickedSetToSpeaker();

   void slot_OnClickedCamerafocusIn();
   void slot_OnClickedSetMainViewfocusIn();
   void slot_OnClickedMicfocusIn();
   void slot_OnClickedSetToSpeakerfocusIn();
   void slot_OnClickedKickOffRoomfocusIn();

   void slot_OnClickedCamerafocusOut();
   void slot_OnClickedSetMainViewfocusOut();
   void slot_OnClickedMicfocusOut();
   void slot_OnClickedSetToSpeakerfocusOut();
   void slot_OnClickedKickOffRoomfocusOut();
   void slot_OnClickedSwitchView();

protected:
	virtual void leaveEvent(QEvent *event);
	virtual void focusOutEvent(QFocusEvent *event);
	bool eventFilter(QObject *obj, QEvent *ev);


	void OnEnteCamera();
	void OnEnteMainView();
	void OnEnteMic();
	void OnEnteToSpeaker();
	void OnEnteKickOffRoom();
signals:
	void sig_clickedCamera(bool);
	void sig_clickedMic(bool);
	void sig_clickedSetMainView(); //����Ϊ������
	void sig_clickedKickOffRoom(); //����
	void sig_clickedSetToSpeaker();//��Ϊ������
   void sig_leave();
   void sig_switchView();

private:
	Ui::MemberCtrlMaskWdg ui; 
	bool mbEnableCtrl = false;

	bool mbMicOpen = false;
	bool mbCameraOpen = false;
	CToolTips* mpTips = NULL;
};
