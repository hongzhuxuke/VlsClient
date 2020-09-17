#ifndef TEXTSETTINGDLG_H
#define TEXTSETTINGDLG_H

#include "ui_TextSettingUI.h"
#include "VhallViewWidget.h"
#include "VHDialog.h"
class TitleWidget;
class QHBoxLayout;
class STRU_OBSCONTROL_TEXT;
class TextSettingDlg : public VHDialog {
   Q_OBJECT

public:
   TextSettingDlg(QDialog *parent = 0);
   ~TextSettingDlg();

   bool Create();
   void Destroy();

   //�����ı���ʽ
   void SetTextFormat(STRU_OBSCONTROL_TEXT* pTextFormat);
   void SetTextRect(float,float,float,float);

   //���õ�ǰ���� (//1������� 2�����޸�)
   void SetCurType(int iType);
   void SetBasePixmap(QPixmap &);
   void SetCerterImg(int ,int,int,int,int ,int ,unsigned char **,int baseWidth,int baseHeight);

   void CenterWindow(QWidget* parent);

   void show();

   
private:
   void InitTextFormat();
   void FlushTextFormat();
private slots:
   void slotPos(QString);
   void slotFont(QString);
   void slotBold();
   void slotItalic();
   void slotUnder();
   void slotColor();
   void OnSave();
   void textEditLimitInput(const QString &);
   void OnClose();
public slots:
   void TextRectChanged(float ,float ,float ,float);
protected:
   virtual void paintEvent(QPaintEvent *);
private:
   QString m_qText;
   Ui::TextSettingDlg ui;
   TitleWidget* m_pTitleBar;
   QHBoxLayout* m_pMainLayout;
   QTextCharFormat m_qTextCharFmt;
   VHallViewWidget *m_pViewWidget;
   int m_iCurType;         //1������� 2�����޸�
   float m_renderW;
   float m_renderH;   
};

#endif // TEXTSETTINGDLG_H
