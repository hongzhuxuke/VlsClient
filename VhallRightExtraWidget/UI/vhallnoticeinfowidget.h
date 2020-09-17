#ifndef VHALLNOTICEINFOWIDGET_H
#define VHALLNOTICEINFOWIDGET_H

#include <QWidget>
#include <QWebEngineView>
#include <QWebChannel>
#include <QWebEngineSettings>

namespace Ui {
class VhallNoticeInfoWidget;
}


class VhallNoticeInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VhallNoticeInfoWidget(QWidget *parent = 0);
    ~VhallNoticeInfoWidget();
    void appendNoticeMsg(QString msg);
    void Refresh(bool bInit = false);
    void ShowNoticeWnd();
    bool IsLoadUrlFinished();
public slots:
    void sendNotice(QString msg);
    void doSendNotice(QString msg);
    void loadFinished(bool);

protected:
   bool eventFilter(QObject *obj, QEvent *e);

private:
   void AddWebEngineView();

private slots:
   void slotsTextChanged();

private:
    Ui::VhallNoticeInfoWidget *ui;
    QWebEngineView *m_pWebEngineView = NULL;
    QWebChannel* m_pWebChannel = NULL;
    bool m_bIsFirstShow = true;
    bool mIsLoadUrlFinished = true;
};

#endif // VHALLNOTICEINFOWIDGET_H
