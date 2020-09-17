#include <windows.h>
#include "vhallrightextrawidget.h"
#include "VhallRightExtraWidgetIns.h"
#include "Msg_VhallRightExtraWidget.h"
#include <QDebug>
#include "IMainUILogic.h"
#include "DebugTrace.h"

VhallRightExtraWidget::VhallRightExtraWidget(QWidget *parent) :
    QWidget(parent),
    m_bSharink(false) {
    ui.setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::Tool);
    m_btnChange = new ToolButton(":/extern/07.png",this);
    connect(m_btnChange, SIGNAL(sigClicked()), this, SLOT(EnterChange()));
    ui.gridLayoutChange->addWidget(m_btnChange);
    ui.widget->installEventFilter(this);
    m_btnChange->installEventFilter(this);  
    
    this->setWindowTitle(QString::fromWCharArray(L"聊天"));
}

VhallRightExtraWidget::~VhallRightExtraWidget() {

}

bool VhallRightExtraWidget::nativeEvent(const QByteArray &eventType, void *message, long *result){
   MSG* msg = reinterpret_cast<MSG*>(message);
   
   if(WM_SETCURSOR == msg->message) {
      MouseMovedExtraWidget();
   }

   if(WM_KEYDOWN == msg->message) {
      if(msg->wParam==9) {
         emit this->SigTabClicked();
      }
   }
   
   return QWidget::nativeEvent(eventType, message, result);
}
void VhallRightExtraWidget::leaveEvent(QEvent *e) {   
   QWidget::leaveEvent(e);
   MouseMovedExtraWidget();
}
bool VhallRightExtraWidget::eventFilter(QObject *o, QEvent *e){
   if(o==ui.widget||o==m_btnChange) 
   {
      QWidget *parent = (QWidget*)(this->parent());
      if (e->type() == QEvent::MouseButtonPress&&m_bDesktopMode && parent) {
         this->pressPoint = parent->cursor().pos();
         this->startPoint = parent->pos();
      }
      else if (e->type() == QEvent::MouseMove&&m_bDesktopMode && parent) {
         int dx = parent->cursor().pos().x() - this->pressPoint.x();
         int dy = parent->cursor().pos().y() - this->pressPoint.y();
          int x = this->startPoint.x()+dx;
          int y = this->startPoint.y()+dy;
          
          int aScreenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
          int aScreenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    
          if(x<=0){
            x=0;
          }
          else if(x+this->width()>aScreenWidth) {
            x=aScreenWidth-this->width();
          }
          

          if(y<=0) {
            y=0;
          }
          else if(y+this->height()>aScreenHeight) {
            y=aScreenHeight-this->height();
          }


          aScreenWidth;aScreenHeight;
          parent->move(x, y);
          if(menuWidget) {
               menuWidget->hide();
          }
      }

   }
   return QWidget::eventFilter(o,e);
}

void VhallRightExtraWidget::AppendWidget(void *w,wchar_t *name) {
   QWidget *qw=static_cast<QWidget *>(w);
   if(qw) {
      qw->setObjectName(QString::fromWCharArray(name));
      ui.stackedWidget->addWidget(qw);

      if(QString::fromWCharArray(name) == QString::fromWCharArray(L"观众")) {
         m_pOnlineListWidget = qw;
         qDebug()<<"VhallRightExtraWidget::AppendWidget m_pOnlineListWidget";
      }
   }
}
void VhallRightExtraWidget::RemoveWidget(void *w) {
   QWidget *qw=static_cast<QWidget *>(w);
   if(qw) {
      ui.stackedWidget->removeWidget(qw);
   }
}
bool VhallRightExtraWidget::IsCurrentWidget(void *w) {
   QWidget *qw=static_cast<QWidget *>(w);
   if(!qw) {
      return false;
      ui.stackedWidget->removeWidget(qw);
   }

   return ui.stackedWidget->currentWidget() == qw;
}

void VhallRightExtraWidget::MouseMovedExtraWidget() {

}

