#include "DesktopShareSelectWdg.h"
#include <QDesktopWidget>

DesktopShareSelectWdg::DesktopShareSelectWdg(QWidget *parent)
   : QDialog(parent)
{
   ui.setupUi(this);
   this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint | Qt::Dialog);
   setAttribute(Qt::WA_TranslucentBackground);
   setAutoFillBackground(false);

   mCaptureTimer = new QTimer(this);
   if (mCaptureTimer) {
      connect(mCaptureTimer, SIGNAL(timeout()), this, SLOT(slot_StartCapture()));
   }
   if (m_pNoiseTips == NULL) {
      m_pNoiseTips = new DesktopWarnningTips();
   }
   connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(slot_AcceptToCaptureScreen()));
   connect(ui.pushButton_left, SIGNAL(clicked()), this, SLOT(slot_SelectPrev()));
   connect(ui.pushButton_right, SIGNAL(clicked()), this, SLOT(slot_SelectNext()));

   connect(ui.pushButton_close, SIGNAL(sigClicked()), this, SLOT(slot_OnClose()));
   ui.pushButton_close->loadPixmap(":/sysButton/close_button");
   ui.pushButton_left->setEnabled(true);
   ui.pushButton_right->setEnabled(true);
   ui.label_tips->installEventFilter(this);
}

DesktopShareSelectWdg::~DesktopShareSelectWdg() {
   if (mCaptureTimer) {
      mCaptureTimer->stop();
   }
   if (m_pNoiseTips) {
      delete m_pNoiseTips;
      m_pNoiseTips = nullptr;
   }
}

void DesktopShareSelectWdg::StopCapture() {
   if (mCaptureTimer) {
      mCaptureTimer->stop();
   }
}

void DesktopShareSelectWdg::slot_SelectPrev() {
   int curIndex = ui.listWidget->currentRow();
   if (curIndex != 0) {
      ui.listWidget->setCurrentRow(curIndex - 1);
   }
}

void DesktopShareSelectWdg::slot_OnClose() {
   reject();
}

void DesktopShareSelectWdg::slot_SelectNext() {
   int curIndex = ui.listWidget->currentRow();
   if (curIndex != ui.listWidget->count() - 1) {
      ui.listWidget->setCurrentRow(curIndex + 1);
   }
}

VHD_WindowInfo DesktopShareSelectWdg::GetCurrentWindowInfo() {
   return mMoniterInfo;
}

int DesktopShareSelectWdg::GetCurrentIndex() {
   return mMonitorIndex;
}

bool DesktopShareSelectWdg::eventFilter(QObject *obj, QEvent *ev) {
   if (ev->type() == QEvent::Enter) {
      //ÊÓÆµÖÊÁ¿ÆÀ¹À
      if (obj == ui.label_tips && m_pNoiseTips != NULL) {
         m_pNoiseTips->show();
         QPoint pos = this->mapToGlobal(ui.label_tips->pos());
         m_pNoiseTips->move(pos + QPoint(25, m_pNoiseTips->height()/2));
      }
   }
   if (ev->type() == QEvent::Leave) {
      if (obj == ui.label_tips && m_pNoiseTips != NULL && !m_pNoiseTips->isHidden()) {
         m_pNoiseTips->close();
      }
   }
   return true;
}

void DesktopShareSelectWdg::InsertDesktopPreviewItem(std::vector<VHD_WindowInfo>& moniters) {
   int monitorCount = moniters.size();
   if (monitorCount <= 2) {
      ui.pushButton_left->hide();
      ui.pushButton_right->hide();
      ui.label_left->show();
      ui.label_right->show();
   }
   for (int index = 0; index < monitorCount; index++) {
      DesktopSharePreviewItem* previewWdg = new DesktopSharePreviewItem(ui.listWidget);
      if (previewWdg) {
         previewWdg->SetDesktopInfo(moniters.at(index).screen_id, moniters.at(index));
         if (index == 0) {
            previewWdg->SetSelectItemSheet(true);
            if (monitorCount > 2) {
               ui.pushButton_right->show();
            }
         }
      }
      QListWidgetItem* item = new QListWidgetItem(ui.listWidget);
      if (previewWdg && item) {
         connect(ui.listWidget, SIGNAL(itemPressed(QListWidgetItem *)), this, SLOT(slot_itemPressed(QListWidgetItem*)));
         connect(ui.listWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(slot_currentItemChanged(QListWidgetItem *, QListWidgetItem *)));
         item->setSizeHint(QSize(225, 128));
         ui.listWidget->addItem(item);
         ui.listWidget->setItemWidget(item, previewWdg);
      }
   }
   ui.listWidget->setCurrentRow(0);
   if (mCaptureTimer) {
      mCaptureTimer->start(500);
   }
}

