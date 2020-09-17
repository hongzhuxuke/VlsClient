#ifndef MEDIAITEM_H
#define MEDIAITEM_H

#include <QObject>
#include <QWidget>
#include <string>
class QLabel;
class QCheckBox;
class PushButton;

struct MediaInfo {
   QString strFileName;
   bool isSelected;
};

class MediaItem : public QWidget {
   Q_OBJECT

public:
   MediaItem(MediaInfo *mediaInfo, QWidget *parent);
    ~MediaItem();
   bool isChecked();
   QString getFilename();
private slots:
    void emitDeleteItems();
    void emitCheckBoxState(int);

signals:
    void deleteItems(MediaItem*);
    void CheckBoxState(int);
	void CheckBoxClicked();

private:
   QCheckBox *mMediaCbx = NULL;
   QLabel *mMediaName = NULL;
   PushButton *mMediaDeleteBtn = NULL;
	QString mMediaFileName;
public:
   bool mCheckedState = false;
};

#endif // MEDIAITEM_H
