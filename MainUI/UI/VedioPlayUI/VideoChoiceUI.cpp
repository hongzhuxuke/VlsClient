#include "stdafx.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QPen>
#include <QColor>
#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QListWidget>
#include <QToolButton>
#include <vector>
#include "WebRtcSDKInterface.h"
#include <QDebug>
#include <QFileInfo>
#include <QFileDialog>
#include "IMainUILogic.h"
#include "TitleWidget.h"
#include "push_button.h"
#include "VedioPlayUI.h"
#include "VideoChoiceUI.h"
#include "AlertTipsDlg.h"
#include "ToolButton.h"
#include "Push_button.h"
#include "ICommonData.h"
#include "Msg_OBSControl.h"
#include "IOBSControlLogic.h"
#include "pathManager.h"
#include <QSpacerItem>

using namespace std;

#define ADD_FILE   QString::fromWCharArray(L"QPushButton{background-color:rgb(50, 50, 50);\
border:1px solid rgb(214, 214, 214);\
border-radius:12px;\
color:rgb(214, 214, 214);\
font: 9pt \"微软雅黑\";}\
QPushButton:hover{background-color:rgb(255, 255, 255); border:1px solid rgb(255, 255, 255);border-radius:12px; color:rgb(50, 50, 50); font:9pt \"微软雅黑\"; }")

VideoChoiceItem::VideoChoiceItem(QWidget *parent) :
   QWidget(parent) {

   this->setFixedHeight(40);
   this->setMaximumWidth(380);
   this->setMinimumWidth(380);

   QHBoxLayout *layout = new QHBoxLayout(this);
   this->setLayout(layout);
   layout->setContentsMargins(0, 0, 0, 0);

   m_pLabel = new QLabel(this);
   if (m_pLabel) {
      layout->addWidget(m_pLabel);
   }

   m_pCloseButton = new PushButton(this);
   if (m_pCloseButton) {
      m_pCloseButton->setFixedSize(20, 20);
      m_pCloseButton->loadPixmap(":/sysButton/deleteItem");
      layout->addWidget(m_pCloseButton);
      connect(m_pCloseButton, SIGNAL(clicked()), this, SLOT(closeBtnClicked()));
   }
   setStyleSheet(TOOLTIPQSS);
}

VideoChoiceItem::~VideoChoiceItem() {
}

void VideoChoiceItem::closeBtnClicked() {
   emit this->closeClicked(this);

   QJsonObject body;
   SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_MultiMedia_Close, L"MultiMedia_Close", body);
}

void VideoChoiceItem::setFileName(const QString &text) {
   m_qText = text;
   if (!m_pLabel) {
      return;
   }

   QFileInfo textInfo;
   textInfo.setFile(text);
   QString tStr = textInfo.fileName();
   QString str = m_pLabel->fontMetrics().elidedText(tStr, Qt::ElideRight, 360);
   m_pLabel->setText(str);
   this->setToolTip(tStr);
}
QString VideoChoiceItem::getFilename() {
   return m_qText;
}
void VideoChoiceItem::setEndLine(bool ok) {
   m_bDrawLine = ok;
}
void VideoChoiceItem::FlushLabelColor(bool pressed) {
   if (pressed) {
      if (m_bChecked) {
         m_pLabel->setStyleSheet(
            "QLabel{color:rgb(252,86,89);}"
         );
      }
      else {
         m_pLabel->setStyleSheet("QLabel{color:rgb(70,70,70);}");
      }

   }
   else {
      if (m_bChecked) {
         m_pLabel->setStyleSheet(
            "QLabel{color:rgb(255,51,51);}"
            "QLabel:hover{color:rgb(255,100,100);}"
         );
      }
      else {
         m_pLabel->setStyleSheet(
            "QLabel{color:rgb(141,141,141);}"
            "QLabel:hover{color:rgb(200,200,200);}"
         );
      }
   }
}

void VideoChoiceItem::setChecked(bool ok) {
   m_bChecked = ok;
   FlushLabelColor(false);
}
void VideoChoiceItem::leaveEvent(QEvent *e) {
   QWidget::leaveEvent(e);
   FlushLabelColor(false);
}

