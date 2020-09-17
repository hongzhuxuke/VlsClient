#include "stdafx.h"
#include <QPainter>
#include <QBoxLayout>
#include <QColorDialog>
#include <QDesktopWidget>
#include <QDebug>
#include <QAbstractItemView> 
#include <QUuid>
#include "TitleWidget.h"
#include "TextSettingDlg.h"
#include "Msg_OBSControl.h"
#include "IOBSControlLogic.h"
#include "ICommonData.h"

#define TEXTEDIT_BYTE_LIMIT 30

#define MAKERGB(r,g,b)      ((((DWORD)r << 16)|((DWORD)g << 8)|(DWORD)b))
#define REVERSE_COLOR(col)  MAKERGB(RGB_B(col), RGB_G(col), RGB_R(col))

TextSettingDlg::TextSettingDlg(QDialog *parent)
   : VHDialog(parent)
   , m_pTitleBar(NULL)
   , m_pMainLayout(NULL)
   , m_iCurType(1)
   , m_pViewWidget(NULL) {
   ui.setupUi(this);
   setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
   setAttribute(Qt::WA_TranslucentBackground);
   setAutoFillBackground(true);
   m_renderW = 0;
   m_renderH = 0;
}

TextSettingDlg::~TextSettingDlg() {
}

bool TextSettingDlg::Create() {
   m_pTitleBar = new TitleWidget(tr("插入文字"), this);
   if (NULL == m_pTitleBar) {
      ASSERT(FALSE);
      return false;
   }
   ui.gridLayoutTitle->addWidget(m_pTitleBar);
   connect(m_pTitleBar, SIGNAL(closeWidget()), this, SLOT(OnClose()));
   //font-family lists
   ui.fontText->setFocus();
   ui.fontBox->setFontFilters(QFontComboBox::ScalableFonts);
   ui.boldBtn->setIcon(QIcon(":/sysButton/fontBold"));
   ui.italicBtn->setIcon(QIcon(":/sysButton/fontItalic"));
   ui.underBtn->setIcon(QIcon(":/sysButton/fontUnderline"));
   ui.colorBtn->setIcon(QIcon(":/sysButton/fontColor"));

   m_qTextCharFmt.setFontPointSize(16);
   m_qTextCharFmt.setFontFamily("宋体");
   m_qTextCharFmt.setForeground(QColor(255, 0, 0));

   connect(ui.fontBox, SIGNAL(activated(QString)), this, SLOT(slotFont(QString)));
   connect(ui.boldBtn, SIGNAL(clicked()), this, SLOT(slotBold()));
   connect(ui.italicBtn, SIGNAL(clicked()), this, SLOT(slotItalic()));
   connect(ui.underBtn, SIGNAL(clicked()), this, SLOT(slotUnder()));
   connect(ui.colorBtn, SIGNAL(clicked()), this, SLOT(slotColor()));
   connect(ui.fontText, SIGNAL(textChanged(const QString &)), this, SLOT(textEditLimitInput(const QString &)));
   connect(ui.saveBtn, SIGNAL(clicked()), this, SLOT(OnSave()));
   connect(ui.comboBoxPos, SIGNAL(activated(QString)), this, SLOT(slotPos(QString)));
   m_pViewWidget = new VHallViewWidget(this);
   m_pViewWidget->SetMaxRenderRect(QRect(0, 0, 400, 275));
   m_pViewWidget->SetViewMode(VHALLVIEWWIDGETVIEWMODE_TEXT);
   connect(m_pViewWidget, SIGNAL(TextRectChanged(float, float, float, float)), this, SLOT(TextRectChanged(float, float, float, float)));
   //m_pViewWidget->show();
   ui.gridLayoutCenter->addWidget(m_pViewWidget);
   return true;
}

void TextSettingDlg::Destroy() {
   if (NULL != m_pMainLayout) {
      delete m_pMainLayout;
      m_pMainLayout = NULL;
   }

   if (NULL != m_pTitleBar) {
      delete m_pTitleBar;
      m_pTitleBar = NULL;
   }
}

void TextSettingDlg::show() {
   QSet<QString> defaultString;
   defaultString.insert(QString::fromWCharArray(L"宋体"));
   defaultString.insert(QString::fromWCharArray(L"仿宋"));
   defaultString.insert(QString::fromWCharArray(L"黑体"));
   defaultString.insert(QString::fromWCharArray(L"微软雅黑"));
   defaultString.insert(QString::fromWCharArray(L"新宋体"));
   defaultString.insert(QString::fromWCharArray(L"楷体"));
   defaultString.insert(QString::fromWCharArray(L"Arial"));
   defaultString.insert(QString::fromWCharArray(L"Arial Rounded MT"));
   for (int i = 0; i < ui.fontBox->count(); i++) {
      QString fontStr = ui.fontBox->itemText(i);
      if (defaultString.find(fontStr) == defaultString.end()) {
         ui.fontBox->removeItem(i);
         i--;
      }
   }

   QDialog::show();
   int viewWidth = ui.fontBox->width();
   int viewHeight = ui.fontBox->count() * 18;
   ui.fontBox->view()->window()->setMaximumSize(viewWidth, viewHeight);
   ui.fontBox->view()->window()->setMinimumSize(viewWidth, viewHeight);
   ui.fontBox->view()->window()->resize(viewWidth, viewHeight);
}

