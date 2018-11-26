//#include <opencv2/core/core.hpp> 
//#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/opencv.hpp>
#include "HairRemoval.hpp"
#include <iostream>
#include <list>
#include <cmath>
#include <vector>
#define T 24
 
using namespace cv;
using namespace std;

int main()
{
    Mat mat_colored, mat_out; //Declaração da matriz da imagem colorida
    Mat mat_hair[3], mat_closed0[3],mat_closed45[3],mat_closed90[3],mat_closed[3]; // Matrizes auxiliares para o fechamento e para os cabelos
    Mat kernel0,kernel45,kernel90; //Declaração dos kernels para fazer closing
    Mat channels[3]; // Matriz dos canais BRG separados
    Mat generalized_grayscale[3], mat_hair_final;// Matrizes necessarias para calculo da matriz de cabelo final binaria

    mat_colored = imread("./fig1.png",CV_LOAD_IMAGE_COLOR); //carrega arquivo

    split(mat_colored,channels); // Separa a matriz colorida em 3 canais BGR

    // Inicializa matrizes com 0
    for (int i =0;i<3;i++){
        mat_closed0[i] = Mat::zeros(mat_colored.rows,mat_colored.cols, CV_8UC1); // Vetor de 3 matrizes (BGR) para o grau 0
        mat_closed45[i] = Mat::zeros(mat_colored.rows,mat_colored.cols, CV_8UC1);// Vetor de 3 matrizes (BGR) para o grau 45
        mat_closed90[i] = Mat::zeros(mat_colored.rows,mat_colored.cols, CV_8UC1);// Vetor de 3 matrizes (BGR) para o grau 90
        mat_closed[i] = Mat::zeros(mat_colored.rows,mat_colored.cols, CV_8UC1); // Vetor de 3 matrizes (BGR) para resultado final dos fechamentos
        mat_hair[i] = Mat::zeros(mat_colored.rows,mat_colored.cols, CV_8UC1); // Vetor de 3 matrizes (BGR) para os cabelos
    }
    
    //cria os kernels para fazer o fechamento
    kernel0 = Mat::zeros(1,13, CV_8UC1);
    kernel45 = Mat::zeros(9,9, CV_8UC1);
    kernel90 = Mat::zeros(13,1, CV_8UC1);

    // preenche os kernels com 1's
    for(int i=1;i<12;i++){
        kernel0.at<uchar>(0, i, 0) = 1;
        kernel90.at<uchar>(i, 0, 0) = 1;
    }

    // preenche os kernels com 1's
    for(int j=1;j<8;j++){
        for(int k=1;k<8;k++){
            if (j == k){
                kernel45.at<uchar>(j,k,0) = 1;
            }
        }
    }

    
    // Aplica o fechamento com os 3 kernels

    //Blue
    morphologyEx(channels[0], mat_closed0[0],cv::MORPH_CLOSE, kernel0);
    morphologyEx(channels[0], mat_closed45[0],cv::MORPH_CLOSE, kernel45);
    morphologyEx(channels[0], mat_closed90[0],cv::MORPH_CLOSE, kernel90);
    //Red
    morphologyEx(channels[1], mat_closed0[1],cv::MORPH_CLOSE, kernel0);
    morphologyEx(channels[1], mat_closed45[1],cv::MORPH_CLOSE, kernel45);
    morphologyEx(channels[1], mat_closed90[1],cv::MORPH_CLOSE, kernel90);
    //Green
    morphologyEx(channels[2], mat_closed0[2],cv::MORPH_CLOSE, kernel0);
    morphologyEx(channels[2], mat_closed45[2],cv::MORPH_CLOSE, kernel45);
    morphologyEx(channels[2], mat_closed90[2],cv::MORPH_CLOSE, kernel90);

    
    // For para calcular o maximo entre os fechamentos aplicados com 3 graus diferentes e para os 3 planos BGR
    for (int i=0; i< mat_colored.rows; i++){
        for (int j=0 ;j< mat_colored.cols ; j++){ 
            mat_closed[0].at<uchar>(i,j) = max(max(mat_closed0[0].at<uchar>(i,j), mat_closed45[0].at<uchar>(i,j)), mat_closed90[0].at<uchar>(i,j));
            mat_closed[1].at<uchar>(i,j) = max(max(mat_closed0[1].at<uchar>(i,j), mat_closed45[1].at<uchar>(i,j)), mat_closed90[1].at<uchar>(i,j));
            mat_closed[2].at<uchar>(i,j) = max(max(mat_closed0[2].at<uchar>(i,j), mat_closed45[2].at<uchar>(i,j)), mat_closed90[2].at<uchar>(i,j));
        }
    }

    // Calcula a diferença absoluta dos canais originais (channel) e dos canais com fechamento calculados no for acima (generalized_grayscale)
    absdiff(channels[0], mat_closed[0], generalized_grayscale[0]);
    absdiff(channels[1], mat_closed[1], generalized_grayscale[1]);
    absdiff(channels[2], mat_closed[2], generalized_grayscale[2]);


    // For para construir a mat_hair, que é binaria e definida pelo treshhold T
    for (int i=0; i< mat_colored.rows; i++){
        for (int j=0 ;j< mat_colored.cols ; j++){ 
            if(generalized_grayscale[0].at<uchar>(i,j) > T){
                mat_hair[0].at<uchar>(i,j) = 0;
            }
            else{
                mat_hair[0].at<uchar>(i,j) = 255;
            }
            if(generalized_grayscale[1].at<uchar>(i,j) > T){
                mat_hair[1].at<uchar>(i,j) = 0;
            }
            else{
                mat_hair[1].at<uchar>(i,j) = 255;
            }
            if(generalized_grayscale[2].at<uchar>(i,j) > T){
                mat_hair[2].at<uchar>(i,j) = 0;
            }
            else{
                mat_hair[2].at<uchar>(i,j) = 255;
            }
        }
    }

    // Junção das 3 channels de fechamento
    merge(mat_closed,3,mat_out);

    // União das 3 matrizes de cabelo B, G, R
    mat_hair_final = mat_hair[0] + mat_hair[1] + mat_hair[2];
    
    // Negativo da matriz de cabelos para que os pelos fiquem branco

    // for (int i=0; i< mat_colored.rows; i++){
    //     for (int j=0 ;j< mat_colored.cols ; j++){ 
    //         if(mat_hair_final.at<uchar>(i,j) == 255){
    //             mat_hair_final.at<uchar>(i,j) = 0;
    //         }
    //         else{
    //             mat_hair_final.at<uchar>(i,j) = 255;
    //         }
    //     }
    // }

    //mostra imagem com pelos e as imagens com closing
    
    // imshow("generalized_gray Blue", generalized_grayscale[0]);
    // imshow("generalized_gray Green", generalized_grayscale[0]);
    // imshow("generalized_gray Red", generalized_grayscale[0]);
    // imshow("Blue", mat_closed[0]);
    // imshow("Green", mat_closed[0]);
    // imshow("Red", mat_closed[0]);
    // imshow("Cabelo B", mat_hair[0]);
    // imshow("Cabelo G", mat_hair[1]);
    // imshow("Cabelo R", mat_hair[2]);
    
    imshow("Imagem Original", mat_colored);
    imshow("Sem cabelo", mat_out);
    imshow("Matriz Cabelos", mat_hair_final);
    waitKey(0);
    destroyAllWindows();
    // imwrite("sempelo1.jpg", mat_out);
    return 0;
}
