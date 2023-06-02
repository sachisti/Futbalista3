#include "futbalista.h"

using namespace std;
using namespace cv;

int CAMERA_INDEX = 0;
VideoCapture cap(0);

Mat get_frame(){
    Mat frame;
    cap >> frame;
}

int setup_kamera(){
    int s;
    VideoCapture cap(CAMERA_INDEX);
    s = cap.get(CAP_PROP_FRAME_WIDTH);
    cap.release();
    return s;
}

KameraVec::KameraVec(int mh, int ms, int mv, int mxh, int mxs, int mxv) : low(mh, ms, mv), high(mxh, mxs, mxv){
    mid = {-1, -1};
}

int KameraVec::maxAreaContour(vector<vector<Point>> cnts) {
    double ma = -1;
    int id = -1;
    for (int i = 0; i < cnts.size(); i++) {
        double na = contourArea(cnts[i]);
        if (na > ma) {
            ma = na;
            id = i;
        }
    }
    return id;
}


bool Lopta::find(Mat frame) { // updates mid and r
    cvtColor(frame, frame, COLOR_BGR2HSV);
    blur(frame, frame, Point(3, 3));
    Mat mask;
    inRange(frame, low, high, mask);
    erode(mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(min_size, min_size)));
    dilate(mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(min_size, min_size)));
    vector<vector<Point>> cnts;
    findContours(mask, cnts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    if (cnts.size() > 0) {
        vector<Point> cnt = cnts[maxAreaContour(cnts)];
        minEnclosingCircle(cnt, mid, r);
        return 1;
    }
    return 0;
}

bool Lopta::find(){
    
    return find(get_frame());
}

void Lopta::debug() {
    Mat frame = get_frame();
    Lopta::find(frame);
    circle(frame, mid, r, { 0, 255, 255 }, 2);
    imshow("Ball tracking debug", frame);
}

bool Branka::find(Mat frame) { // updates mid, h, w
    cvtColor(frame, frame, COLOR_BGR2HSV);
    blur(frame, frame, Point(3, 3));
    Mat mask;
    inRange(frame, low, high, mask);
    erode(mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(min_size, min_size)));
    dilate(mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(min_size, min_size)));
    vector<vector<Point>> cnts;
    findContours(mask, cnts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    if (cnts.size() > 0) {
        vector<Point> cnt = cnts[maxAreaContour(cnts)];
        RotatedRect rect;
        rect = minAreaRect(cnt);
        h = rect.size.height;
        w = rect.size.width;
        mid = rect.center;
        return 1;
    }
    return 0;
}

bool Branka::find(){
    Mat frame;
    cap >> frame;
    return find(frame);
}

void Branka::debug() {
    Mat frame = get_frame();
    Branka::find(frame);
    rectangle(frame, { mid.x - w / 2, mid.y - h / 2 }, { mid.x + w / 2, mid.y + h / 2 }, { 0, 255, 255 }, 2);
    imshow("Goal tracking debug", frame);
}


void calibrate_camera() {
    namedWindow("ValueRange");
    namedWindow("SatRange");
    namedWindow("HueRange");
    int mV = 50, mxV = 255, mS = 50, mxS = 255, mH = 0, mxH = 255;
    createTrackbar("Value min", "ValueRange", &mV, 255);
    createTrackbar("Value max", "ValueRange", &mxV, 255);
    createTrackbar("Saturation min", "SatRange", &mS, 255);
    createTrackbar("Saturation max", "SatRange", &mxS, 255);
    createTrackbar("Hue min", "HueRange", &mH, 255);
    createTrackbar("Hue max", "HueRange", &mxH, 255);
    while (1) {
        Mat frame = get_frame();// = imread("E:/DOKUMENTY_MACKO/.Programovanie/Sachista3/monke.jpg");
        if (frame.empty())
            break;
        // Press  ESC on keyboard to exit
        char c = (char)waitKey(25);
        if (c == 27) {
            cout << "minHSV: " << mH << " " << mS << " " << mV << endl;
            cout << "miaxHSV: " << mxH << " " << mxS << " " << mxV << endl;
            break;
        }
        cvtColor(frame, frame, COLOR_BGR2HSV);
        blur(frame, frame, Point(3, 3));
        imshow("Defult", frame);
        Mat value;
        inRange(frame, Scalar(0, 0, mV), Scalar(255, 255, mxV), value);
        imshow("ValueRange", value);
        Mat sat;
        inRange(frame, Scalar(0, mS, 0), Scalar(255, mxS, 255), sat);
        sat &= value;
        imshow("SatRange", sat);
        Mat hue;
        if (mH <= mxH) {
            inRange(frame, Scalar(mH, 0, 0), Scalar(mxH, 255, 255), hue);
        }
        else {
            Mat temp, temp1;
            inRange(frame, Scalar(0, 0, 0), Scalar(mxH, 255, 255), temp);
            inRange(frame, Scalar(mH, 0, 0), Scalar(255, 255, 255), temp1);
            hue = temp | temp1;
        }
        hue &= sat;
        imshow("HueRange", hue);
        resizeWindow("ValueRange", Size(500, 500));
        resizeWindow("SatRange", Size(500, 500));
        resizeWindow("HueRange", Size(500, 500));
    }
    destroyAllWindows();

    return;
}
    