void TextSettingDlg::SetTextFormat(STRU_OBSCONTROL_TEXT* pTextFormat) {
   if (!pTextFormat) {
      ASSERT(FALSE);
      return;
   }
   //修改时，回显原有文字和格式
   ui.fontText->setText(QString::fromUtf16((const ushort*)pTextFormat->m_strText));
   slotFont(QString::fromUtf16((const ushort*)pTextFormat->m_strFont));
   //宽度
   m_qTextCharFmt.setFontWeight(pTextFormat->m_iBold ? QFont::Bold : QFont::Normal);
   //加粗
   m_qTextCharFmt.setFontItalic(pTextFormat->m_iItalic);
   //下划线
   m_qTextCharFmt.setFontUnderline(pTextFormat->m_iUnderLine);
   QColor color;
   color.setRgb(QRgb(pTextFormat->m_iColor));
   //颜色
   m_qTextCharFmt.setForeground(color);
   FlushTextFormat();
}

void TextSettingDlg::SetTextRect(float x, float y, float w, float h) {
   m_renderW = w;
   m_renderH = h;
   if (x == 0 && y == 0 && w == 0 && h == 0) {
      if (m_pViewWidget) {
         m_pViewWidget->SetTextDrawMode(VIEWWIDGETPOS_LD);
         ui.comboBoxPos->setCurrentIndex(1);
      }
   }
   else {
      if (m_pViewWidget) {
         m_pViewWidget->SetTextDrawMode(VIEWWIDGETPOS_CO);
         m_pViewWidget->SetTextPos(x, y);
      }
   }
}


void TextSettingDlg::SetCurType(int iType) {
   m_iCurType = iType;
   QString title;
   if (m_iCurType == 1) {
      title = QString::fromWCharArray(L"插入文字");
   }
   else {
      title = QString::fromWCharArray(L"修改文字");
   }
   m_pTitleBar->SetTitle(title);

   if (m_iCurType == 1) {
      InitTextFormat();
   }
   FlushTextFormat();
}

void TextSettingDlg::SetBasePixmap(QPixmap &pixmap) {
   if (m_pViewWidget) {
      m_pViewWidget->SetBasePixmap(pixmap);
   }
}

void TextSettingDlg::SetCerterImg(int x, int y, int w, int h, int imgWidth, int imgHeight, unsigned char **data, int baseWidth, int baseHeight) {
   if (m_pViewWidget) {
      m_pViewWidget->SetCerterPixmap(x, y, w, h, imgWidth, imgHeight, data, baseWidth, baseHeight);
   }
}

void TextSettingDlg::slotFont(QString f) {
   m_qTextCharFmt.setFontFamily(f);
   FlushTextFormat();
}

void TextSettingDlg::slotPos(QString posStr) {
   if (m_pViewWidget) {
      m_pViewWidget->SetTextDrawMode(posStr);
   }
}

void TextSettingDlg::slotBold() {
   m_qTextCharFmt.setFontWeight(ui.boldBtn->isChecked() ? QFont::Bold : QFont::Normal);
   FlushTextFormat();
}

void TextSettingDlg::slotItalic() {
   m_qTextCharFmt.setFontItalic(ui.italicBtn->isChecked());
   FlushTextFormat();
}

void TextSettingDlg::slotUnder() {
   m_qTextCharFmt.setFontUnderline(ui.underBtn->isChecked());
   FlushTextFormat();
}

void TextSettingDlg::InitTextFormat() {
   QFont f = ui.fontBox->currentFont();
   f.setFamily("微软雅黑");
   ui.fontBox->setCurrentFont(f);

   slotFont(ui.fontBox->currentFont().toString());
   slotBold();
   slotItalic();
   slotUnder();
   m_qTextCharFmt.setForeground(QColor(Qt::white));
}

void TextSettingDlg::FlushTextFormat() {
   if (m_pViewWidget) {
      QString VIEWWIDGETPOS mode = m_pViewWidget->GetTextDrawMode();
      if (mode == VIEWWIDGETPOS_CO) {
         float x = 0;
         float y = 0;
         m_pViewWidget->GetTextPos(x, y);
         m_pViewWidget->PaintText(x, y, m_renderW, m_renderH, m_qText, m_qTextCharFmt);
      }
      else {
         m_pViewWidget->PaintText(mode, m_renderW, m_renderH, m_qText, m_qTextCharFmt);
      }
   }
}
void TextSettingDlg::TextRectChanged(float x, float y, float w, float h) {
   m_renderW = w;
   m_renderH = h;
}


void TextSettingDlg::slotColor() {
   QColor color = QColorDialog::getColor(Qt::red, this);
   if (color.isValid()) {
      m_qTextCharFmt.setForeground(color);
      FlushTextFormat();
   }
}

