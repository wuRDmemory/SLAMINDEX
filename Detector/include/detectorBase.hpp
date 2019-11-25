#pragma once

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

struct MKeyPoint {
    Point2f mPix;
    float mOrient;
    float mResponse;
    int   mLevel;

    MKeyPoint(Point2f px, float orient, float response, int level): 
    mPix(px), mOrient(orient), mResponse(response), mLevel(level) {}
};

struct Cell {
    cv::Point2f px;
    float response;
    int   level;

    Cell(): response(-1), level(-1), px(-1, -1) {}    
};

class DetectorBase {
public:
    DetectorBase(int width, int height, int cellSize, int level): 
        mBorder(3), mWidth(width), mHeight(height), 
        mCellSize(cellSize), mLevels(level),
        mRows(height/cellSize), mCols(width/cellSize) {
            mCells.resize(mRows*mCols, Cell());
        };

    virtual ~DetectorBase() {}

    /** images   : image pyramid     
     ** keyPoints: result key points 
     ** levels   : key points levels
     **/
    virtual bool detect(vector<Mat>& images, vector<MKeyPoint>& keyPoints) = 0;

protected:
    int mBorder;
    int mCellSize, mLevels;
    int mRows,  mCols;
    int mWidth, mHeight;

    vector<Cell> mCells;
};

