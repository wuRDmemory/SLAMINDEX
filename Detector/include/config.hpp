#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

struct Config {
public:
    int imageWidth;
    int imageHeight;
    int cellSize;
    int levels;
    int threshold;

    string imagePath;

    Config(const string configPath) {
        FileStorage file(configPath, FileStorage::READ);

        imagePath   = static_cast<string>(file["image_path"]);
        imageWidth  = static_cast<int>(file["imwidth"]);
        imageHeight = static_cast<int>(file["imheight"]);
        cellSize    = static_cast<int>(file["cellsize"]);
        levels      = static_cast<int>(file["pyrLevel"]);
        threshold   = static_cast<int>(file["intThreshold"]);

        cout << "image path  : " << imagePath   << endl;
        cout << "image width : " << imageWidth  << endl;
        cout << "image height: " << imageHeight << endl;
        cout << "cell  size  : " << cellSize    << endl;
        cout << "levels      : " << levels      << endl;
        cout << "threshold   : " << threshold   << endl;
    }
};