void DesktopShareSelectWdg::slot_itemPressed(QListWidgetItem* item) {
}

void DesktopShareSelectWdg::slot_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
   if (current == previous) {
      return;
   }
   if (previous) {
      DesktopSharePreviewItem *previewWdg = dynamic_cast<DesktopSharePreviewItem*>(ui.listWidget->itemWidget(previous));
      if (previewWdg) {
         previewWdg->SetSelectItemSheet(false);
      }
   }
   if (current) {
      DesktopSharePreviewItem *currentWdg = dynamic_cast<DesktopSharePreviewItem*>(ui.listWidget->itemWidget(current));
      if (currentWdg) {
         currentWdg->SetSelectItemSheet(true);
         if (QApplication::desktop()->numScreens() > 2) {
            if (currentWdg->GetCurrentIndex() == 0) {
               ui.pushButton_left->hide();
               ui.pushButton_right->show();
               ui.label_left->show();
               ui.label_right->hide();
            }
            else if (currentWdg->GetCurrentIndex() == ui.listWidget->count() - 1) {
               ui.pushButton_left->show();
               ui.pushButton_right->hide();
               ui.label_left->hide();
               ui.label_right->show();
            }
            else {
               ui.pushButton_left->show();
               ui.pushButton_right->show();
               ui.label_left->hide();
               ui.label_right->hide();
            }
         }
      }
   }
}

void DesktopShareSelectWdg::slot_AcceptToCaptureScreen() {
   QListWidgetItem *current = ui.listWidget->currentItem();
   if (current) {
      DesktopSharePreviewItem *currentWdg = dynamic_cast<DesktopSharePreviewItem*>(ui.listWidget->itemWidget(current));
      if (currentWdg) {
         mMonitorIndex = currentWdg->GetCurrentIndex();
         mMoniterInfo = currentWdg->GetCurrentWindowInfo();
         accept();
      }
   }
}


void DesktopShareSelectWdg::mousePressEvent(QMouseEvent *event) {
   if (event) {
      mPressPoint = this->pos() - event->globalPos();
      mIsMoved = true;
   }
}

void DesktopShareSelectWdg::mouseMoveEvent(QMouseEvent *event) {
   if (event && (event->buttons() == Qt::LeftButton) && mIsMoved  && m_bIsEnableMove) {
      this->move(event->globalPos() + mPressPoint);
      this->raise();
   }
}

void DesktopShareSelectWdg::mouseReleaseEvent(QMouseEvent *) {
   if (mIsMoved) {
      mIsMoved = false;
   }
   QWidget* parent_widget = this;
   QPoint pos = parent_widget->pos();
   QRect rect = QApplication::desktop()->availableGeometry(pos);
   QRect wnd_rect = parent_widget->frameGeometry();
   if (pos.y() > rect.height() - 150) {
      pos.setY(rect.height() - 150);
      parent_widget->move(pos);
   }
   else if (pos.y() < rect.y()) {
      parent_widget->move(pos + QPoint(0, +100));
   }
   else if (wnd_rect.x() >= rect.x() + rect.width() - 100) {
      parent_widget->move(QPoint(wnd_rect.x(), wnd_rect.y()) + QPoint(-100, 0));
   }
   else if (wnd_rect.x() + this->width() < rect.x() + 100) {
      parent_widget->move(QPoint(wnd_rect.x(), wnd_rect.y()) + QPoint(100, 0));
   }
}

void DesktopShareSelectWdg::keyPressEvent(QKeyEvent *event) {
   switch (event->key())
   {
   case Qt::Key_Escape:
      break;
   default:
      QDialog::keyPressEvent(event);
   }
}

void DesktopShareSelectWdg::slot_StartCapture() {
   for (int index = 0; index < ui.listWidget->count(); index++) {
      QListWidgetItem* item = ui.listWidget->item(index);
      if (item) {
         DesktopSharePreviewItem *currentWdg = dynamic_cast<DesktopSharePreviewItem*>(ui.listWidget->itemWidget(item));
         if (currentWdg) {
            currentWdg->StartCaptrue();
            currentWdg->repaint();
         }
      }
   }
}