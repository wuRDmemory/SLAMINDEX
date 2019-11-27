#include "orbFeature.hpp"

bool OctNode::split(OctNode* n1, OctNode* n2, OctNode* n3, OctNode* n4) {
    const int w = mRect.width/2;
    const int h = mRect.height/2;
    const int x = mRect.tl().x;
    const int y = mRect.tl().y;

    n1->mRect = Rect(Point(x+0, y+0), Point(x+w*1-1, y+1*h-1));
    n1->mPts.clear();
    n2->mRect = Rect(Point(x+w, y+0), Point(x+w*2-1, y+1*h-1));
    n2->mPts.clear();
    n3->mRect = Rect(Point(x+0, y+h), Point(x+w*1-1, y+2*h-1));
    n3->mPts.clear();
    n4->mRect = Rect(Point(x+w, y+h), Point(x+w*2-1, y+2*h-1));
    n4->mPts.clear();

    OctNode* vNodes[] = {n1, n2, n3, n4};
    for (KeyPoint& kp: mPts) {
        const int px = kp.pt.x - x;
        const int py = kp.pt.y - y;
        
        const int dx = px/w;
        const int dy = py/h;
        vNodes[dx + 2*dy]->mPts.push_back(kp);
    }

    return true;
}

orbFeature::orbFeature(int width, int height, int cellSize, int levels, int ftrNum, int threshold):
    DetectorBase(width, height, cellSize, levels), mThreshold(threshold), mMaxFtrNum(ftrNum) {
    /*   calculate each level key point number   */
    mLevelFeatureCnt.resize(levels);

    const float invFactor = 0.5f;
    mLevelFeatureCnt[0]   = int(ftrNum*(1 - 0.5f)/(1 - pow(invFactor, levels)));
    for (int i = 1; i < mLevels; i++) {
        mLevelFeatureCnt[i] = invFactor*mLevelFeatureCnt[i-1];
    }
}

bool orbFeature::detect(vector<Mat>& images, vector<MKeyPoint>& keyPoints) {
    assert(images.size() == mLevels);

    /*  fetch all potential point in each level  */
    for (int k = 0; k < mLevels; k++) {
        const int w = images[k].cols;
        const int h = images[k].rows;

        const int nW = w/mCellSize;
        const int nH = h/mCellSize;

        vector<KeyPoint> distKeyPoints;
        distKeyPoints.reserve(mMaxFtrNum*10);

        for (int i = 0; i < nH; i++)
        for (int j = 0; j < nW; j++) {
            Point tl((i + 0)*nW, (i + 0)*nH - 1);
            Point br((i + 1)*nW, (i + 1)*nH - 1);
            
            vector<KeyPoint> kps;
            FAST(images[k].colRange(tl.x, br.x).rowRange(tl.y, br.y), kps, mThreshold);

            for (int i = 0; i < kps.size(); i++) {
                distKeyPoints.push_back(kps[i]);
            }
        }

        /*   distribute them  */
        distributeOctTree(distKeyPoints, Point(0, 0), Point(w, h), mLevelFeatureCnt[k]);
    }
}

bool orbFeature::distributeOctTree(vector<KeyPoint>& distKeyPoints, Point tl, Point br, int predKPCnt) {
    int initSeq = int((br.x - tl.x)/(br.y - tl.y));
    
    const float xgap = static_cast<float>(br.x - tl.x)/initSeq;

    list<OctNode>    listNodes;
    vector<OctNode*> nodePtrs;
    nodePtrs.resize(initSeq);


    /*  初始的根节点  */
    for (int i = 0; i < initSeq; i++) {
        OctNode node;
        const int W = (br.x - tl.x)/initSeq;
        const int H = (br.y - tl.y)/initSeq;

        Point2f p1(br.x, br.y), p2(W-1, H-1);
        node.mRect = Rect(p1, p2);

        listNodes.push_back(node);
        nodePtrs[i] = &listNodes.back();
    }

    for (KeyPoint& kp : distKeyPoints) {
        nodePtrs[kp.pt.x / xgap]->mPts.push_back(kp);
    }

    /*   广度优先遍历   */
    while(!listNodes.empty()) {
        OctNode node = listNodes.front();
        listNodes.pop_front();

        /*   split   */
        OctNode n1, n2, n3, n4;
        node.split(&n1, &n2, &n3, &n4)
    }

}