void VideoChoiceItem::mouseReleaseEvent(QMouseEvent *e) {
   QWidget::mouseReleaseEvent(e);
   FlushLabelColor(false);
}

void VideoChoiceItem::mousePressEvent(QMouseEvent *e) {
   QWidget::mousePressEvent(e);
   FlushLabelColor(true);
}

//void VideoChoiceItem::paintEvent(QPaintEvent *e) {
//   QWidget::paintEvent(e);
//   QPainter p(this);
//   if (m_bDrawLine) {
//      QPen pen;
//      p.setPen(QColor(59, 59, 59));
//      p.drawLine(0, height() - 1, width(), height() - 1);
//   }
//}

void VideoChoiceItem::setWidth(int width) {
   this->setFixedWidth(width);
}

VideoChoiceUI::VideoChoiceUI(QDialog *parent) :
   VHDialog(parent) {
   this->setFixedSize(412, 360);
   this->setWindowTitle(QString::fromWCharArray(L"插播文件"));
   QString icon = QApplication::applicationDirPath() + "/vhall.ico";
   icon = icon.replace("/","\\");
   this->setWindowIcon(QIcon(icon));
}

VideoChoiceUI::~VideoChoiceUI() {
   if (mPlayOutTipsLable) {
      mPlayOutTipsLable->removeEventFilter(this);
   }
}

void VideoChoiceUI::SetCircleMode(int mode) {
   if (!m_pCircleCombox) {
      return;
   }

   if (mode >= CycleTyle_None && mode < CycleTyle_END) {

      for (int i = 0; i < m_pCircleCombox->count(); i++) {
         QVariant data = m_pCircleCombox->itemData(i);
         if (data.toInt() == mode) {
            m_pCircleCombox->setCurrentIndex(i);
            break;
         }
      }
   }

}

int VideoChoiceUI::getCircleMode() {
   if (!m_pCircleCombox) {
      return 0;
   }

   int mode = 0;
   bool ok = false;
   int currentMode = m_pCircleCombox->currentData().toInt(&ok);
   if (ok) {
      if (currentMode >= CycleTyle_None && currentMode < CycleTyle_END) {

         mode = currentMode;
      }
   }
   return mode;
}

