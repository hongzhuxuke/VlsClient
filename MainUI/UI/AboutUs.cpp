#include "stdafx.h"
#include <QFrame>
#include <QPainter>
#include "TitleWidget.h"
#include "AboutUs.h"
#include "MainUIIns.h"

#include "ICommonData.h"

AboutUsDialog::AboutUsDialog(QWidget *parent) : QDialog(parent) {
   setFixedSize(340, 180);
   setWindowFlags(Qt::Dialog | Qt::Tool | Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint);
   setAttribute(Qt::WA_TranslucentBackground);
   setAutoFillBackground(true);
}

AboutUsDialog::~AboutUsDialog() {

}

void AboutUsDialog::translateLanguage() {
   wstring wzGurVersion;
   PublishInfo loStreamInfo;
   VH::CComPtr<ICommonData> pCommonData;
   DEF_GET_INTERFACE_PTR(SingletonMainUIIns, PID_ICommonToolKit, IID_ICommonData, pCommonData, return, ASSERT(FALSE));
   pCommonData->GetCurVersion(wzGurVersion);

   pCommonData->GetStreamInfo(loStreamInfo);
   QString Ver;
   if (!loStreamInfo.m_bMengZhu) {
      Ver = QString("%1 %2").arg(tr("微吼直播助手： V")).arg(QString::fromStdWString(wzGurVersion));
      m_pCopyZHLabel->setText(tr("微吼直播版权所有"));
   } else {
      Ver = QString("%1 %2").arg(tr("盟主直播助手： V")).arg(QString::fromStdWString(wzGurVersion));
      m_pCopyZHLabel->setText(tr("盟主直播版权所有"));
   }
   
   m_pNameLabel->setText(Ver);
   m_pTelLabel->setText(tr("服务咨询电话： 400-682-6882"));
   m_pCopyENLabel->setText(tr("Copyright @vhall 2015 All Right Reserved."));
}

bool AboutUsDialog::Create() {
   m_pTitleBar = new TitleWidget("关于我们", NULL);
   connect(m_pTitleBar, SIGNAL(closeWidget()), this, SLOT(close()));
   m_pNameLabel = new QLabel();
   m_pTelLabel = new QLabel();
   m_pCopyZHLabel = new QLabel();
   m_pCopyENLabel = new QLabel();
   m_pNameLabel->setAlignment(Qt::AlignHCenter);
   m_pTelLabel->setAlignment(Qt::AlignHCenter);
   m_pCopyZHLabel->setAlignment(Qt::AlignHCenter);
   m_pCopyENLabel->setAlignment(Qt::AlignHCenter);
   translateLanguage();

   QVBoxLayout *pMainLayout = new QVBoxLayout(this);
   pMainLayout->addWidget(m_pTitleBar, 0, Qt::AlignTop);
   pMainLayout->setContentsMargins(0, 0, 0, 0);

   QVBoxLayout *pCenterLayout = new QVBoxLayout(this);
   pCenterLayout->addWidget(m_pNameLabel);
   pCenterLayout->addSpacing(12);
   pCenterLayout->addWidget(m_pTelLabel);
   pCenterLayout->addSpacing(26);
   pCenterLayout->addWidget(m_pCopyZHLabel);
   pCenterLayout->addSpacing(8);
   pCenterLayout->addWidget(m_pCopyENLabel);
   pCenterLayout->setContentsMargins(0, 0, 0, 20);
   pMainLayout->addLayout(pCenterLayout);
   setLayout(pMainLayout);

   return true;
}

void AboutUsDialog::Destroy() {

   if (NULL != m_pTitleBar) {
      delete m_pTitleBar;
      m_pTitleBar = NULL;
   }

   if (NULL != m_pNameLabel) {
      delete m_pNameLabel;
      m_pNameLabel = NULL;
   }

   if (NULL != m_pTelLabel) {
      delete m_pTelLabel;
      m_pTelLabel = NULL;
   }

   if (NULL != m_pCopyZHLabel) {
      delete m_pCopyZHLabel;
      m_pCopyZHLabel = NULL;
   }

   if (NULL != m_pCopyENLabel) {
      delete m_pCopyENLabel;
      m_pCopyENLabel = NULL;
   }
}

void AboutUsDialog::paintEvent(QPaintEvent *) {
   QPainter painter(this);

   //画背景
   painter.setRenderHint(QPainter::Antialiasing, true);
   painter.setPen(QPen(QColor(54, 54, 54), 1));
   painter.setBrush(QColor(38, 38, 38));
   painter.drawRoundedRect(rect(), 4.0, 4.0);

   //画分隔线
   painter.setPen(QPen(QColor(17, 17, 17), 1));
   painter.drawLine(2, 108, 338, 108);
   painter.setPen(QPen(QColor(78, 78, 78), 1));
   painter.drawLine(2, 109, 338, 109);
}