#include "orbFeature.hpp"

bool OctNode::split(OctNode& n1, OctNode& n2, OctNode& n3, OctNode& n4) {
    const int w = mRect.width /2;
    const int h = mRect.height/2;
    const int x = mRect.tl().x;
    const int y = mRect.tl().y;

    n1.mRect = Rect(x+0, y+0, w, h);
    n1.mPts.clear();
    n2.mRect = Rect(x+w, y+0, w, h);
    n2.mPts.clear();
    n3.mRect = Rect(x+0, y+h, w, h);
    n3.mPts.clear();
    n4.mRect = Rect(x+w, y+h, w, h);
    n4.mPts.clear();

    OctNode* vNodes[] = {&n1, &n2, &n3, &n4};
    for (KeyPoint& kp: mPts) {
        const float px = kp.pt.x;
        const float py = kp.pt.y;

        if (px < x + w) {
            if (py < y + h) {
                n1.mPts.push_back(kp);
            } else {
                n3.mPts.push_back(kp);
            }
        } else if (py < y + h) {
            n2.mPts.push_back(kp);
        } else {
            n4.mPts.push_back(kp);
        }
    }

    if (n1.mPts.size() == 1) {
        n1.mNoMore = true;
    }
    if (n2.mPts.size() == 1) {
        n2.mNoMore = true;
    }
    if (n3.mPts.size() == 1) {
        n3.mNoMore = true;
    }
    if (n4.mPts.size() == 1) {
        n4.mNoMore = true;
    }
    return true;
}

ORBFeature::ORBFeature(int width, int height, int cellSize, int levels, int ftrNum, int threshold):
    DetectorBase(width, height, cellSize, levels), mThreshold(threshold), mMaxFtrNum(ftrNum) {
    /*   calculate each level key point number   */
    mLevelFeatureCnt.resize(levels);

    const float invFactor = 0.5f;
    mLevelFeatureCnt[0]   = int(ftrNum*(1 - 0.5f)/(1 - pow(invFactor, levels)));
    for (int i = 1; i < mLevels; i++) {
        mLevelFeatureCnt[i] = invFactor*mLevelFeatureCnt[i-1];
    }
}

bool ORBFeature::detect(vector<Mat>& images, vector<MKeyPoint>& keyPoints) {
    assert(images.size() == mLevels);

    keyPoints.clear();
    /*  fetch all potential point in each level  */
    for (int k = 0; k < mLevels; k++) {
        const int w = images[k].cols;
        const int h = images[k].rows;

        const int nW = w/mCellSize;
        const int nH = h/mCellSize;

        vector<KeyPoint> distKeyPoints;
        distKeyPoints.reserve(mMaxFtrNum*10);

        for (int i = 0; i < mCellSize; i++)
        for (int j = 0; j < mCellSize; j++) {
            Point tl((i + 0)*nW, (j + 0)*nH);
            Point br((i + 1)*nW, (j + 1)*nH);
            
            vector<KeyPoint> kps;
            FAST(images[k].colRange(tl.x, br.x).rowRange(tl.y, br.y), kps, mThreshold);

            for (int i = 0; i < kps.size(); i++) {
                KeyPoint kpt = kps[i];
                Point2f   pt = kpt.pt + Point2f(tl);
                pt *= (1<<k);
                if (pt.x < mBorder          || pt.y < mBorder          || 
                    pt.x > mWidth - mBorder || pt.y > mHeight - mBorder) {
                    continue;
                }

                kps[i].pt = pt;
                distKeyPoints.push_back(kps[i]);
            }
        }

        cout << "level: " << k << "  detect  " << distKeyPoints.size() << endl;
        /*   distribute them  */
        vector<Point2f> vkeyPoints;
        distributeOctTree(images[0], distKeyPoints, vkeyPoints, Point(0, 0), Point(mWidth, mHeight), mLevelFeatureCnt[k]);

        for (int i = 0; i < vkeyPoints.size(); i++) {
            keyPoints.push_back(MKeyPoint(vkeyPoints[i], 0, 0, k));
        }
    }
}

