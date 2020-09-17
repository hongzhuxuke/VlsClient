#include "vhareamark.h"
#include "ui_vhareamark.h"
#include <QBitmap>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>
#include <windows.h>
#include <QPushButton>

#include "pub.Const.h"
#include "pub.Struct.h"
#include "MainUIIns.h"

#define RES_TOP_LEAVE_NAME  ":/MonitorAreaCapture/TOP_HOVER"
#define RES_TOP_ENTER_NAME  ":/MonitorAreaCapture/TOP"
#define RES_TOP_LEAVE         QString("background-image:url(%1);").arg(RES_TOP_LEAVE_NAME)
#define RES_TOP_ENTER         QString("background-image:url(%1);").arg(RES_TOP_ENTER_NAME)

#define BTN_WIDTH 4

#define RES_LEFT_TOP_LEAVE_NAME  ":/MonitorAreaCapture/LEFT_TOP_HOVER"
#define RES_LEFT_TOP_ENTER_NAME  ":/MonitorAreaCapture/LEFT_TOP"
#define RES_LEFT_TOP_LEAVE          QString("background-image:url(%1);").arg(RES_LEFT_TOP_LEAVE_NAME)
#define RES_LEFT_TOP_ENTER          QString("background-image:url(%1);").arg(RES_LEFT_TOP_ENTER_NAME)

#define RES_RIGHT_TOP_LEAVE_NAME  ":/MonitorAreaCapture/RIGHT_TOP_HOVER"
#define RES_RIGHT_TOP_ENTER_NAME  ":/MonitorAreaCapture/RIGHT_TOP"
#define RES_RIGHT_TOP_LEAVE         QString("background-image:url(%1);").arg(RES_RIGHT_TOP_LEAVE_NAME)
#define RES_RIGHT_TOP_ENTER         QString("background-image:url(%1);").arg(RES_RIGHT_TOP_ENTER_NAME)

#define RES_LEFT_BOTTOM_LEAVE_NAME  ":/MonitorAreaCapture/LEFT_BOTTOM_HOVER"
#define RES_LEFT_BOTTOM_ENTER_NAME  ":/MonitorAreaCapture/LEFT_BOTTOM"
#define RES_LEFT_BOTTOM_LEAVE       QString("background-image:url(%1);").arg(RES_LEFT_BOTTOM_LEAVE_NAME)
#define RES_LEFT_BOTTOM_ENTER       QString("background-image:url(%1);").arg(RES_LEFT_BOTTOM_ENTER_NAME)

#define RES_RIGHT_BOTTOM_LEAVE_NAME  ":/MonitorAreaCapture/RIGHT_BOTTOM_HOVER"
#define RES_RIGHT_BOTTOM_ENTER_NAME  ":/MonitorAreaCapture/RIGHT_BOTTOM"
#define RES_RIGHT_BOTTOM_LEAVE      QString("background-image:url(%1);").arg(RES_RIGHT_BOTTOM_LEAVE_NAME)
#define RES_RIGHT_BOTTOM_ENTER      QString("background-image:url(%1);").arg(RES_RIGHT_BOTTOM_ENTER_NAME)

#define LINE_WIDTH   2
#define RGB_LINE_ENTER QString("background-color:rgb(255,51,51)")
#define RGB_LINE_LEAVE QString("background-color:rgb(230,42,52)")