bool VideoChoiceUI::Create() {
   m_pTitleBar = new TitleWidget(QString::fromWCharArray(L"插播文件"), NULL);
   m_pTitleBar->setMinimumHeight(40);
   m_pTitleBar->setMaximumHeight(40);
   connect(m_pTitleBar, SIGNAL(closeWidget()), this, SLOT(close()));
   m_pTitleBar->setFocusPolicy(Qt::NoFocus);

   QVBoxLayout *layout = new QVBoxLayout();

   //title
   layout->setContentsMargins(0, 0, 0, 0);
   layout->addWidget(m_pTitleBar, 0, Qt::AlignTop);

   QVBoxLayout *centerLayout = new QVBoxLayout();
   centerLayout->setContentsMargins(0, 0, 0, 0);
   centerLayout->setSpacing(0);

   //addButton   circle
   QWidget *topWidget = new QWidget();
   QVBoxLayout *addlayOut = new QVBoxLayout();
   QHBoxLayout *addLayoutTop = new QHBoxLayout();
   topWidget->setLayout(addlayOut);
   topWidget->setMinimumHeight(77);
   topWidget->setMaximumHeight(77);
   addlayOut->setContentsMargins(20, 11, 15, 0);


   topWidget->setStyleSheet("QWidget{background-color:rgb(50,50,50);}");
   QPushButton *addButton = new QPushButton(this);
   addButton->setStyleSheet("QPushButton:hover{background-color:rgb(214, 214, 214);border-radius:12px;color: rgb(32, 32, 32);font: 9pt;}QPushButton{ background-color:rgb(50, 50, 50); border: 1px solid rgb(214, 214, 214); border-radius: 12px; color: rgb(208, 202, 203);font: 9pt;}");
   addButton->setText(QString::fromWCharArray(L"插播文件"));
   connect(addButton, SIGNAL(clicked()), this, SLOT(ChoiceFiles()));
   addButton->setFixedSize(70, 24);


   QLabel *circleTipLabel = new QLabel(QString::fromWCharArray(L"循环方式:"));
   circleTipLabel->setStyleSheet(QString::fromWCharArray(L"QLabel{color:#D6D6D6;font: 9pt \"微软雅黑\"}"));
   circleTipLabel->setFocusPolicy(Qt::NoFocus);
   m_pCircleCombox = new QComboBox(this);
   m_pCircleCombox->setMaximumSize(100, 28);
   m_pCircleCombox->setMinimumSize(100, 28);
   m_pCircleCombox->addItem(QString::fromWCharArray(L"顺序播放"), QVariant((int)CycleTyle_None));
   m_pCircleCombox->addItem(QString::fromWCharArray(L"列表循环"), QVariant((int)CycleTyle_List));
   m_pCircleCombox->addItem(QString::fromWCharArray(L"单视频循环"), QVariant((int)CycleTyle_Single));
   m_pCircleCombox->setView(new QListView());
   m_pCircleCombox->setFocusPolicy(Qt::NoFocus);

   addLayoutTop->addWidget(addButton);
   addLayoutTop->addSpacing(170);
   addLayoutTop->addWidget(circleTipLabel);
   addLayoutTop->addWidget(m_pCircleCombox);
   addlayOut->addLayout(addLayoutTop);

   //label
   //QLabel *tipLabel = new QLabel(QString::fromWCharArray(L"支持的视频格式：MP4、RMVB、FLV、MOV"));
   mFileTypeTips = new QLabel(QString::fromWCharArray(L"支持格式：MP3 WAV MP4 RMVB FLV MOV，最大支持1080P视频"));
   mFileTypeTips->setStyleSheet(QString::fromWCharArray(L"QLabel{font: 75 9pt \"微软雅黑\";color:#9D9D9D;}"));

   addlayOut->addWidget(mFileTypeTips);
   mFileTypeTips->setFocusPolicy(Qt::NoFocus);
   centerLayout->addWidget(topWidget);

   //list
   QWidget *centerWidget = new QWidget();
   centerWidget->setStyleSheet("QWidget{background-color:rgb(38,38,38);}");
   //centerWidget->setStyleSheet("background-color:red;");
   QVBoxLayout *centerListLayout = new QVBoxLayout();
   centerWidget->setLayout(centerListLayout);
   centerListLayout->setContentsMargins(20, 0, 2, 0);


   m_pListWidget = new QListWidget(this);
   m_pListWidget->setFrameShape(QListWidget::NoFrame);
   m_pListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   m_pListWidget->setFocusPolicy(Qt::StrongFocus);
   connect(m_pListWidget, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(PlayListDoubleClicked(const QModelIndex &)));
   connect(m_pListWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(currentItemChanged(QListWidgetItem*, QListWidgetItem*)));

   centerListLayout->addWidget(m_pListWidget);

   centerLayout->addWidget(centerWidget);
   layout->addLayout(centerLayout);

   m_pListWidget->setDragEnabled(true);
   m_pListWidget->setDragDropMode(QAbstractItemView::InternalMove);
   m_pListWidget->setDefaultDropAction(Qt::CopyAction);

   QWidget *bottomWidget = new QWidget();
   bottomWidget->setMinimumHeight(80);
   bottomWidget->setMaximumHeight(80);
   //sure button
   QVBoxLayout *buttomLayout = new QVBoxLayout();
   bottomWidget->setLayout(buttomLayout);
   buttomLayout->setSpacing(0);

   mpPlayOutCheckBox = new QCheckBox(this);
   mpPlayOutCheckBox->setObjectName(QStringLiteral("mpPlayOutCheckBox"));
   mpPlayOutCheckBox->setText(QString::fromWCharArray(L"插播外放声音"));

   buttomLayout->setContentsMargins(0, 0, 0, 12);
   bottomWidget->setStyleSheet("QWidget{background-color:rgb(50,50,50);border-bottom-left-radius:4px;border-bottom-right-radius:4px;}");
   m_pSureButton = new QPushButton(this);
   m_pSureButton->setStyleSheet("QPushButton:hover{background-color:rgb(255, 51, 51);border-radius:12px;color:rgb(255, 235, 235);font: 9pt;}QPushButton{ background-color: rgba(255, 45, 52, 51); border: 1px solid rgb(255, 51, 51); border-radius:12px; color:rgb(208, 202, 203);font: 9pt; }");
   m_pSureButton->setFixedSize(70, 24);
   m_pSureButton->setText(QString::fromWCharArray(L"立即播放"));

   connect(m_pSureButton, SIGNAL(clicked()), this, SLOT(SureClicked()));
   QLabel *bottonTipLabel = new QLabel(this);
   QHBoxLayout *bottomTipLabelLayout = new QHBoxLayout();
   bottomTipLabelLayout->setContentsMargins(20, 0, 0, 0);
   bottomTipLabelLayout->addWidget(bottonTipLabel);
   buttomLayout->addLayout(bottomTipLabelLayout);

   QWidget *sureButtonWidget = new QWidget();
   sureButtonWidget->setMinimumHeight(30);
   sureButtonWidget->setMaximumHeight(30);

   mPlayOutTipsLable = new QLabel(this);
   mPlayOutTipsLable->setObjectName(QStringLiteral("tips"));
   mPlayOutTipsLable->setMinimumSize(QSize(14, 14));
   mPlayOutTipsLable->setMaximumSize(QSize(14, 14));
   mPlayOutTipsLable->setStyleSheet(QStringLiteral("border-image: url(:/sysButton/img/sysButton/noiseTips.png)"));

   mPlayOutTipsLable->installEventFilter(this);

   QSpacerItem *verticalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
   QHBoxLayout *sureButtonLayout = new QHBoxLayout();
   sureButtonWidget->setLayout(sureButtonLayout);
   sureButtonLayout->setContentsMargins(20, 0, 20, 0);

   sureButtonLayout->addWidget(mpPlayOutCheckBox);
   sureButtonLayout->addWidget(mPlayOutTipsLable);
   sureButtonLayout->addItem(verticalSpacer);
   sureButtonLayout->addWidget(m_pSureButton);
   buttomLayout->addWidget(sureButtonWidget);


   m_rightTipLabel = bottonTipLabel;
   bottonTipLabel->setStyleSheet(QString::fromWCharArray(L"QLabel{font: 100  9pt \"微软雅黑\";color:rgb(216, 49, 49);}"));

   layout->addWidget(bottomWidget);
   layout->setSpacing(0);
   setLayout(layout);

   setAutoFillBackground(true);
   m_pListWidget->setStyleSheet(
      "QListWidget::item {color: rgb(153, 153, 153);background-color: rgb(38,38,38);border-width: 0px 0px 0px 0px;border-radius: 0px;}"
      "QListWidget::item:focus {color: rgb(153, 153, 153);background-color: rgb(38,38,38);}"
      "QListWidget::item:hover {color: rgb(153, 153, 153);background-color: rgb(38,38,38);}"
      "QListWidget::item:focus"
      "{"
      "color: rgb(153, 153, 153);border-image:null;"
      "}"
      "QListWidget::item:hover {"
      "color: rgb(153, 153, 153);border-image:null;"
      "}"
   );


   this->setStyleSheet(
      "QListWidget{background-color:rgb(50,50,50);outline:0px;}"
      "QScrollBar:vertical"
      "{"
      "width:4px;"
      "margin:0px,0px,0px,0px;"
      "}"
      "QScrollBar::handle:vertical"
      "{"
      "width:4px;"
      "background:rgb(205,205,205);"
      "border-radius:2px;"
      "min-height:20;"
      "}"
      "QScrollBar::handle:vertical:hover"
      "{"
      "width:4px;"
      "background:rgb(180,180,180);"
      "border-radius:2px;"
      "min-height:20;"
      "}"
      "QScrollBar::add-line:vertical"
      "{"
      "height:0px;"
      "width:4px;"
      "subcontrol-position:bottom;"
      "}"
      "QScrollBar::sub-line:vertical"
      "{"
      "height:0px;"
      "width:4px;"
      "subcontrol-position:top;"
      "}"
      "QScrollBar::sub-page:vertical {"
      "background: rgb(38,38,38);"
      "}"
      "QScrollBar::add-page:vertical {"
      "background: rgb(38,38,38);"
      "}"
   );

   //addButton->setStyleSheet(ADD_FILE);
   //m_pSureButton->setStyleSheet(ADD_FILE);

   connect(m_pCircleCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCircleModeChanged(int)));
   if (mpPlayTipsWdg == NULL) {
      mpPlayTipsWdg = new NoiseTooltip(this);
      mpPlayTipsWdg->setFixedHeight(100);
   }
   return true;
}

