#ifndef VHALLRIGHTEXTRAWIDGET_H
#define VHALLRIGHTEXTRAWIDGET_H

#include <QWidget>
#include <QEvent>
#include "ToolButton.h"
#include "ui_vhallrightextrawidget.h"

class VhallRightExtraWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VhallRightExtraWidget(QWidget *parent = 0);
    ~VhallRightExtraWidget();
    void AppendWidget(void *w,wchar_t *name);
    void RemoveWidget(void *w);
    bool IsCurrentWidget(void *w);
    bool ISShrink();
    void SetShrink(bool);
    void MouseMovedExtraWidget();
    QStringList GetTabsObjectNames();
    void SetPage(QString);
	void SetDesktopMode(bool DesktopMode);
    void SetMenuWidget(QWidget *);

    void SetSuspensionEnable(bool);
    void * GatherOnlineList();
    void RecoveryOnlineList(void *);
signals:
    void SigEnterBtn();
    void SigClickedTest();
    void SigTabClicked();

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
    void leaveEvent(QEvent *);      
    bool eventFilter(QObject *o, QEvent *e);
private:
    Ui::VhallRightExtraWidget ui;
    ToolButton *m_btnChange;
    bool m_bSharink;
    bool m_bDesktopMode = false;
    QPoint pressPoint;
    QPoint startPoint;
    QWidget *menuWidget = NULL;
    QWidget *m_pOnlineListWidget = NULL;

public slots:
   void EnterChange();
private slots:
   void on_stackedWidget_currentChanged(int);
   void on_pushButton_test_clicked();
};

#endif // VHALLRIGHTEXTRAWIDGET_H