void TextSettingDlg::paintEvent(QPaintEvent *) {
   //QPainter painter(this);
   //painter.setRenderHint(QPainter::Antialiasing, true);
   //painter.setPen(QPen(QColor(54, 54, 54), 1));
   //painter.setBrush(QColor(38, 38, 38));
   //painter.drawRoundedRect(rect(), 4.0, 4.0);
}

void TextSettingDlg::OnSave() {
   VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));

   STRU_OBSCONTROL_TEXT textControl;
   textControl.m_iControlType = m_iCurType;
   textControl.m_iColor = m_qTextCharFmt.foreground().color().rgb();
   textControl.m_iBold = m_qTextCharFmt.fontWeight() > 50 ? 1 : 0;
   textControl.m_iItalic = m_qTextCharFmt.fontItalic() ? 1 : 0;
   textControl.m_iUnderLine = m_qTextCharFmt.fontUnderline() ? 1 : 0;
   wcscpy(textControl.m_strFont, m_qTextCharFmt.fontFamily().toStdWString().c_str());
   wcscpy(textControl.m_strText, ui.fontText->text().toStdWString().c_str());
   if (m_qTextCharFmt.fontItalic()) {
      _snwprintf(textControl.m_strText, MAX_FONT_TEXT_LEN, L"%s", textControl.m_strText);
   }
   int w, h;
   float ix, iy, iw, ih;
   QPixmap pixmap = m_pViewWidget->CreateTextPIC(ix, iy, iw, ih);
   if (pixmap.isNull()) {
      hide();
      if (m_iCurType == 2) {
         textControl.m_iControlType = 3;
         SingletonMainUIIns::Instance().PostCRMessage(MSG_OBSCONTROL_TEXT, &textControl, sizeof(STRU_OBSCONTROL_TEXT));
      }
      return;
   }
   QImage img = pixmap.toImage();
   QString fileName = QUuid::createUuid().toString() + ".png";
   QString filePath = QString::fromStdWString(GetAppDataPath()) + QString("image/") + fileName;
   filePath = filePath.replace("\\","/");
   img.save(filePath, "PNG");
   w = img.width();
   h = img.height();
   //unsigned char *data = pObsControlLogic->MemoryCreate(w*h * 4);
   //if (data)
   {
      //memset(data,0, w * h * 4);
      //for (int y = 0; y < h; y++)
      //{
      //   for (int x = 0; x < w; x++)
      //   {
      //      union argbPixel
      //      {
      //         unsigned int pixel;
      //         unsigned char pixels[4];
      //      };
      //      argbPixel pixel;
      //      pixel.pixel = img.pixel(x, y);

      //      //ABGR
      //      unsigned char b = pixel.pixels[0];
      //      unsigned char g = pixel.pixels[1];
      //      unsigned char r = pixel.pixels[2];
      //      unsigned char a = pixel.pixels[3];
      //      //ABGR
      //      *(unsigned int *)(data + y * w * 4 + x * 4) = a << 24 | b << 16 | g << 8 | r;
      //   }
      //}

      textControl.m_w = w;
      textControl.m_h = h;
      memset(textControl.m_textPic, 0, 512);
      filePath.toWCharArray(textControl.m_textPic);
      textControl.m_ix = ix;
      textControl.m_iy = iy;
      textControl.m_iw = iw;
      textControl.m_ih = ih;
      textControl.m_iColor = (int)m_qTextCharFmt.foreground().color().rgb();
      SingletonMainUIIns::Instance().PostCRMessage(MSG_OBSCONTROL_TEXT, &textControl, sizeof(STRU_OBSCONTROL_TEXT));

      QJsonObject body;
      body["ix"] = ix;
      body["iy"] = iy;
      body["iw"] = iw;
      body["ih"] = ih;
      body["tx"] = QString::fromWCharArray(textControl.m_strText);
      body["tfm"] = QString::fromWCharArray(textControl.m_strFont);
      body["tb"] = textControl.m_iBold;
      body["ti"] = textControl.m_iItalic;
      body["tu"] = textControl.m_iUnderLine;
      body["ta"] = QString::number(textControl.m_iColor);
      SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_MultiMedia_InsertText_Ok, L"InsertText_Ok", body);
   }

   ui.fontText->clear();
   hide();

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->ReportEvent("crwz01");
}

void TextSettingDlg::textEditLimitInput(const QString &text) {
   m_qText = text;
   FlushTextFormat();
}

void TextSettingDlg::OnClose() {
   ui.fontText->clear();
   hide();
}

void TextSettingDlg::CenterWindow(QWidget* parent) {
   int x = 0;
   int y = 0;
   if (NULL == parent) {
      const QRect rect = QApplication::desktop()->availableGeometry();
      x = rect.left() + (rect.width() - width()) / 2;
      y = rect.top() + (rect.height() - height()) / 2;
   }
   else {
      QPoint point(0, 0);
      point = parent->mapToGlobal(point);
      x = point.x() + (parent->width() - width()) / 2;
      y = point.y() + (parent->height() - height()) / 2;
   }
   move(x, y);
}