void VideoChoiceUI::Destroy() {

}

void VideoChoiceUI::paintEvent(QPaintEvent *) {
   QPainter painter(this);

   //画背景
   //painter.setRenderHint(QPainter::Antialiasing, true);
   //painter.setPen(QPen(QColor(54, 54, 54), 1));
   //painter.setBrush(QColor(50, 50, 50));
   //painter.drawRoundedRect(rect(), 4.0, 4.0);

   FlushItems();
}
void VideoChoiceUI::showEvent(QShowEvent *e) {
   QDialog::showEvent(e);
   if (m_pListWidget) {
      m_pListWidget->setFocus();
   }
}

void VideoChoiceUI::AppendFileList(const QStringList &fileList, int currentIndex) {
   bool isMoreThanMaxFileCount = false;
   if (!m_pListWidget) {
      return;
   }
   m_bPaintItem = false;

   //judge file equal
   auto IsFileInList = [&, this](const QString &fullFilename) {
      for (int index = 0; index < m_pListWidget->count(); index++) {
         QListWidgetItem *item = m_pListWidget->item(index);
         if (item) {
            QString currFile;
            QListWidgetItem *item = m_pListWidget->item(index);
            if (item) {
               QWidget *w = m_pListWidget->itemWidget(item);
               if (w) {
                  VideoChoiceItem *itemWidget = dynamic_cast<VideoChoiceItem *>(w);
                  if (itemWidget) {
                     currFile = itemWidget->getFilename();
                  }
               }
            }
            if (currFile.isEmpty()) {
               continue;
            }

            if (currFile.toLower() == fullFilename.toLower()) {
               return true;
            }

            QFileInfo currFileInfo;
            currFileInfo.setFile(currFile);
            QFileInfo inputFileInfo;
            inputFileInfo.setFile(fullFilename);
            if (currFileInfo == inputFileInfo && currFileInfo.filePath() == inputFileInfo.filePath()) {
               return true;
            }
         }
      }
      return false;
   };

   auto InsertFileList = [&](const QString &filename) {
      if (IsFileInList(filename) || filename.isEmpty()) {
         return false;
      }

      //check max count
      if (m_pListWidget->count() >= MAXFILECOUNT) {
         isMoreThanMaxFileCount = true;
         return false;
      }

      VideoChoiceItem *itemWidget = new VideoChoiceItem();
      itemWidget->setFileName(filename);
      //itemWidget->setWidth(382);
      connect(itemWidget, SIGNAL(closeClicked(VideoChoiceItem *)), this, SLOT(DeleteItem(VideoChoiceItem *)));
      QListWidgetItem *item = new QListWidgetItem();
      m_pListWidget->insertItem(m_pListWidget->count(), item);
      item->setSizeHint(itemWidget->size());
      m_pListWidget->setItemWidget(item, itemWidget);
      return true;
   };

   int width = 0;
   int height = 0;
   QString strList;
   for_each(fileList.begin(), fileList.end(), [&](const QString &filename) {
      VH::CComPtr<ICommonData> pCommonData;
      DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
      ClientApiInitResp initResp;
      pCommonData->GetInitApiResp(initResp);
      if (initResp.selectLiveType == eLiveType_Live) {
         VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
         DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
         if (pObsControlLogic) {
            pObsControlLogic->GetMediaFileWidthAndHeight(filename.toStdString().c_str(), width, height);
         }
      }
      else {
         int nRet = vlive::GetWebRtcSDKInstance()->GetPlayMeidaFileWidthAndHeight(filename.toStdString(), width, height);
      }


      if (initResp.selectLiveType == eLiveType_VhallActive) {
         if (filename.contains(".wav") || filename.contains(".WAV") || filename.contains(".mp3") || filename.contains(".MP3")) {
            return;
         }
         //互动无效
         if (width <= 1280 && height <= 720) {
            InsertFileList(filename);
            strList += "/*/" + filename;
         }
      }
      else {
         InsertFileList(filename);
         strList += "/*/" + filename;
      }
   });

   QJsonObject body;
   body["vin"] = strList;
   SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_MultiMedia_InsertVedioList, L"InsertVedioList", body);

   if (currentIndex >= 0 && currentIndex < m_pListWidget->count()) {
      m_pListWidget->setCurrentRow(currentIndex);
   }
   else if(m_pListWidget->count() > 0){
      m_pListWidget->setCurrentRow(0);
   }

   m_bPaintItem = true;
   FlashTips();
   repaint();
}