VHAreaMark::VHAreaMark(QWidget *parent) :
   QWidget(parent),
   ui(new Ui::VHAreaMark),
    mCloseBtn(nullptr){
   
   ui->setupUi(this);


   QPixmap pixmap;
   int minHeight=0;
   int minWidth=0;

   this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint|Qt::Tool);
   this->setAttribute(Qt::WA_TranslucentBackground);

   ui->widget_left->installEventFilter(this);
   ui->widget_left_top->installEventFilter(this);
   ui->widget_top->installEventFilter(this);
   ui->widget_right_top->installEventFilter(this);
   ui->widget_right->installEventFilter(this);
   ui->widget_right_bottom->installEventFilter(this);
   ui->widget_bottom->installEventFilter(this);
   ui->widget_left_bottom->installEventFilter(this);
   ui->widget_top_center_top_center->installEventFilter(this);
   ui->widget_top_center_top_left->installEventFilter(this);
   ui->widget_top_center_top_right->installEventFilter(this);

   ui->widget_bottom->setCursor(Qt::SizeVerCursor);
   ui->widget_top_center_top_left->setCursor(Qt::SizeVerCursor);
   ui->widget_top_center_top_right->setCursor(Qt::SizeVerCursor);
   ui->widget_left->setCursor(Qt::SizeHorCursor);
   ui->widget_right->setCursor(Qt::SizeHorCursor);
   ui->widget_left_top->setCursor(Qt::SizeFDiagCursor);
   ui->widget_left_bottom->setCursor(Qt::SizeBDiagCursor);
   ui->widget_right_top->setCursor(Qt::SizeBDiagCursor);
   ui->widget_right_bottom->setCursor(Qt::SizeFDiagCursor);
   ui->widget_top_center_top_center->setCursor(Qt::SizeAllCursor);

   pixmap.load(RES_TOP_LEAVE_NAME);
   ui->widget_top_center_top_center->setMinimumSize(pixmap.size());
   ui->widget_top_center_top_center->setMaximumSize(pixmap.size());
   ui->widget_top_center_top_center->setStyleSheet(RES_TOP_LEAVE);

   ui->layout_top_center_top_left->setContentsMargins(0,0,0,LINE_WIDTH);
   ui->layout_top_center_top_right->setContentsMargins(0,0,0,LINE_WIDTH);
   
   minHeight += pixmap.height();
   minWidth += pixmap.width();

   pixmap.load(RES_LEFT_TOP_LEAVE_NAME);
   ui->widget_left_top->setMinimumSize(pixmap.size());
   ui->widget_left_top->setMaximumSize(pixmap.size());
   ui->widget_left_top->setStyleSheet(RES_LEFT_TOP_LEAVE);
   ui->layout_left_top->setContentsMargins(BTN_WIDTH,BTN_WIDTH,0,0);

   ui->widget_left->setMinimumWidth(pixmap.width());
   ui->widget_left->setMaximumWidth(pixmap.width());
   ui->widget_left_mask_left->setMinimumWidth(BTN_WIDTH-LINE_WIDTH);
   ui->widget_left_mask_left->setMaximumWidth(BTN_WIDTH-LINE_WIDTH);
   ui->layout_left->setHorizontalSpacing(LINE_WIDTH);
   
   minWidth += pixmap.width();

   pixmap.load(RES_RIGHT_TOP_LEAVE_NAME);
   ui->widget_right_top->setMinimumSize(pixmap.size());
   ui->widget_right_top->setMaximumSize(pixmap.size());
   ui->widget_right_top->setStyleSheet(RES_RIGHT_TOP_LEAVE);
   ui->layout_right_top->setContentsMargins(0,BTN_WIDTH,BTN_WIDTH,0);

   ui->widget_right->setMinimumWidth(pixmap.width());
   ui->widget_right->setMaximumWidth(pixmap.width());
   ui->widget_right_mask_right->setMinimumWidth(BTN_WIDTH-LINE_WIDTH);
   ui->widget_right_mask_right->setMaximumWidth(BTN_WIDTH-LINE_WIDTH);
   ui->layout_right->setHorizontalSpacing(LINE_WIDTH);

   ui->widget_top_center_bottom->setMinimumHeight(pixmap.height()-BTN_WIDTH);
   ui->widget_top_center_bottom->setMaximumHeight(pixmap.height()-BTN_WIDTH);
   
   minWidth += pixmap.width();
   minHeight += (pixmap.height()-BTN_WIDTH);

   pixmap.load(RES_LEFT_BOTTOM_LEAVE_NAME);
   ui->widget_left_bottom->setMinimumSize(pixmap.size());
   ui->widget_left_bottom->setMaximumSize(pixmap.size());
   ui->widget_left_bottom->setStyleSheet(RES_LEFT_BOTTOM_LEAVE);
   ui->layout_left_bottom->setContentsMargins(BTN_WIDTH,0,0,BTN_WIDTH);

   pixmap.load(RES_RIGHT_BOTTOM_LEAVE_NAME);
   ui->widget_right_bottom->setMinimumSize(pixmap.size());
   ui->widget_right_bottom->setMaximumSize(pixmap.size());
   ui->widget_right_bottom->setStyleSheet(RES_RIGHT_BOTTOM_LEAVE);
   ui->layout_right_bottom->setContentsMargins(0,0,BTN_WIDTH,BTN_WIDTH);

   ui->widget_bottom->setMinimumHeight(pixmap.height());
   ui->widget_bottom->setMaximumHeight(pixmap.height());
   ui->widget_bottom_mask_bottom->setMinimumHeight(BTN_WIDTH-LINE_WIDTH);
   ui->widget_bottom_mask_bottom->setMaximumHeight(BTN_WIDTH-LINE_WIDTH);
   ui->layout_bottom->setVerticalSpacing(LINE_WIDTH);
   
   minHeight += (pixmap.height());

   ui->widget_left->setStyleSheet(RGB_LINE_LEAVE);
   ui->widget_top_center_top_left->setStyleSheet(RGB_LINE_LEAVE);
   ui->widget_top_center_top_right->setStyleSheet(RGB_LINE_LEAVE);
   ui->widget_right->setStyleSheet(RGB_LINE_LEAVE);
   ui->widget_bottom->setStyleSheet(RGB_LINE_LEAVE);


   mCloseBtn = new AreaCloseWdg(this);
   if (mCloseBtn) {
       mCloseBtn->hide();
       mCloseBtn->setFixedWidth(63);
       mCloseBtn->setFixedHeight(23);  
       connect(mCloseBtn, SIGNAL(sig_clicked()), this, SLOT(slot_closeAreaShare()));
   }

   this->setMinimumSize(minWidth,minHeight);
   this->repaint();
}

