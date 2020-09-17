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

   //设置当前类型 (//1代表添加 2代表修改)
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
   int m_iCurType;         //1代表添加 2代表修改
};

#endif // TEXTSETTINGDLG_H
