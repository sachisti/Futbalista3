#include "opencv2/opencv.hpp"
#include <iostream>

using namespace std;
using namespace cv;

class KameraVec {
public:
    Scalar low, high; // hsv values
    Point2f mid = {-1, -1};
    KameraVec(int mh, int ms, int mv, int mxh, int mxs, int mxv) : low(mh, ms, mv), high(mxh, mxs, mxv) {}

    int maxAreaContour(vector<vector<Point>> cnts) {
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
};

class Lopta : KameraVec {
    int min_size = 5;
public:
    float r = -1;
    using KameraVec::KameraVec;

    void find(Mat frame) { // updates mid and r
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
        }
    }

    void debug(Mat frame) {
        find(frame);
        circle(frame, mid, r, { 0, 255, 255 }, 2);
        imshow("Ball tracking debug", frame);
    }
};

class Branka : KameraVec {
    int min_size = 15;
public:
    float h = -1, w = -1;
    using KameraVec::KameraVec;

    void find(Mat frame) { // updates mid, h, w
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
        }
    }

    void debug(Mat frame) {
        find(frame);
        rectangle(frame, { mid.x - w / 2, mid.y - h / 2 }, { mid.x + w / 2, mid.y + h / 2 }, { 0, 255, 255 }, 2);
        imshow("Goal tracking debug", frame);
    }
};

void calibrate_camera() {
    VideoCapture cap(0);

    if (!cap.isOpened()) {
        cout << "Error opening video stream or file" << endl;
        return;
    }
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
        Mat frame;// = imread("E:/DOKUMENTY_MACKO/.Programovanie/Sachista3/monke.jpg");
        cap >> frame;
        if (frame.empty())
            break;
        // Press  ESC on keyboard to exit
        char c = (char)waitKey(25);
        if (c == 27) {
            cout << "minHSV: " << mH << " " << mS << " " << mV << endl;
            cout << "miaxHSV: " << mxH << " " << mxS << " " << mxV << endl;
            break;
        }
        /*
        imshow("Frame", frame);
        Rect trim(0, 0, frame.size().width, frame.size().height/2);
        Mat trimed = frame(trim);
        imshow("Trimed", trimed);

        Mat gray;
        cvtColor(trimed, gray, COLOR_BGR2GRAY);
        imshow("Gray", gray);

        Mat edges;
        Sobel(gray, edges, CV_64F, 1, 1);
        imshow("Edges", edges);

        Mat red, hsv, hsvchannel[3];
        cvtColor(trimed, hsv, COLOR_BGR2HSV);
        blur(hsv, hsv, Size(5, 5));
        split(hsv, hsvchannel);
        Mat vts;
        Scalar lowerBound = cv::Scalar(150, 100, 50);
        Scalar upperBound = cv::Scalar(255, 255, 255);
        inRange(hsv, lowerBound, upperBound, vts);
        imshow("in range", vts);
        */
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

    cap.release();
    destroyAllWindows();

    return;
}