bool VhallRightExtraWidget::ISShrink() {
   return m_bSharink;
}

void VhallRightExtraWidget::SetShrink(bool sharink) {
   m_bSharink=sharink;
   if(m_bSharink) {
      hide();
   }
   else {
      show();
   }
}

QStringList VhallRightExtraWidget::GetTabsObjectNames() {
   QStringList l;
   for(int i=0;i<ui.stackedWidget->count();i++) {
      if(i!=ui.stackedWidget->currentIndex()) {
         QWidget *w=ui.stackedWidget->widget(i);
         if(w) {
            l.append(w->objectName());
         }
      }
   }
   return l;
}

void VhallRightExtraWidget::EnterChange() {   
   emit this->SigEnterBtn();
}

void VhallRightExtraWidget::on_stackedWidget_currentChanged(int) {
   bool toClear=true;
   if(ui.stackedWidget->count() > 0) {
      QWidget *w=ui.stackedWidget->currentWidget();
      if(w) {
         ui.label->setText(w->objectName());
         toClear=false;
      }
   }
   
   if(toClear) {
      ui.label->clear();
   }
}
void VhallRightExtraWidget::SetPage(QString obj){
   for(int i=0;i<ui.stackedWidget->count();i++) {
      QWidget *w = ui.stackedWidget->widget(i);
      if(w) {
         if(w->objectName()==obj) {
            ui.stackedWidget->setCurrentWidget(w);  
            break;
         }
      }
   }
}
void VhallRightExtraWidget::on_pushButton_test_clicked(){
   qDebug()<<"VhallRightExtraWidget::on_pushButton_test_clicked";
   emit SigClickedTest();
}
void VhallRightExtraWidget::SetDesktopMode(bool ok){

   VH::CComPtr<IMainUILogic> pMainUILogic = NULL;
   DEF_GET_INTERFACE_PTR(SingletonVhallRightExtraWidgetIns, PID_IMainUI, IID_IMainUILogic, pMainUILogic, return);
   m_bDesktopMode = ok;
   TRACE6(" %s : mode: %d", __FUNCTION__, m_bDesktopMode);
   if (ok) {//点击聊天
      QWidget *parent = NULL;

	   if (this->parentWidget() != NULL) {
		   pMainUILogic->RemoveExtraWidget();
         parent = (QWidget*)(this->parent());
	   }
	   else
	   {
		   TRACE6(" %s : parentWidget: NULL", __FUNCTION__);
	   }

	   if (parent && parent->isHidden()) {//当前聊天界面未显示，则显示
		   RECT rt;
		   SystemParametersInfo(SPI_GETWORKAREA, 0, &rt, 0);
		   int x = rt.right - this->width() - 40;
		   int y = rt.bottom - this->height() - 40;
		   parent->show();
		   parent->move(x, y);
		   this->show();
		   this->move(0, 0);
		   TRACE6(" %s : Show,move (%d, %d)", __FUNCTION__, x, y);
	   }
	   else if (parent && !parent->isHidden()) {//当前聊天界面显示，则隐藏
		   parent->hide();
		   TRACE6(" %s : Hide", __FUNCTION__);
	   }
	   else
	   {
		   TRACE6(" %s : parent is NULL", __FUNCTION__);
	   }

   }
   else {
	   pMainUILogic->AddRightExWidget(this, true);
   }
}

void VhallRightExtraWidget::SetMenuWidget(QWidget *w){
   this->menuWidget = w;
}
void VhallRightExtraWidget::SetSuspensionEnable(bool ok){
   if(ok) {
      m_btnChange->show();
   }
   else {
      m_btnChange->hide();
   }
}
void * VhallRightExtraWidget::GatherOnlineList(){
   RemoveWidget(m_pOnlineListWidget);
   return m_pOnlineListWidget;
}
void VhallRightExtraWidget::RecoveryOnlineList(void *w){
   if(w == m_pOnlineListWidget && w != NULL) {
      AppendWidget(w,L"观众");
   }
}

