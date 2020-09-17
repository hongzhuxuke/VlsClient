#pragma once

#include <QWidget>
#include "ui_VhallRenderMaskWdg.h"

class VhallRenderMaskWdg : public QWidget
{
	Q_OBJECT

public:
	VhallRenderMaskWdg(QWidget *parent = Q_NULLPTR);
	~VhallRenderMaskWdg();

   void SetRoleName(const QString &role);
   void SetUserName(const QString &userName);
   void SetResetSize(bool mainView);


private:
	Ui::VhallRenderMaskWdg ui;
};
