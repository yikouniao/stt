#include "stdafx.h"
#include "obj-tracking.h"

using namespace std;
using namespace cv;

const char fname_mp4[] = "D:\\temp\\test_";
char fname_bmp[] = "C:\\temp\\BMPCAPTURE.bmp";

Rect box; // bounding box
bool drawing_box = false;
bool gotBB = false; // 是否已经画出bounding box

int fps = 25;//帧率25

static void mouseHandler(int event, int x, int y, int flags, void *param) {
  switch (event) {
  case CV_EVENT_MOUSEMOVE:
    if (drawing_box) {
      box.width = x - box.x;
      box.height = y - box.y;
    }
    break;
  case CV_EVENT_LBUTTONDOWN:
    drawing_box = true;
    box = Rect(x, y, 0, 0);
    break;
  case CV_EVENT_LBUTTONUP:
    drawing_box = false;
    if (box.width < 0) {
      box.x += box.width;
      box.width *= -1;
    }
    if (box.height < 0) {
      box.y += box.height;
      box.height *= -1;
    }
    gotBB = true;
    break;
  }
}

void ObjTracking(long m_hChl) {
  if (m_hChl != -1) {
    namedWindow("frame");
    cvSetMouseCallback("frame", mouseHandler, NULL);
    VSNET_ClientCapturePicture(m_hChl, fname_bmp);
    Mat frame = imread(fname_bmp);
    imshow("frame", frame);
    waitKey(1000 / fps);
    //get bounding box

    while (!gotBB) {
      rectangle(frame, cvPoint(box.x, box.y),
                cvPoint(box.x + box.width, box.y + box.height), Scalar(0, 0, 255));
      imshow("frame", frame);
      waitKey(1000 / fps);
    }

    cvSetMouseCallback("frame", NULL, NULL);
    while (true) {
      VSNET_ClientCapturePicture(m_hChl, fname_bmp);
      Mat frame = imread(fname_bmp);

      // do tracking here

      rectangle(frame, cvPoint(box.x, box.y),
                cvPoint(box.x + box.width, box.y + box.height), Scalar(255, 0, 0));
      imshow("frame", frame);
      waitKey(1000 / fps);
    }
  }
}