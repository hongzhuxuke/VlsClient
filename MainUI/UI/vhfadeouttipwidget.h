#ifndef VHFADEOUTTIPWIDGET_H
#define VHFADEOUTTIPWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPaintEvent>
#include <QLabel>
class VHFadeOutTipWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VHFadeOutTipWidget(QWidget *parent = 0);
    ~VHFadeOutTipWidget();
    void Tip(/*QWidget* mainWdg,*/QPoint, QString, const int& iW);
    
    void Stop();
    void SetSize(int ,int);

public slots:
    void timeout();
protected:
   void paintEvent(QPaintEvent *);
   void mousePressEvent(QMouseEvent *event);
private:
	void rePos(/*QWidget* mainWdg*/);

   QTimer *m_timer = NULL;
   int m_closeCount = 0;
   QLabel *m_tipLabel = NULL;
   QPoint mPoint;
   QSize m_globalSize;
   QWidget *m_pParentWdg = NULL;
};

#endif // VHFADEOUTTIPWIDGET_H
