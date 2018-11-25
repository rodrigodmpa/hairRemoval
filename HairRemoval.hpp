#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdio>
#include <list>
#include <cmath>
#include <vector>
#include <algorithm>

using namespace cv;
using namespace std;

class HairRemoval{

    public:
        bool toGrayscale(Mat* in, Mat* out); //in é uma imagem com cor (3 canais) que resulta em um out grayscale (1 canal)
        double euclidianDistance(list<Point2d>& l); //linha reta de start até end
        
    private:
        const float bgr[3] = {0.114, 0.587, 0.299}; //pesos para converter para grayscale
};