void VideoChoiceUI::PlayListDoubleClicked(const QModelIndex &index) {
   qDebug() << "########################VideoChoiceUI::PlayListDoubleClicked####################";
   if (!m_pListWidget) {
      return;
   }

   int row = index.row();
   if (row < 0 || row >= m_pListWidget->count()) {
      qDebug() << "########################VideoChoiceUI::PlayListDoubleClicked#################### row < 0 || row >= m_pListWidget->count()";
      return;
   }

   m_pListWidget->setCurrentRow(row);
   accept();
}
void VideoChoiceUI::SureClicked() {
   QJsonObject body;
   QString strMadieName = m_pCircleCombox->currentText();
   body["vpo"] = QString::fromUtf8(strMadieName.toUtf8());
   SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_MultiMedia_StartPlay, L"MultiMedia_StartPlay", body);
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   ClientApiInitResp initResp;
   pCommonData->GetInitApiResp(initResp);
   if (initResp.selectLiveType == eLiveType_Live) {
      //仅用于提示
      wchar_t *msg = L"视频播放中，麦克风已关闭";
      SingletonMainUIIns::Instance().PostCRMessage(MSG_OBSCONTROL_STREAM_NOTIFY, msg, (wcslen(msg) + 1) * sizeof(wchar_t));
   }
   accept();
   pCommonData->ReportEvent("crsp01");
}

