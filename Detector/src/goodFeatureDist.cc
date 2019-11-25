#include "goodFeatureDist.hpp"

GoodFeatureDist::GoodFeatureDist(int width, int height, int cellSize, int levels):
    DetectorBase(width, height, cellSize, levels) {

    mCols = mWidth  / mCellSize;
    mRows = mHeight / mCellSize;

    mROIs.reserve(mCellSize*mCellSize);
    for (int i = 0; i < mCellSize; i++)
    for (int j = 0; j < mCellSize; j++) {
        Point tl((i+0)*mCols-0, (j+0)*mRows-0);
        Point br((i+1)*mCols-1, (j+1)*mRows-1);
        mROIs.push_back(Rect(tl, br));
    }

    mOccupied.resize(mRows*mCols, false);
}

GoodFeatureDist::~GoodFeatureDist() {
    mOccupied.clear();
}

bool GoodFeatureDist::detect(vector<Mat>& images, vector<MKeyPoint>& keyPoints) {
    assert(images.size() == mLevels);
    
    Mat& image = images[0];
    for (int i = 0; i < mROIs.size(); i++) {
        Rect& rectRoi = mROIs[i];
        Point rectTL  = rectRoi.tl();
        Point rectBR  = rectRoi.br();
        Mat   roi     = image.colRange(rectTL.x, rectBR.x).rowRange(rectTL.y, rectBR.y);

        vector<Point2f> corners;
        goodFeaturesToTrack(roi, corners, 10, 0.1, 5);
        for (int j = 0; j < corners.size(); j++) {
            cv::Point2f px = corners[j] + cv::Point2f(rectTL.x, rectTL.y);

            if (px.x < mBorder or px.x > mWidth  - mBorder or 
                px.y < mBorder or px.y > mHeight - mBorder) {
                continue;
            }

            int x = (int)px.x / mCellSize;
            int y = (int)px.y / mCellSize;
            if (mOccupied[y*mCols + x]) {
                continue;
            }

            mOccupied[y*mCols + x] = true;
            keyPoints.push_back(MKeyPoint(px, 0, 1, 0));
        }
    }

    cout << "Detect " << keyPoints.size() << " Key Points" << endl;

    return true;
}
