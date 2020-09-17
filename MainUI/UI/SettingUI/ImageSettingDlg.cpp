#include "stdafx.h"
#include <QPainter>
#include <QBoxLayout>
#include <QColorDialog>
#include <QFileDialog>
#include <QDesktopWidget>
#include "ConfigSetting.h"
#include "TitleWidget.h"
#include "ImageSettingDlg.h"
#include "Msg_OBSControl.h"

#include "ICommonData.h"


#define TEXTEDIT_BYTE_LIMIT 30

#define MAKERGB(r,g,b)      ((((DWORD)r << 16)|((DWORD)g << 8)|(DWORD)b))
#define REVERSE_COLOR(col)  MAKERGB(RGB_B(col), RGB_G(col), RGB_R(col))

ImageSettingDlg::ImageSettingDlg(QDialog *parent)
: VHDialog(parent)
, m_pTitleBar(NULL)
, m_pMainLayout(NULL)
, m_iCurType(1) {
   ui.setupUi(this);
   setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
   setAttribute(Qt::WA_TranslucentBackground);
   setAutoFillBackground(true);
}

ImageSettingDlg::~ImageSettingDlg() {
}

bool ImageSettingDlg::Create() {
   m_pTitleBar = new TitleWidget(tr("添加图片"), this);
   if (NULL == m_pTitleBar) {
      ASSERT(FALSE);
      return false;
   }
   ui.gridLayoutTitle->addWidget(m_pTitleBar);
   connect(m_pTitleBar, SIGNAL(closeWidget()), this, SLOT(OnClose()));
   connect(ui.saveBtn, SIGNAL(clicked()), this, SLOT(OnSave()));
   connect(ui.comboBoxPos,SIGNAL(activated(QString)),this,SLOT(slotPos(QString)));
   m_pViewWidget = new VHallViewWidget(this);
   m_pViewWidget->SetMaxRenderRect(QRect(0,0,400,275));
   m_pViewWidget->SetViewMode(VHALLVIEWWIDGETVIEWMODE_PIC);

   //m_pViewWidget->show();
   ui.gridLayoutCenter->addWidget(m_pViewWidget);
   
   return true;
}

void ImageSettingDlg::Destroy() {
   if (NULL != m_pMainLayout) {
      delete m_pMainLayout;
      m_pMainLayout = NULL;
   }

   if (NULL != m_pTitleBar) {
      delete m_pTitleBar;
      m_pTitleBar = NULL;
   }
}

void ImageSettingDlg::SetCurType(int iType) {
   m_iCurType = iType;
   QString title;
   if(m_iCurType==1){
      title = ADD_IMAGE;
   }
   else{
      title = MODIFY_IMAGE;
   }
   m_pTitleBar->SetTitle(title);
   
}
void ImageSettingDlg::SetBasePixmap(QPixmap &pixmap)
{
   if(m_pViewWidget)
   {
      m_pViewWidget->SetBasePixmap(pixmap);
   }
}
void ImageSettingDlg::SetCerterImg(int x,int y,int w,int h,int imgWidth,int imgHeight,unsigned char **data,int baseWidth,int baseHeight)
{
   if(m_pViewWidget)
   {
      m_pViewWidget->SetCerterPixmap(x,y,w,h,imgWidth,imgHeight,data,baseWidth,baseHeight);
   }
}
void ImageSettingDlg::SetImage(float x,float y,float w,float h,QString filename)
{
   if(m_pViewWidget)
   {
      if(x==0&&y==0&&w==0&&h==0)
      {
         m_pViewWidget->SetImageDrawMode(VIEWWIDGETPOS_RD);
         m_pViewWidget->PaintImage(x,y,w,h,filename);
         ui.comboBoxPos->setCurrentIndex(5);
      }
      else
      {
         m_pViewWidget->SetImageDrawMode(VIEWWIDGETPOS_CO);
         m_pViewWidget->PaintImage(x,y,w,h,filename);
      }
   }
}
void ImageSettingDlg::slotPos(QString posStr)
{
   m_pViewWidget->SetImageDrawMode(posStr);
}