void VideoChoiceUI::ChoiceFiles() {
   if (!m_pListWidget) {
      return;
   }
   QStringList fileNameList;
   QString selectFileName;
   TCHAR szPathName[2048];
   OPENFILENAME ofn = { sizeof(OPENFILENAME) };
   ofn.hwndOwner = NULL;// 打开OR保存文件对话框的父窗口
   if (mLiveType == eLiveType_Live) {
      ofn.lpstrFilter = TEXT("文件(*.mp3 *.wav *.mp4 *.flv *.mov *.rmvb)\0*.mp3;*.wav;*.mp4;*.flv;*.mov;*.rmvb\0\0");
   }
   else if (mLiveType == eLiveType_VhallActive) {
      ofn.lpstrFilter = TEXT("文件(*.mp4 *.flv *.mov *.rmvb)\0*.mp4;*.flv;*.mov;*.rmvb\0\0");
   }
   lstrcpy(szPathName, TEXT(""));
   ofn.lpstrFile = szPathName;
   ofn.nMaxFile = sizeof(szPathName);//存放用户选择文件的 路径及文件名 缓冲区
   ofn.lpstrTitle = TEXT("选择文件");//选择文件对话框标题
   ofn.Flags = OFN_EXPLORER;//如果需要选择多个文件 则必须带有  OFN_ALLOWMULTISELECT标志
   BOOL bOk = GetOpenFileName(&ofn);//调用对话框打开文件
   TRACE6("%s GetOpenFileName\n", __FUNCTION__);
   if (bOk) {
      selectFileName = QString::fromStdWString(szPathName);
      fileNameList.push_back(selectFileName);
   }

   if (fileNameList.isEmpty()) {
      return;
   }
   int width = 0;
   int height = 0;
   for (int i = 0; i < fileNameList.size(); i++) {
      QString fileName = fileNameList.at(i);
      if (!fileName.contains(".mp3") && !fileName.contains(".MP3") && !fileName.contains(".WAV") && !fileName.contains(".wav")) {
         if (mLiveType == eLiveType_Live) {
            VH::CComPtr<IOBSControlLogic> pObsControlLogic = NULL;
            DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_IOBSControl, IID_IOBSControlLogic, pObsControlLogic, return, ASSERT(FALSE));
            if (pObsControlLogic) {
               pObsControlLogic->GetMediaFileWidthAndHeight(fileName.toStdString().c_str(), width, height);
            }
         }
         else {
            int nRet = vlive::GetWebRtcSDKInstance()->GetPlayMeidaFileWidthAndHeight(fileName.toStdString(), width, height);
         }

         //产品确定最大支持视频分辨率为1080P;
         if (mLiveType == eLiveType_Live && width > 1920 && height > 1080) {
            RightTip(QString::fromWCharArray(L"插播视频分辨率最大支持1080P"));
            return;
         }
         else if (mLiveType == eLiveType_VhallActive && width > 1280 && height > 720){
            RightTip(QString::fromWCharArray(L"插播视频分辨率最大支持720P"));
            return;
         }
      }
   }
   int lastFileNameListCount = m_pListWidget->count();
   AppendFileList(fileNameList, -1);
   if (m_pListWidget->count() > lastFileNameListCount) {
      m_pListWidget->setCurrentRow(lastFileNameListCount);
   }
}

