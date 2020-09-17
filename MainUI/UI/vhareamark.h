#ifndef VHAREAMARK_H
#define VHAREAMARK_H

#include <QWidget>
#include <QPushButton>
#include "AreaCloseWdg.h"

namespace Ui {
class VHAreaMark;
}

class VHAreaMark : public QWidget
{
    Q_OBJECT

public:
    explicit VHAreaMark(QWidget *parent = 0);
    ~VHAreaMark();
	void ShowShared(int left,int top,int right,int bottom);

signals:
	void SigAreaChanged(int left,int top,int right,int bottom);
    void SigCloseAreaShare();
protected:
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
	 void resizeEvent(QResizeEvent *);
    bool eventFilter(QObject *, QEvent *);
    void paintEvent(QPaintEvent *);
    void MaskInit();
    void Mask(QWidget *c);
    void MaskFinial();
    void ReGeoMetry(int, int, int, int, bool notOnlyResize=false);
    void ToMask();
    void ShowCloseBtn();
private slots:
    void slot_closeAreaShare();
private:
    Ui::VHAreaMark *ui;
    QPixmap m_mainMask;
    QObject *m_MouseObj = NULL;
    QPoint m_pressPoint;
    QPoint m_startPoint;
    QSize m_startSize;

    AreaCloseWdg *mCloseBtn;
};

#endif // VHAREAMARK_H