void ImageSettingDlg::on_btnChangeImage_clicked()
{
   VH::CComPtr<IMainUILogic> pMainUILogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return, ASSERT(FALSE));

   VH::CComPtr<IVedioPlayLogic> pVedioPlayLogic;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IMainUI, IID_IVedioPlayLogic, pVedioPlayLogic, return);

   QWidget* pMainUI = NULL;
   pMainUILogic->GetMainUIWidget((void**)&pMainUI);
   
   
   this->hide();
   //QString fileName = QFileDialog::getOpenFileName(QWidget::find((WId)pMainUI->winId()), QObject::tr("打开文件"), "", QObject::tr("图像文件(*.jpg *.png *.bmp);;Allfile(*.*)"), 0, 0);
   QString selectFileName;// = QFileDialog::getOpenFileName(NULL, QStringLiteral("打开文件"), "", QStringLiteral("文件(*.jpg *.JPG *.png *.PNG);"), 0, 0);
   TCHAR szPathName[2048];
   // lStructSize
   // 指定这个结构的大小，以字节为单位。
   // Windows 95/98和Windows NT 4.0：特意为Windows 95/98或Windows NT 4.0，及带有WINVER和_WIN32_WINNT >= 0x0500编译时，
   //  为这个成员使用OPENFILENAME_SIZE_VERSION_400。
   // Windows 2000及更高版本：这个参数使用sizeof (OPENFILENAME)。
   OPENFILENAME ofn = { sizeof(OPENFILENAME) };
   ofn.hwndOwner = NULL;// 打开OR保存文件对话框的父窗口
   //过滤器 如果为 NULL 不使用过滤器
   //具体用法看上面  注意 /0
   lstrcpy(szPathName, TEXT(""));
   ofn.lpstrFilter = TEXT("图像文件(*.jpg *.png *.bmp)\0*.jpg;*.png;*.bmp\0\0");
   ofn.lpstrFile = szPathName;
   ofn.nMaxFile = sizeof(szPathName);//存放用户选择文件的 路径及文件名 缓冲区
   ofn.lpstrTitle = TEXT("选择文件");//选择文件对话框标题
   ofn.Flags = OFN_EXPLORER;//如果需要选择多个文件 则必须带有  OFN_ALLOWMULTISELECT标志
   BOOL bOk = GetOpenFileName(&ofn);//调用对话框打开文件
   TRACE6("%s GetOpenFileName\n", __FUNCTION__);
   if (bOk) {
      selectFileName = QString::fromStdWString(szPathName);

   }
   this->show();
   
   if(selectFileName.isNull()){
      return ;
   }
   
   if(m_pViewWidget){
      m_pViewWidget->ResetImageFile(selectFileName);
   }
}

void ImageSettingDlg::paintEvent(QPaintEvent *) {
   QPainter painter(this);
   painter.setRenderHint(QPainter::Antialiasing, true);
   painter.setPen(QPen(QColor(54, 54, 54), 1));
   painter.setBrush(QColor(38, 38, 38));
   painter.drawRoundedRect(rect(), 4.0, 4.0);
}

void ImageSettingDlg::OnSave() {

   if(m_pViewWidget)
   {
      float x,y,w,h;
      QString fileName;
      QString imgPos;
      m_pViewWidget->GetImageInfo(x,y,w,h,fileName,imgPos);
      STRU_OBSCONTROL_IMAGE ImagePath;
      ImagePath.m_dwType = m_iCurType;
      wcscpy(ImagePath.m_strPath, fileName.toStdWString().c_str());
      ImagePath.x=x;
      ImagePath.y=y;
      ImagePath.w=w;
      ImagePath.h=h;
      ImagePath.isFullScreen = VIEWWIDGETPOS_FU == imgPos;
      
      SingletonMainUIIns::Instance().PostCRMessage(
      MSG_OBSCONTROL_IMAGE, &ImagePath, sizeof(STRU_OBSCONTROL_IMAGE));

		QJsonObject body;
		body["ix"] = x;
		body["iy"] = y;
		body["iw"] = w;
		body["ih"] = h;
		body["in"] = fileName;

		SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_MultiMedia_InsertImage_Ok, L"InsertImage_Ok", body);

   }
      
   hide();

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->ReportEvent("crtp01");
}
void ImageSettingDlg::OnClose() {
   hide();
}

void ImageSettingDlg::CenterWindow(QWidget* parent) {
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