bool ORBFeature::distributeOctTree(Mat& image, vector<KeyPoint>& distKeyPoints, vector<Point2f>& keyPoints, Point tl, Point br, int predKPCnt) {
    int initSeq = int((br.x - tl.x)/(br.y - tl.y));
    
    const float xgap = static_cast<float>(br.x - tl.x)/initSeq;

    list<OctNode> listNodes;
    
    {   /*  初始的根节点  */
        vector<OctNode*> nodePtrs;
        nodePtrs.resize(initSeq);

        for (int i = 0; i < initSeq; i++) {
            OctNode node;
            const int W = (br.x - tl.x)/initSeq;
            const int H = (br.y - tl.y)/initSeq;

            // adaptive
            node.mRect = Rect(i*W, 0, W, H);
            listNodes.push_back(node);

            nodePtrs[i] = &listNodes.back();
        }

        for (KeyPoint& kp : distKeyPoints) {
            nodePtrs[kp.pt.x / xgap]->mPts.push_back(kp);
        }
    }

    for (auto iter = listNodes.begin(); iter != listNodes.end();) {
        if (iter->mPts.empty()) {
            iter = listNodes.erase(iter);
        } else if (iter->mPts.size() == 1) {
            iter->mNoMore = true;
        } else {
            iter->mNoMore = false;
            iter++;
        }
    }

    vector<pair<int, pair<OctNode*, list<OctNode>::iterator>>> leafNodes;
    leafNodes.reserve(initSeq*4);

    int  expandNodeCnt = 0, prevCnt = 0;

    bool stop = false;
    /*   广度优先遍历   */
    while(!stop) {
        // recode last count
        prevCnt = listNodes.size();
        
        leafNodes.clear();

        expandNodeCnt = 0;

        auto it    = listNodes.begin();
        while (it != listNodes.end()) {
            if (it->mNoMore) {
                it++;
                continue;
            }

            /*   split   */
            OctNode n1, n2, n3, n4;
            it->split(n1, n2, n3, n4);

            /*   把分割好的叶子节点添加到节点列表中   */
            /*   把需要继续分割的叶子节点放入到leafsnodes中，主要用于排序   */
            if (n1.mPts.size() > 0) {
                // 注意一定要插入到队列的前面
                listNodes.push_front(n1);
                if (n1.mPts.size() > 1) {
                    expandNodeCnt++;
                    leafNodes.emplace_back(n1.mPts.size(), make_pair(&listNodes.front(), listNodes.begin()));
                }
            }

            if (n2.mPts.size() > 0) {
                // 注意一定要插入到队列的前面
                listNodes.push_front(n2);
                if (n2.mPts.size() > 1) {
                    expandNodeCnt++;
                    leafNodes.emplace_back(n2.mPts.size(), make_pair(&listNodes.front(), listNodes.begin()));
                }
            }

            if (n3.mPts.size() > 0) {
                // 注意一定要插入到队列的前面
                listNodes.push_front(n3);
                if (n3.mPts.size() > 1) {
                    expandNodeCnt++;
                    leafNodes.emplace_back(n3.mPts.size(), make_pair(&listNodes.front(), listNodes.begin()));
                }
            }

            if (n4.mPts.size() > 0) {
                // 注意一定要插入到队列的前面
                listNodes.push_front(n4);
                if (n4.mPts.size() > 1) {
                    expandNodeCnt++;
                    leafNodes.emplace_back(n4.mPts.size(), make_pair(&listNodes.front(), listNodes.begin()));
                }
            }

            it = listNodes.erase(it);
        }

        int N = static_cast<int>(listNodes.size());
        if (N >= predKPCnt or N == prevCnt) {
            /*   如果叶子节点不再增加，或者已经到达去往的   */
            stop = true;
        } else if (listNodes.size() + 3*expandNodeCnt >= predKPCnt) {
            /*   预测的数量已经到达预期的数量了   */
            while (!stop) {
                prevCnt = listNodes.size();
                // 
                vector<pair<int, pair<OctNode*, list<OctNode>::iterator>>> prevLeafNodes = leafNodes;
                leafNodes.clear();

                sort(prevLeafNodes.begin(), prevLeafNodes.end(), [](const pair<int, pair<OctNode*, list<OctNode>::iterator>>& a, const pair<int, pair<OctNode*, list<OctNode>::iterator>>& b) {
                    return a.first > b.first;
                });

                for (int i = 0; i < prevLeafNodes.size(); i++) {
                    OctNode* node = prevLeafNodes[i].second.first;
                    list<OctNode>::iterator& listIter = prevLeafNodes[i].second.second;
                    
                    OctNode n1, n2, n3, n4;
                    node->split(n1, n2, n3, n4);

                    if (n1.mPts.size() > 0) {
                        // 注意一定要插入到队列的前面
                        listNodes.push_front(n1);
                        if (n1.mPts.size() > 1) {
                            expandNodeCnt++;
                            leafNodes.emplace_back(n1.mPts.size(), make_pair(&listNodes.front(), listNodes.begin()));
                        }
                    }

                    if (n2.mPts.size() > 0) {
                        // 注意一定要插入到队列的前面
                        listNodes.push_front(n2);
                        if (n2.mPts.size() > 1) {
                            expandNodeCnt++;
                            leafNodes.emplace_back(n2.mPts.size(), make_pair(&listNodes.front(), listNodes.begin()));
                        }
                    }

                    if (n3.mPts.size() > 0) {
                        // 注意一定要插入到队列的前面
                        listNodes.push_front(n3);
                        if (n3.mPts.size() > 1) {
                            expandNodeCnt++;
                            leafNodes.emplace_back(n3.mPts.size(), make_pair(&listNodes.front(), listNodes.begin()));
                        }
                    }

                    if (n4.mPts.size() > 0) {
                        // 注意一定要插入到队列的前面
                        listNodes.push_front(n4);
                        if (n4.mPts.size() > 1) {
                            expandNodeCnt++;
                            leafNodes.emplace_back(n4.mPts.size(), make_pair(&listNodes.front(), listNodes.begin()));
                        }
                    }

                    listNodes.erase(listIter);
                    if (listNodes.size() >= predKPCnt) {
                        break;
                    }
                }

                int N = static_cast<int>(listNodes.size());
                if (N == prevCnt or N >= predKPCnt) {
                    stop = true;
                }
            }
        }
    }

    Mat originShow = image.clone();
    Mat distShow   = image.clone();

    cvtColor(originShow, originShow, cv::COLOR_GRAY2BGR);
    cvtColor(distShow  , distShow  , cv::COLOR_GRAY2BGR);

    cout << "listNode size: " << listNodes.size() << endl;
    keyPoints.clear();
    for (auto iter = listNodes.begin(); iter != listNodes.end(); iter++) {
        OctNode& node = *iter;
        KeyPoint p0   = node.mPts[0];
        
        circle(originShow, p0.pt, 2, cv::Scalar(255, 0, 0), 1);
        for (int i = 1; i < node.mPts.size(); i++) {
            KeyPoint pt = node.mPts[i];
            circle(originShow, pt.pt, 3, cv::Scalar(255, 0, 0), 1);

            if (pt.response > p0.response) {
                p0 = pt;
            }
        }
        rectangle(originShow, node.mRect, cv::Scalar(0, 0, 255), 1);

        circle(distShow, p0.pt, 3, cv::Scalar(0, 255, 0), 1);
        rectangle(distShow, node.mRect, cv::Scalar(0, 0, 255), 1);
        keyPoints.push_back(p0.pt);
    }

    imshow("originShow", originShow);
    imshow("distShow", distShow);
    waitKey();
    return true;
}