void VideoChoiceUI::currentItemChanged(QListWidgetItem*, QListWidgetItem*) {
   for (int i = 0; i < m_pListWidget->count(); i++) {
      QListWidgetItem *item = m_pListWidget->item(i);
      if (item) {
         QWidget *w = m_pListWidget->itemWidget(item);
         if (w) {
            VideoChoiceItem *videoWidget = dynamic_cast<VideoChoiceItem *>(w);
            if (videoWidget) {
               videoWidget->setChecked(i == m_pListWidget->currentRow());
            }
         }
      }
   }

}

void VideoChoiceUI::DeleteItem(VideoChoiceItem *itemWidget) {
   if (!m_pListWidget) {
      return;
   }

   if (!itemWidget) {
      return;
   }

   for (int i = 0; i < m_pListWidget->count(); i++) {
      QListWidgetItem *item = m_pListWidget->item(i);
      if (item) {
         if (m_pListWidget->itemWidget(item) == itemWidget) {
            QString str = itemWidget->getFilename();
            QJsonObject body;
            body["vin"] = str;
            SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_MultiMedia_DelVedio, L"MultiMedia_DelVedio", body);
            m_pListWidget->removeItemWidget(item);
            delete item;
            break;
         }
      }
   }

   FlashTips();
   m_pListWidget->setFocus();
}

void VideoChoiceUI::Clear() {
   if (!m_pListWidget) {
      return;
   }

   m_pListWidget->clear();
}

