//#include <opencv2/core/core.hpp> 
//#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/opencv.hpp>
#include "HairRemoval.hpp"
#include <iostream>
#include <list>
#include <cmath>
#include <vector>
 
using namespace cv;
using namespace std;

int main()
{
    Mat mat_colored, mat_closed0,mat_closed45,mat_closed90,mat_closed; //Declaração da matriz da imagem colorida
    Mat kernel0,kernel45,kernel90; //Declaração dos kernels para fazer closing
    mat_colored = imread("./fig1.png",CV_LOAD_IMAGE_COLOR); //carrega arquivo
    
    mat_closed0 = Mat::zeros(mat_colored.rows,mat_colored.cols, CV_8UC3);
    mat_closed45 = Mat::zeros(mat_colored.rows,mat_colored.cols, CV_8UC3);
    mat_closed90 = Mat::zeros(mat_colored.rows,mat_colored.cols, CV_8UC3);
    mat_closed = Mat::zeros(mat_colored.rows,mat_colored.cols, CV_8UC3);

    //cria os kernels para fazer o fechamento
    kernel0 = Mat::zeros(1,13, CV_8UC1);
    kernel45 = Mat::zeros(9,9, CV_8UC1);
    kernel90 = Mat::zeros(13,1, CV_8UC1);

    
    // preenche os kernels com 1's
    for(int i=1;i<12;i++){
        kernel0.at<uchar>(0, i, 0) = 1;
        kernel90.at<uchar>(i, 0, 0) = 1;
    }

    for(int j=1;j<8;j++){
        for(int k=1;k<8;k++){
            if (j == k){
                kernel45.at<uchar>(j,k,0) = 1;
            }
        }
    }

    // cout << "M = " << endl << " " << kernel135 << endl << endl;
    // cout << "M = " << endl << " " << kernel0 << endl << endl;
    // cout << "M = " << endl << " " << kernel45 << endl << endl;
    // cout << "M = " << endl << " " << kernel90 << endl << endl;
    
    // Aplica o fechamento com os 4 kernels
    morphologyEx(mat_colored, mat_closed0,cv::MORPH_CLOSE, kernel0);
    morphologyEx(mat_colored, mat_closed45,cv::MORPH_CLOSE, kernel45);
    morphologyEx(mat_colored, mat_closed90,cv::MORPH_CLOSE, kernel90);

    for (int i=0; i< mat_colored.rows; i++){
        for (int j=0 ;j< mat_colored.cols ; j++){  
            for (int k=0 ;k< 3 ; k++){  
                mat_closed.at<Vec3b>(i,j)[k] = max(max(mat_closed0.at<Vec3b>(i, j) [k], mat_closed45.at<Vec3b>(i, j)[k]), mat_closed90.at<Vec3b>(i, j) [k]);
            }
        }
    }

    //mostra imagem com pelos e as imagens com closing
    imshow("fechamento - kernel 0", mat_closed0);
    imshow("fechamento - kernel 45", mat_closed45);
    imshow("fechamento - kernel 90", mat_closed90);
    imshow("Imagem Final", mat_closed);
    imshow("Imagem Original", mat_colored);
    waitKey(0);
    destroyAllWindows();


    // //lista de pontos do trajeto
    // list<Point2d> path;

    // //Point pega (x,y) e não (linha,coluna), o canto superior esquerdo é (0,0) 
    // Point2d start = Point(415,260);
    // Point2d end = Point(1000,815);
    
    // //instancia do editor que faz transformações na imagem
    // HairRemoval myEditor;
    
    // //As matrizes foram criadas?
    // if( !mat_colored.data || !mat_grayscale.data || !mat_hist.data) {
    //     cout << "mat invalido!" << endl;
    //     return -1;
    // }
    
    // myEditor.toGrayscale(&mat_colored,&mat_grayscale); //RGB -> Grayscale


    // cout << "Distancia em linha reta (em pixels) = " << myEditor.euclidianDistance(path) << endl;
    
    // //cria os arquivos
    // imwrite("mars_path.bmp",mat_colored);
    // imwrite("mars_gray.bmp",mat_grayscale);
    // imwrite("mars_hist.bmp",mat_hist);
    
    // //mostra janela
    // namedWindow("Colored_Path",CV_WINDOW_AUTOSIZE); 
    // imshow("Colored_Path", mat_colored);
     
 
    // waitKey(0);
    // destroyAllWindows();

    return 0;
}
