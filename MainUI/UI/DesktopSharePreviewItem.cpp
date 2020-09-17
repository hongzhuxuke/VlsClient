#include "DesktopSharePreviewItem.h"
#include <QPainter>
#include <QtWin>

#define SELECT_LINE_WIDTH  8  //±ß¿òÏß¿í

DesktopSharePreviewItem::DesktopSharePreviewItem(QWidget *parent)
    : QWidget(parent), mbIsSelectd(false){
    ui.setupUi(this);
}

DesktopSharePreviewItem::~DesktopSharePreviewItem(){
}

void DesktopSharePreviewItem::SetSelectItemSheet(bool select) {
   mbIsSelectd = select;
   repaint();
}


void DesktopSharePreviewItem::SetDesktopInfo(int monitorIndex, VHD_WindowInfo& moniters) {
   mMonitorIndex = monitorIndex;
   mMoniterInfo = moniters;
}

int DesktopSharePreviewItem::GetCurrentIndex() {
   return mMonitorIndex;
}

VHD_WindowInfo DesktopSharePreviewItem::GetCurrentWindowInfo() {
   return mMoniterInfo;
}

void DesktopSharePreviewItem::StartCaptrue() {
   //HDC captureDC = NULL;
   //captureDC = GetDC(NULL);
   //if (captureDC == NULL) {
   //   return;
   //}

   //int width = mMoniterInfo.rect.right - mMoniterInfo.rect.left;
   //int height = mMoniterInfo.rect.bottom - mMoniterInfo.rect.top;

   //HDC hMemDC = nullptr;
   //HBITMAP hBmp = nullptr;
   //HGDIOBJ hOldBmp = nullptr;
   //hMemDC = CreateCompatibleDC(captureDC);
   //if (hMemDC) {
   //   hBmp = CreateCompatibleBitmap(captureDC, 1920, 1080);
   //   if (hBmp) {
   //      hOldBmp = SelectObject(hMemDC, hBmp);
   //      BitBlt(hMemDC, 0, 0, width, height, captureDC, mMoniterInfo.rect.left, mMoniterInfo.rect.top, CAPTUREBLT | SRCCOPY);
   //      SelectObject(hMemDC, hOldBmp);
   //      mCaptrueImage = QtWin::fromHBITMAP(hBmp);
   //      DeleteObject(hBmp);
   //   }
   //   DeleteDC(hMemDC);
   //}
   //ReleaseDC(NULL, captureDC);


   if (mCaptureDC == nullptr) {
      mCaptureDC = GetDC(NULL);
   }
   if (mMemDC == nullptr) {
      mMemDC = CreateCompatibleDC(mCaptureDC);
   }

   int width = mMoniterInfo.rect.right - mMoniterInfo.rect.left;
   int height = mMoniterInfo.rect.bottom - mMoniterInfo.rect.top;

   int bytes_per_row = width * 4;
   int buffer_size = bytes_per_row * height;
   // Describe a device independent bitmap (DIB) that is the size of the desktop.
   //BITMAPINFO bmi = {};
   //bmi.bmiHeader.biHeight = -height;
   //bmi.bmiHeader.biWidth = width;
   //bmi.bmiHeader.biPlanes = 1;
   //bmi.bmiHeader.biBitCount = 4 * 8;
   //bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
   //bmi.bmiHeader.biSizeImage = bytes_per_row * height;
   //void* data = nullptr;
   //HANDLE hMySharedMapFile = CreateFileMapping(((HANDLE)0xFFFFFFFF), NULL, PAGE_READWRITE, 0, buffer_size, L"MySharedMem");
   //HBITMAP hBmp = CreateDIBSection(mCaptureDC, &bmi, DIB_RGB_COLORS, &data, hMySharedMapFile, 0);
   HGDIOBJ previous_object = nullptr;

   if (mMemDC) {
      HBITMAP hBmp  = CreateCompatibleBitmap(mCaptureDC, width, height);
      if (hBmp) {
         previous_object = SelectObject(mMemDC, hBmp);
         BitBlt(mMemDC, 0, 0, width, height, mCaptureDC, mMoniterInfo.rect.left, mMoniterInfo.rect.top, CAPTUREBLT | SRCCOPY);
         SelectObject(mMemDC, previous_object);
         mCaptrueImage = QtWin::fromHBITMAP(hBmp);
         DeleteObject(hBmp);
      }
      DeleteObject(mMemDC);
      mMemDC = nullptr;
   }
   if (mCaptureDC) {
      DeleteObject(mCaptureDC);
      mCaptureDC = nullptr;
   }
}

void DesktopSharePreviewItem::paintEvent(QPaintEvent *) {
   QPainter painter(this);
   painter.drawPixmap(0, 0, this->width(), this->height(), mCaptrueImage);
   if (mbIsSelectd) {
      painter.setPen(QPen(QColor(252, 86, 89), SELECT_LINE_WIDTH, Qt::SolidLine));
      painter.drawRect(0, 0, this->width(), this->height());
   }
}

bool DesktopSharePreviewItem::GetSelectState() {
   return mbIsSelectd;
}