QStringList VideoChoiceUI::GetPlayList(int &currentIndex) {
   QStringList playList;
   if (m_pListWidget) {
      for (int i = 0; i < m_pListWidget->count(); i++) {
         QListWidgetItem *item = m_pListWidget->item(i);
         if (item) {
            QWidget *w = m_pListWidget->itemWidget(item);
            if (w) {
               VideoChoiceItem *itemWidget = dynamic_cast<VideoChoiceItem *>(w);
               if (itemWidget) {
                  QString fileName = itemWidget->getFilename();
                  if (!fileName.isEmpty()) {
                     playList.append(fileName);
                  }
               }
            }
         }
      }

      currentIndex = m_pListWidget->currentRow();
   }

   return playList;
}
void VideoChoiceUI::CenterWindow(QWidget* parent) {
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
void VideoChoiceUI::RightTip(const QString &tip) {
   if (!m_rightTipLabel) {
      return;
   }

   m_rightTipLabel->setText(tip);
}
void VideoChoiceUI::FlashTips() {

   if (!m_pListWidget) {
      return;
   }

   if (!m_pSureButton) {
      return;
   }

   if (m_pListWidget->count() == 0) {
      RightTip(QString::fromWCharArray(L"请添加要插播的视频"));
      m_pSureButton->setEnabled(false);
   }
   else if (m_pListWidget->count() >= MAXFILECOUNT) {
      RightTip(QString::fromWCharArray(L"最多添加10个视频"));
      m_pSureButton->setEnabled(true);
   }
   else {
      RightTip("");
      m_pSureButton->setEnabled(true);
   }
}
void VideoChoiceUI::FlushItems() {
   if (!m_bPaintItem) {
      return;
   }
   if (m_pListWidget) {
      for (int i = 0; i < m_pListWidget->count(); i++) {
         QListWidgetItem *item = m_pListWidget->item(i);
         if (item) {
            QWidget *w = m_pListWidget->itemWidget(item);
            if (w) {
               VideoChoiceItem *itemWidget = dynamic_cast<VideoChoiceItem *>(w);
               if (itemWidget) {
                  itemWidget->setEndLine(i != (m_pListWidget->count() - 1));
               }
            }
         }
      }
   }

   //FLushFileList();
}
void VideoChoiceUI::FLushFileList(bool accept) {
   int currentIndex;
   QStringList fileList = GetPlayList(currentIndex);
   if (!accept) {
      currentIndex = -1;
   }
   emit this->SigSyncFileList(fileList, currentIndex);
}

void VideoChoiceUI::OnCircleModeChanged(int index) {
   if (-1 == index) {
      return;
   }

   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->ReportEvent("crsp03");
}

bool VideoChoiceUI::IsEnablePlayOutAudio() {
   return mpPlayOutCheckBox->checkState() > 0 ? true : false;
}

void VideoChoiceUI::SetCurrentPlayOutState(bool enable) {
   if (enable) {
      mpPlayOutCheckBox->setCheckState(Qt::Checked);
   }
   else {
      mpPlayOutCheckBox->setCheckState(Qt::Unchecked);
   }
}
void VideoChoiceUI::HidePlayOutCheck(bool hide) {
   if (hide) {
      mpPlayOutCheckBox->hide();
      if (mPlayOutTipsLable) {
         mPlayOutTipsLable->hide();
      }
   }
   else {
      mpPlayOutCheckBox->show();
      if (mPlayOutTipsLable) {
         mPlayOutTipsLable->show();
      }
   }
}

void VideoChoiceUI::SetLiveType(int type) {
   mLiveType = type;
   if (mLiveType == eLiveType_Live) {
      mFileTypeTips->setText(QString::fromWCharArray(L"支持格式：MP3 WAV MP4 RMVB FLV MOV，最大支持1080P视频"));
   }
   else  if (mLiveType == eLiveType_VhallActive) {
      mFileTypeTips->setText(QString::fromWCharArray(L"支持格式：MP4 RMVB FLV MOV，视频最大支持720P视频"));
   }
   else {
      mFileTypeTips->setText(QString::fromWCharArray(L"支持格式：MP4 RMVB FLV MOV，视频最大支持480P视频"));
   }
}

bool VideoChoiceUI::eventFilter(QObject *obj, QEvent *ev) {
   if (ev) {
      if (ev->type() == QEvent::Enter) {
         if (obj == mPlayOutTipsLable && mpPlayTipsWdg != NULL) {
            mpPlayTipsWdg->setText(QString::fromWCharArray(L"勾选后，插播视频时本地会播放视频声音不勾选，插播视频时本地不会播放视频声音"));
            mpPlayTipsWdg->show();
            QPoint pos = this->mapToParent(mpPlayOutCheckBox->pos());
            mpPlayTipsWdg->move(pos + QPoint(-30, 330));
         }
      }
      if (ev->type() == QEvent::Leave) {
         if (obj == mPlayOutTipsLable && mpPlayTipsWdg != NULL) {
            mpPlayTipsWdg->close();
         }
      }
   }
   return QWidget::eventFilter(obj, ev);
}
