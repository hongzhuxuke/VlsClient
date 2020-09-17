#include "cvoicetranslatewdg.h"
#include <QMouseEvent>

#define EMPTY_SPACE  QString("        ")  //用于文字两边空余显示。

CVoiceTranslateWdg::CVoiceTranslateWdg(QWidget *parent)
: QWidget(parent) {
   ui.setupUi(this);
   this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
   setAttribute(Qt::WA_TranslucentBackground);
   setAutoFillBackground(false);

   SetWndToTop(false);
   connect(&m_cleatTextTimer,SIGNAL(timeout()),this,SLOT(slot_ClearText()));
   //ui.pushButton_direct->hide();
}

CVoiceTranslateWdg::~CVoiceTranslateWdg() {
   m_cleatTextTimer.stop();
}

void CVoiceTranslateWdg::ResetFontSize(const int size) {
   this->setStyleSheet(QString::fromUtf8("QLabel#label{\n"
      "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(10, 10, 10, 5), stop:0.2 rgba(15, 15, 15, 100),stop:0.25 rgba(10, 10, 10, 120), stop:0.75 rgba(10, 10, 10, 120),stop:0.8 rgba(15, 15, 15, 110),stop:1 rgba(10, 10, 10, 5));\n"
      "color: rgb(255, 255, 255);\n"
      "font: %1pt \"\345\276\256\350\275\257\351\233\205\351\273\221\";\n"
      "}").arg(size));
   //if (size == 15) {
   //   resize(QSize(this->width(),30));
   //}
   //else if(size == 20){
   //   resize(QSize(this->width(), 32));
   //}
   //else if(size == 25){
   //   resize(QSize(this->width(), 36));
   //}
   update();
}

void CVoiceTranslateWdg::SetVoiceTranslateText(QString text) {
   if (text.isEmpty()) {
      ui.label->setText("");
      hide();
   }
   else {
      QString showText = EMPTY_SPACE + text + EMPTY_SPACE;
      QChar c = QChar(65533);  //底层vhall_media_core上报的文本是utf8的，转成unicode会出现乱码。乱码字符为‘65533’，此处应用层进行过滤。
      showText = showText.replace(c, "");
      if (!showText.isEmpty()) {
         ui.label->setText(showText);
         show();
         raise();
      }
      else {
         ui.label->setText("");
         hide();
      }
   }
   m_cleatTextTimer.stop();
   m_cleatTextTimer.start(2000);
}

void CVoiceTranslateWdg::SetWndToTop(bool toTop /*= false*/) {
   //if (toTop) {
   //   ui.pushButton_direct->setStyleSheet(QLatin1String("QPushButton#pushButton_direct{\n"
   //      "border-image: url(:/toolWidget/img/toolWidget/moveToBottomDefualt.png);\n"
   //      "}\n"
   //      "\n"
   //      "QPushButton#pushButton_direct:hover{\n"
   //      "border-image: url(:/toolWidget/img/toolWidget/moveToBottom.png);\n"
   //      "}\n"
   //      "\n"
   //      ""));
   //}
   //else {
   //   ui.pushButton_direct->setStyleSheet(QLatin1String("QPushButton#pushButton_direct{\n"
   //      "border-image: url(:/toolWidget/img/toolWidget/moveToTopDefualt.png);\n"
   //      "}\n"
   //      "\n"
   //      "QPushButton#pushButton_direct:hover{\n"
   //      "border-image: url(:/toolWidget/img/toolWidget/moveToTop.png);\n"
   //      "}\n"
   //      "\n"
   //      ""));
   //}
   m_bIsTop = toTop;
   update();
}

bool CVoiceTranslateWdg::IsTopPos() {
   return m_bIsTop;
}

void CVoiceTranslateWdg::mousePressEvent(QMouseEvent *event) {
   if (m_pMainUi) {
      m_pMainUi->raise();
   }
}

void CVoiceTranslateWdg::RegisterMainUI(QWidget *mainUi) {
   if (mainUi) {
      m_pMainUi = mainUi;
   }
}  

void CVoiceTranslateWdg::slot_ClearText() {
   ui.label->setText(QString());
   m_cleatTextTimer.stop();
   this->hide();
}