void VHAreaMark::slot_closeAreaShare() {
    emit SigCloseAreaShare();
}

void VHAreaMark::ReGeoMetry(int x,int y,int w,int h,bool notOnlyResize){
   if(x<0) {
      x=0;
   }

   if(y<0) {
      y=0;
   }
 //  
	//QJsonObject body;
	//body["ix"] = x;
	//body["iy"] = y;
	//body["iw"] = w;
	//body["ih"] = h;
	//SingletonMainUIIns::Instance().reportLog(STR_VH_LIVE_OPERATE, eLogRePortK_RegionShare_Change, L"RegionShare_Change", body);

   bool bOnlyResize=false;
   if(w<this->minimumWidth()) {
      w=this->minimumWidth();
      bOnlyResize=true;
   }
   
   if(w>qApp->desktop()->width()) {
      w=qApp->desktop()->width();
      bOnlyResize=true;
   }
   
   if(h>qApp->desktop()->height()) {
      h=qApp->desktop()->height();
      bOnlyResize=true;
   }
   
   if(h<this->minimumHeight()){
      h=this->minimumHeight();
      bOnlyResize=true;
   }

   if(x>qApp->desktop()->width()-w) {
      x=qApp->desktop()->width()-w;
   }
   
   if(y>qApp->desktop()->height()-h) {
      y=qApp->desktop()->height()-h;
   }
   ;
   if (bOnlyResize&&!notOnlyResize) {
      this->resize(w,h);
   }
   else {
      
      this->setGeometry(x,y,w,h);
   }

   this->ToMask();
   ::SetWindowPos((HWND)this->winId(),HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
}

void VHAreaMark::ShowCloseBtn() {
    if (mCloseBtn) {
        QPoint pos = ui->widget_right_top->mapToGlobal(QPoint(0, 0));
        pos = pos - QPoint(37, mCloseBtn->height() + 10);
        mCloseBtn->move(pos);
        mCloseBtn->show();
    }
}

void VHAreaMark::showEvent(QShowEvent *e) {
    QWidget::showEvent(e);
    this->ToMask();
    ShowCloseBtn();
}

void VHAreaMark::hideEvent(QHideEvent *e){
    if (mCloseBtn) {
        mCloseBtn->hide();
    }
    QWidget::hideEvent(e);
}

void VHAreaMark::resizeEvent(QResizeEvent *e) {
   QWidget::resizeEvent(e);
   this->ToMask();
}

void VHAreaMark::paintEvent(QPaintEvent *) {
   QPixmap pixmap=QPixmap(width(),height());
   pixmap.fill(QColor(0,0,0,1));
   QPainter painter(this);
   painter.drawPixmap(rect(), pixmap);
}

bool VHAreaMark::eventFilter(QObject *obj, QEvent *e) {
    if (e->type() != QEvent::Hide || e->type() != QEvent::Close) {
        ShowCloseBtn();
    }
    if(e->type()==QEvent::Enter){
        if(obj==ui->widget_top_center_top_center) {
            ui->widget_top_center_top_center->setStyleSheet(RES_TOP_ENTER);
        }
        else if(obj == ui->widget_left_top) {
            ui->widget_left_top->setStyleSheet(RES_LEFT_TOP_ENTER);
        }
        else if(obj == ui->widget_right_top) {
            ui->widget_right_top->setStyleSheet(RES_RIGHT_TOP_ENTER);
        }
        else if(obj == ui->widget_left_bottom) {
            ui->widget_left_bottom->setStyleSheet(RES_LEFT_BOTTOM_ENTER);
        }
        else if(obj == ui->widget_right_bottom) {
            ui->widget_right_bottom->setStyleSheet(RES_RIGHT_BOTTOM_ENTER);
        }
    }
    else if(e->type()==QEvent::Leave){
        if(obj==ui->widget_top_center_top_center) {
            ui->widget_top_center_top_center->setStyleSheet(RES_TOP_LEAVE);
        }
        else if(obj == ui->widget_left_top) {
            ui->widget_left_top->setStyleSheet(RES_LEFT_TOP_LEAVE);
        }
        else if(obj == ui->widget_right_top) {
            ui->widget_right_top->setStyleSheet(RES_RIGHT_TOP_LEAVE);
        }
        else if(obj == ui->widget_left_bottom) {
            ui->widget_left_bottom->setStyleSheet(RES_LEFT_BOTTOM_LEAVE);
        }
        else if(obj == ui->widget_right_bottom) {
            ui->widget_right_bottom->setStyleSheet(RES_RIGHT_BOTTOM_LEAVE);
        }
    }
    else {
        QMouseEvent *me=dynamic_cast<QMouseEvent *>(e);
        if(!me) {
            return true;
        }
      
        this->m_MouseObj=obj;
        QPoint currentPos=me->globalPos();
        int x=currentPos.x();
        int y=currentPos.y();
        int mw=this->m_pressPoint.x()-x;
        int mh=this->m_pressPoint.y()-y;
      
        if(e->type()==QEvent::MouseButtonPress){
            this->m_pressPoint=currentPos;
            this->m_startPoint=this->pos();
            this->m_startSize=this->size();
        }
        else if(e->type()==QEvent::MouseButtonRelease) {
            QPoint pos=ui->widget_left_top_mask->mapToGlobal(QPoint(0,0));
            int left=pos.x();
            int top=pos.y();
            int right=left+ui->widget_top_center_bottom->width()+ui->widget_left_top_mask->width()*2;
            int bottom=top+ui->widget_top_center_bottom->height()+ui->widget_center->height()+ui->widget_bottom_mask->height();
            emit this->SigAreaChanged(left,top,right,bottom);
        }
        else if(e->type()==QEvent::MouseMove) {
            ShowCloseBtn();
            //中间
            if(obj==ui->widget_top_center_top_center) {
                ReGeoMetry(this->m_startPoint.x()-mw,this->m_startPoint.y()-mh,this->width(),this->height());
            }
            //右下角
            else if(obj==ui->widget_right_bottom){
                ReGeoMetry(this->m_startPoint.x(),this->m_startPoint.y(),this->m_startSize.width()-mw,this->m_startSize.height()-mh);
            }
            //右上角
            else if(obj==ui->widget_right_top){
                int newX=this->m_startPoint.x();
            
                int newW=this->m_startSize.width()-mw;
                int newH=this->m_startSize.height()+mh;
             
                if(newW<this->minimumWidth()) {
                    newW=this->minimumWidth();
                }
                if(newW>qApp->desktop()->width()) {
                    newW=qApp->desktop()->width();
                }
                if(newH>qApp->desktop()->height()) {
                    newH=qApp->desktop()->height();
                }
                if(newH<this->minimumHeight()){
                    newH=this->minimumHeight();
                }
             
                int newY=this->m_startPoint.y()+this->m_startSize.height()-newH;
                ReGeoMetry(newX,newY,newW,newH,true);
            }
            // 左上角
            else if(obj==ui->widget_left_top){
                int newW=this->m_startSize.width()+mw;
                int newH=this->m_startSize.height()+mh;
                if(newW<this->minimumWidth()) {
                    newW=this->minimumWidth();
                }
                if(newW>qApp->desktop()->width()) {
                    newW=qApp->desktop()->width();
                }
                if(newH>qApp->desktop()->height()) {
                    newH=qApp->desktop()->height();
                }
                if(newH<this->minimumHeight()){
                    newH=this->minimumHeight();
                }
                int newX=this->m_startPoint.x()+this->m_startSize.width()-newW;
                int newY=this->m_startPoint.y()+this->m_startSize.height()-newH;
                ReGeoMetry(newX,newY,newW,newH,true);

            }
            //左下角
            else if(obj==ui->widget_left_bottom){            
                int newY=this->m_startPoint.y();
                int newW=this->m_startSize.width()+mw;
                int newH=this->m_startSize.height()-mh;
             
                if(newW<this->minimumWidth()) {
                    newW=this->minimumWidth();
                }
                if(newW>qApp->desktop()->width()) {
                    newW=qApp->desktop()->width();
                }
                if(newH>qApp->desktop()->height()) {
                    newH=qApp->desktop()->height();
                }
                if(newH<this->minimumHeight()){
                    newH=this->minimumHeight();
                }
                int newX=this->m_startPoint.x()+this->m_startSize.width()-newW;
                ReGeoMetry(newX,newY,newW,newH,true);
            }
            else if(obj==ui->widget_bottom) {
                ReGeoMetry(this->m_startPoint.x(),this->m_startPoint.y(),this->m_startSize.width(),this->m_startSize.height()-mh);
            }
            else if(obj==ui->widget_left) {
                int newY=this->m_startPoint.y();
                int newW=this->m_startSize.width()+mw;
                int newH=this->m_startSize.height();
             
                if(newW<this->minimumWidth()) {
                newW=this->minimumWidth();
                }
             
                if(newW>qApp->desktop()->width()) {
                newW=qApp->desktop()->width();
                }
             
                if(newH>qApp->desktop()->height()) {
                newH=qApp->desktop()->height();
                }
             
                if(newH<this->minimumHeight()){
                newH=this->minimumHeight();
                }
             
                int newX=this->m_startPoint.x()+this->m_startSize.width()-newW; 
                ReGeoMetry(newX,newY,newW,newH,true);
            }
            else if(obj==ui->widget_right){
                ReGeoMetry(this->m_startPoint.x(),this->m_startPoint.y(),this->m_startSize.width()-mw,this->m_startSize.height());
            }
            else if(obj==ui->widget_top_center_top_left||obj==ui->widget_top_center_top_right){
                int newX=this->m_startPoint.x(); 
                int newW=this->m_startSize.width();
                int newH=this->m_startSize.height()+mh;
             
                if(newW<this->minimumWidth()) {
                newW=this->minimumWidth();
                }
             
                if(newW>qApp->desktop()->width()) {
                newW=qApp->desktop()->width();
                }
             
                if(newH>qApp->desktop()->height()) {
                newH=qApp->desktop()->height();
                }
             
                if(newH<this->minimumHeight()){
                newH=this->minimumHeight();
                }
             
                int newY=this->m_startPoint.y()+this->m_startSize.height()-newH;

                ReGeoMetry(newX,newY,newW,newH,true);
            }
        }
    }

    return true;
}

void VHAreaMark::ToMask(){
    MaskInit();
    this->Mask(ui->widget_center);
    this->Mask(ui->widget_left_top_mask);
    this->Mask(ui->widget_left_bottom_mask);
    this->Mask(ui->widget_right_top_mask);
    this->Mask(ui->widget_right_bottom_mask);
    this->Mask(ui->widget_left_mask);
    this->Mask(ui->widget_left_mask_left);
    this->Mask(ui->widget_right_mask);
    this->Mask(ui->widget_right_mask_right);
    this->Mask(ui->widget_bottom_mask);
    this->Mask(ui->widget_bottom_mask_bottom);
    this->Mask(ui->widget_top_center_bottom);
    this->Mask(ui->widget_top_center_top_left_top);
    this->Mask(ui->widget_top_center_top_right_top);
    this->Mask(ui->widget_top_left_top);
    this->Mask(ui->widget_top_right_top);
    MaskFinial();
    
    ui->widget_top_center_top_center->setStyleSheet(ui->widget_top_center_top_center->styleSheet());
    ui->widget_left_top->setStyleSheet(ui->widget_left_top->styleSheet());
    ui->widget_right_top->setStyleSheet(ui->widget_right_top->styleSheet());
    ui->widget_left_bottom->setStyleSheet(ui->widget_left_bottom->styleSheet());
    ui->widget_right_bottom->setStyleSheet(ui->widget_right_bottom->styleSheet());
    ui->widget_left->setStyleSheet(ui->widget_left->styleSheet());
    ui->widget_top_center_top_left->setStyleSheet(ui->widget_top_center_top_left->styleSheet());
    ui->widget_top_center_top_right->setStyleSheet(ui->widget_top_center_top_right->styleSheet());
    ui->widget_right->setStyleSheet(ui->widget_right->styleSheet());
    ui->widget_bottom->setStyleSheet(ui->widget_bottom->styleSheet());
}

VHAreaMark::~VHAreaMark() {
    ui->widget_left->removeEventFilter(this);
    ui->widget_left_top->removeEventFilter(this);
    ui->widget_top->removeEventFilter(this);
    ui->widget_right_top->removeEventFilter(this);
    ui->widget_right->removeEventFilter(this);
    ui->widget_right_bottom->removeEventFilter(this);
    ui->widget_bottom->removeEventFilter(this);
    ui->widget_left_bottom->removeEventFilter(this);
    ui->widget_top_center_top_center->removeEventFilter(this);
    ui->widget_top_center_top_left->removeEventFilter(this);
    ui->widget_top_center_top_right->removeEventFilter(this);
    delete ui;
}
void VHAreaMark::ShowShared(int left,int top,int right,int bottom) {
   
   this->show();
   int x=left-(ui->widget_left_top->width()-ui->widget_left_top_mask->width());
   int y=top-ui->widget_top_center_top_center->height();
   int w=right-left+(ui->widget_left_top->width()-ui->widget_left_top_mask->width())*2;
   int h=bottom-top+(ui->widget_top_center_top_center->height() + ui->widget_bottom->height() -ui->widget_bottom_mask->height());   
   
   this->ReGeoMetry(x,y,w,h,true);
   
   QPoint pos=ui->widget_left_top_mask->mapToGlobal(QPoint(0,0));
   int rleft=pos.x();
   int rtop=pos.y();
   int rright=left+ui->widget_top_center_bottom->width()+ui->widget_left_top_mask->width()*2;
   int rbottom=top+ui->widget_top_center_bottom->height()+ui->widget_center->height()+ui->widget_bottom_mask->height();


   if(rleft!=left||rtop!=top||rright!=right||rbottom!=bottom) {
      emit this->SigAreaChanged(rleft,rtop,rright,rbottom);
   }
}

void VHAreaMark::MaskInit(){
    this->m_mainMask=QPixmap(this->size());
    this->m_mainMask.fill(Qt::black);
}

void VHAreaMark::MaskFinial(){
    this->setMask(m_mainMask);
    this->repaint();
}

void VHAreaMark::Mask(QWidget *c){
    QPixmap pixmap_mask=QPixmap(c->width(),c->height());
    pixmap_mask.fill(Qt::white);
    QPoint point=c->parentWidget()->mapTo(this,c->pos());
    QPainter painterMain(&m_mainMask);
    painterMain.drawPixmap(point.x(),point.y(),c->width(),c->height(),pixmap_mask);
}
