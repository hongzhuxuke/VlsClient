#ifndef VHALLSHAREDQR_H
#define VHALLSHAREDQR_H

#include <QDialog>
#include <QPaintEvent>    
#include <QPainter>
#include <QPoint>
#include "title_button.h"
#include "cbasedlg.h"

namespace Ui {
class VhallSharedQr;
}

class HttpNetWork;
class VhallSharedQr : public CBaseDlg
{
    Q_OBJECT

public:
    explicit VhallSharedQr(QWidget *parent = 0);
    ~VhallSharedQr();
    void Shared(QString domain,QString streamID);
    void CenterWindow(QWidget* parent);
protected:
    bool eventFilter(QObject *obj, QEvent *ev);
   virtual void customEvent(QEvent* event);

private slots:
   void Slot_OnCheckMixStreamServerFinished(QByteArray, int);
   void Slot_OnCopyShareUrl();
   void Slot_OnClose();
   void Slot_OnLoadSharePic();
private:
    Ui::VhallSharedQr *ui;
    QString mOutFile;
    QPixmap mBackgroundPixmap;
    TitleButton *m_pBtnClose = NULL;
    QPoint pressPoint;
    QPoint startPoint;
    HttpNetWork* mHttpNetWork = NULL;

   QTimer* mLoadSharePicTimer = nullptr;
};

#endif // VHALLSHAREDQR_H
