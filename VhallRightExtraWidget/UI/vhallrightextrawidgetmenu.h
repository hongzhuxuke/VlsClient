#ifndef VHALLRIGHTEXTRAWIDGETMENU_H
#define VHALLRIGHTEXTRAWIDGETMENU_H

#include <QWidget>
#include <QToolButton>
#include <QList>

namespace Ui {
class VhallRightExtraWidgetMenu;
}

class VhallRightExtraWidgetMenu : public QWidget
{
    Q_OBJECT

public:
    explicit VhallRightExtraWidgetMenu(QWidget *parent = 0);
    ~VhallRightExtraWidgetMenu();
    int Refresh(QStringList &names);
signals:
    void SigBtnClicked(QString);
public slots:
    void BtnClicked(QAbstractButton *);
private:
    Ui::VhallRightExtraWidgetMenu *ui;
    QList<QToolButton *> m_btns;
    QButtonGroup *m_btnGroup;
};

#endif // VHALLRIGHTEXTRAWIDGETMENU_H
