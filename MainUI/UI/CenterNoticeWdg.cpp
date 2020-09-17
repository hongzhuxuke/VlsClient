#include "CenterNoticeWdg.h"
#include "priConst.h"

CCenterNoticeWdg::CCenterNoticeWdg(QWidget *parent)
    : QWidget(parent)
{
   ui.setupUi(this);
   EnableMouseTracking();
}

CCenterNoticeWdg::~CCenterNoticeWdg()
{
   DisableMouseTracking();
}

void CCenterNoticeWdg::SetNoticeType(int type) {
   QPixmap pixmap;
   switch (type) {
   case RenderType_Wait:
      pixmap = QPixmap(":/interactivity/renderWait");
      ui.labRenderType->setPixmap(pixmap);
      ui.labRenderType->setMinimumSize(pixmap.size());
      ui.labRenderType->setMaximumSize(pixmap.size());
      ui.labRenderTypeText->setText(QString::fromWCharArray(L"请等待主持人开始直播"));
      ui.labRenderTypeText->show();
      break;
   case RenderType_Finished:
      pixmap = QPixmap(":/interactivity/renderFinished");
      ui.labRenderType->setPixmap(pixmap);
      ui.labRenderType->setMinimumSize(pixmap.size());
      ui.labRenderType->setMaximumSize(pixmap.size());
      ui.labRenderTypeText->setText(QString::fromWCharArray(L"直播已结束"));
      ui.labRenderTypeText->show();
      break;
   case RenderType_MasterExit:
      pixmap = QPixmap(":/interactivity/renderMasterExit");
      ui.labRenderType->setPixmap(pixmap);
      ui.labRenderType->setMinimumSize(pixmap.size());
      ui.labRenderType->setMaximumSize(pixmap.size());
      ui.labRenderTypeText->setText(QString::fromWCharArray(L"主持人已退出直播"));
      ui.labRenderTypeText->show();
      break;
   case RenderType_Screen:
      pixmap = QPixmap(":/interactivity/renderDesktop");
      ui.labRenderType->setPixmap(pixmap);
      ui.labRenderType->setMinimumSize(pixmap.size());
      ui.labRenderType->setMaximumSize(pixmap.size());
      ui.labRenderTypeText->setText(QString::fromWCharArray(L"您正在桌面共享,建议将本界面最小化"));
      ui.labRenderTypeText->show();
      break;
   case RenderType_Photo:
      pixmap = m_PhotoPixmap;
      ui.labRenderType->setPixmap(pixmap);
      ui.labRenderType->setMinimumSize(pixmap.size());
      ui.labRenderType->setMaximumSize(pixmap.size());
      ui.labRenderTypeText->hide();
      break;
   default:
      break;
   }
}

void CCenterNoticeWdg::SetPhotoPixmap(const QPixmap& pix) {
   m_PhotoPixmap = pix;
}

void CCenterNoticeWdg::mouseMoveEvent(QMouseEvent *) {
   if (m_bIsFullWnd) {
      emit Sig_MouseMoveInCenterNoticeWdg();
   }
}

void CCenterNoticeWdg::SetIsFullState(bool full) {
   m_bIsFullWnd = full;
   if (m_bIsFullWnd){
      EnableMouseTracking();
   }
   else {
      DisableMouseTracking();
   }
}

void CCenterNoticeWdg::EnableMouseTracking() {
   this->setMouseTracking(true);
   ui.labRenderType->setMouseTracking(true);
   ui.labRenderTypeText->setMouseTracking(true);
   ui.widget->setMouseTracking(true);
   ui.widget_2->setMouseTracking(true);
}

void CCenterNoticeWdg::DisableMouseTracking() {
   this->setMouseTracking(false);
   ui.labRenderType->setMouseTracking(false);
   ui.labRenderTypeText->setMouseTracking(false);
   ui.widget->setMouseTracking(false);
   ui.widget_2->setMouseTracking(false);
}

void CCenterNoticeWdg::contextMenuEvent(QContextMenuEvent *) {
   emit Sig_ContextMenuEvent();
}

void CCenterNoticeWdg::mouseDoubleClickEvent(QMouseEvent *) {
   emit Sig_MouseDoubleClicked();
}
