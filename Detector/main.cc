#include <iostream>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>

#include "detectorBase.hpp"
#include "goodFeatureDist.hpp"
#include "fastFeature.hpp"
#include "config.hpp"

using namespace std;
using namespace cv;

const string configPath = "../config.yml";

void drawKeyPoints(Mat image, vector<MKeyPoint>& keyPoints) {
    cvtColor(image, image, cv::COLOR_GRAY2BGR);
    for (int i = 0; i < keyPoints.size(); i++) {
        int  level = keyPoints[i].mLevel;
        Point2f px = keyPoints[i].mPix;
        Scalar color;
        switch(level) {
            case 0: color = Scalar(255, 0, 0); break;
            case 1: color = Scalar(0, 255, 0); break;
            case 2: color = Scalar(0, 0, 255); break;
        }
        cv::circle(image, px, 2, color, -1);
    }

    imshow("detect result", image);
    waitKey();
}

int main(int argc, char* argv[]) {
    /*  read config file  */
    Config cfg(configPath);

    /*  create detector  */
    // DetectorBase* detector = new GoodFeatureDist(cfg.imageWidth, cfg.imageHeight, cfg.cellSize);
    DetectorBase* detector = new FastFeature(cfg.imageWidth, cfg.imageHeight, cfg.cellSize, 20);
    
    /*  build image pyramad  */
    Mat image = imread(cfg.imagePath, 0);
    vector<Mat> pyramid(3);
    pyramid[0] = image.clone();

    for (int i = 1; i < 3; i++) {
        pyrDown(pyramid[i-1], pyramid[i]);
    }

    /*  detect key points  */
    vector<MKeyPoint> keyPoints;
    detector->detect(pyramid, keyPoints);

    /*   draw key points  */
    drawKeyPoints(image, keyPoints);
    return 1;
}

