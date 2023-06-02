#include <inttypes.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include "third-party/include/serial/serial.h"


using namespace std;
using namespace cv;

// komunikacia 
int pripoj_arduino();
void zapis_paket_do_arduina(const uint8_t *zapisovany_paket);

// kamera
int setup_kamera();

Mat get_frame();

class KameraVec {
public:
     Scalar low, high; // hsv values
     Point2f mid;
     KameraVec(int mh, int ms, int mv, int mxh, int mxs, int mxv);

     int maxAreaContour(vector<vector<Point>> cnts);
};
class Lopta : public KameraVec {
    int min_size = 5;
public:
    float r = -1;
    using KameraVec::KameraVec;

    bool find(Mat frame);
    bool find();
    void debug();
};
class Branka : public KameraVec {
    int min_size = 15;
public:
    float h = -1, w = -1;
    using KameraVec::KameraVec;

    bool find(Mat frame);
    bool find();
    void debug();
};
void calibrate_camera();


// logovanie
void setup_log();
void zaloguj(char *sprava);
void zaloguj_n(char *sprava, int cislo);

