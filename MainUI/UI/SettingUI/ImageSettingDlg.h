#ifndef IMAGESETTINGDLG_H
#define IMAGESETTINGDLG_H

#include "ui_ImageSettingDlg.h"
#include "VhallViewWidget.h"
#include "VHDialog.h"
#include <QLabel>
class TitleWidget;
class QHBoxLayout;
class ImageSettingDlg : public VHDialog {
   Q_OBJECT

public:
   ImageSettingDlg(QDialog *parent = 0);
   ~ImageSettingDlg();

   bool Create();
   void Destroy();

   //���õ�ǰ���� (//1������� 2�����޸�)
   void SetCurType(int iType);
   void SetBasePixmap(QPixmap &);
   void SetCerterImg(int ,int,int,int,int ,int ,unsigned char **,int baseWidth,int baseHeight);
   void SetImage(float ,float ,float ,float ,QString);

   void CenterWindow(QWidget* parent);
private slots:
   void OnSave();
   void OnClose();
   void slotPos(QString);
   void on_btnChangeImage_clicked();

protected:
   void paintEvent(QPaintEvent *);

   //void mergeFormat(QTextCharFormat);

private:
   Ui::ImageSettingDlg ui;
   TitleWidget* m_pTitleBar;
   QHBoxLayout* m_pMainLayout;
   VHallViewWidget *m_pViewWidget = NULL;
   int m_iCurType;         //1������� 2�����޸�
};

#endif // TEXTSETTINGDLG_H
