#ifndef VHALLDEBUGFORM_H
#define VHALLDEBUGFORM_H

#include <QWidget>
#include <QMutex>

namespace Ui {
class VhallDebugForm;
}

class VhallDebugForm : public QWidget
{
    Q_OBJECT

public:
    explicit VhallDebugForm(QWidget *parent = 0);
    ~VhallDebugForm();
    void HookQDebug();
    void Msg(QString);
    static QMutex mutex;
    static VhallDebugForm *_this;
    static void HookQDebugToFile();
signals:
    void SigMsg(QString);
public slots:
    void SlotMsg(QString);
private:
    Ui::VhallDebugForm *ui;
};

#endif // VHALLDEBUGFORM_H
