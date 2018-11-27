//#include <opencv2/core/core.hpp> 
//#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/opencv.hpp>
#include "HairRemoval.hpp"
#include <iostream>
#include <math.h>
#include <list>
#include <cmath>
#include <vector>
#include <algorithm>
#define T 47
 
using namespace cv;
using namespace std;

struct lines{
    /*
     * 0 - 0º
     * 1 - 45º
     * 2 - 90º
     * 3 - 135º
     */
    int lin[4];
};

void iniciaLinhas(struct lines *linhas){
    for(int i = 0 ; i < 4 ; i++){
        linhas->lin[i] = 1;
    }
}

bool medianFiltering(Mat *in, Mat *out,Mat *hair, int kernel){

    //argumentos devem conter dados
    if( !in->data || !out->data ) {
        printf("Entrada do filtro de mediana invalida!\n");
        return false;
    }

    if(kernel %2 == 0){
        printf("Tamanho da janela deve ser impar.\n");
        return false;
    }

    int borderSize = (kernel-1)/2;
    vector<uchar> kernelValues;

    Mat buffer;
    in->copyTo(buffer);

    copyMakeBorder(buffer,buffer,borderSize,borderSize,borderSize,borderSize,BORDER_DEFAULT); //espelha a borda
    
    //passa o kernel na imagem, e o centro passa a ser a mediana. (uso a função sort)
    for(int x = borderSize ; x < in->cols ; x++){
        for(int y = borderSize ; y < in->rows ; y++){
            
            for(int kernelX = x-borderSize ; kernelX <= x+borderSize ; kernelX++ ){
                for(int kernelY = y - borderSize ; kernelY <= y+borderSize ; kernelY++){
                    kernelValues.push_back(buffer.at<uchar>(Point(kernelX,kernelY)));
                }
            }
            sort(kernelValues.begin(), kernelValues.end());

            if(hair->at<uchar>(Point(x,y)) == 255){ // Só faz a mediana se for região de cabelo
                out->at<uchar>(Point(x,y)) = kernelValues[(kernel*kernel -1)/2];
            }
            kernelValues.clear();


        }
    }
    
    return true;
}

int main(int argc , char *argv[])
{
//    if(argc < 3){
//        cout << "ERRO\n";
//    }
    Mat mat_colored, mat_out; //Declaração da matriz da imagem colorida
    Mat mat_hair[3], mat_closed0[3],mat_closed45[3],mat_closed90[3],mat_closed[3]; // Matrizes auxiliares para o fechamento e para os cabelos
    Mat kernel0,kernel45,kernel90; //Declaração dos kernels para fazer closing
    Mat channels[3]; // Matriz dos canais BRG separados
    Mat generalized_grayscale[3], mat_hair_final,mat_hair_final_grossa,mat_mediana;// Matrizes necessarias para calculo da matriz de cabelo final binaria

    mat_colored = imread("./fig1.png",CV_LOAD_IMAGE_COLOR); //carrega arquivo

    split(mat_colored,channels); // Separa a matriz colorida em 3 canais BGR

    // Inicializa matrizes com 0
    mat_mediana = Mat::zeros(mat_colored.rows,mat_colored.cols, CV_8UC1); // Matriz que contem a mediana
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

    //Kernel para dilatacao
    Mat kernelD = Mat::ones(4,4, CV_8UC1);
    
    int testK[9] = {0,1,0,
                    1,1,1,
                    0,1,0};
    Mat kernelT = Mat(3, 3, CV_8U, testK);


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
            if(generalized_grayscale[0].at<uchar>(i,j) < T){
                mat_hair[0].at<uchar>(i,j) = 0;
            }
            else{
                mat_hair[0].at<uchar>(i,j) = 255;
            }
            if(generalized_grayscale[1].at<uchar>(i,j) < T){
                mat_hair[1].at<uchar>(i,j) = 0;
            }
            else{
                mat_hair[1].at<uchar>(i,j) = 255;
            }
            if(generalized_grayscale[2].at<uchar>(i,j) < T){
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

    mat_hair_final = 255 - mat_hair_final;
    
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
    
    /* Removendo ruídos de mat_hair_final*/


    imwrite("pelo1.png", mat_hair_final);
    struct lines linhas;

    int posX = 0;
    int posY = 0;

    int largura = atoi(argv[1]);
    int comprimento = atoi(argv[2]);

    Mat test = mat_hair_final.clone();

    Point2d candxP = Point(0,0), candxM = Point(0,0), candyP = Point(0,0), candyM = Point(0,0);

    for(int y = 0 ; y < mat_hair_final.rows ; y++){
        for(int x = 0 ; x < mat_hair_final.cols ; x++){

            if(mat_hair_final.at<uchar>(Point(x,y)) == 0){

               iniciaLinhas(&linhas);

               //Em 0º
               posX = x;
               posY = y;
               candxP = Point(posX,posY);
               while(posX < mat_hair_final.cols){
                   posX++;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[0]++;
                       candxP = Point(posX,posY);
                   } else{
                       break;
                   } 
               }

               //Em 45º
               posX = x;
               posY = y;
               while((posX < mat_hair_final.cols) && (posY < mat_hair_final.rows)){
                   posX++;
                   posY++;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[1]++;
                   } else{
                       break;
                   } 
               }

               //Em 90º
               posX = x;
               posY = y;
               candyP = Point(posX,posY);
               while(posY < mat_hair_final.rows){
                   posY++;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[2]++;
                        candyP = Point(posX,posY);
                   } else{
                       break;
                   } 
               }
               //Em 135º

               posX = x;
               posY = y;
               while((posX >= 0) && (posY < mat_hair_final.rows)){
                   posX--;
                   posY++;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[3]++;
                   } else{
                       break;
                   } 
               }
               //Em 180º
               posX = x;
               posY = y;
               candxM = Point(posX,posY);
               while(posX >= 0){
                   posX--;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[0]++;
                       candxM = Point(posX,posY);
                   } else{
                       break;
                   } 
               }
               //Em 225º
               posX = x;
               posY = y;
               while((posX >= 0) && (posY >= 0)){
                   posX--;
                   posY--;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[1]++;
                   } else{
                       break;
                   } 
               }
               //Em 270º
               posX = x;
               posY = y;
               candyM = Point(posX,posY);
               while(posY >= 0){
                   posY--;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[2]++;
                       candyM = Point(posX,posY);
                   } else{
                       break;
                   } 
               }
               //Em 315º
               posX = x;
               posY = y;
               while((posX < mat_hair_final.cols) && (posY >= 0)){
                   posX++;
                   posY--;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[3]++;
                   } else{
                       break;
                   } 
               }
                              
               //acha maior
               int sortLinhas[4] = {linhas.lin[0], linhas.lin[1], linhas.lin[2], linhas.lin[3]};
               sort(sortLinhas,sortLinhas+4, greater<int>());
               
//               cout << x << "," << y << " / " << sortLinhas[0] << " - " << sortLinhas[1] << endl;
               if(!(sortLinhas[0] > comprimento && sortLinhas[1] < largura)){
                    mat_hair_final.at<uchar>(Point(x,y)) = 255;
               }
                
               // INTERPOLAÇÃO
               //dá o valor para os candidatos
               int param = 5;
               if(candxP.x > (mat_hair_final.cols - (param+1))){
                    candxP.x = mat_hair_final.cols   -1;
               } else{
                    candxP.x += param;
               }

               if(candxM.x < param){
                    candxM.x = 0;
               } else{
                    candxP.x -= param;
               }

               if(candyP.y > (mat_hair_final.rows - (param+1))){
                    candyP.y = mat_hair_final.rows - 1;
               } else{
                    candyP.y += param;
               }

               if(candyM.y < param){
                    candyM.y = 0;
               } else{
                    candyP.y -= param;
               }

               Point2d candFinal1, candFinal2;
               int bgrBilinear[3] = {0,0,0};
               //escolhe os candidatos para a interpolação
               
               //Se linha horizontal é maior que a linha vertical
               //Escolhe candidatos verticais e vice versa
               if(linhas.lin[0] > linhas.lin[2]){
                   candFinal1 = candyP;
                   candFinal2 = candyM;
               } else{
                   candFinal1 = candxP;
                   candFinal2 = candyM;
               }

               bgrBilinear[0] = 
                    channels[0].at<uchar>(candFinal2)*sqrt((pow(candFinal1.x-x,2)+pow(candFinal1.y-y,2))/(pow(candFinal2.x - candFinal1.x,2)+pow(candFinal2.y - candFinal1.y,2))) + 
                    channels[0].at<uchar>(candFinal1)*sqrt((pow(candFinal2.x-x,2)+pow(candFinal2.y-y,2))/(pow(candFinal2.x - candFinal1.x,2)+pow(candFinal2.y - candFinal1.y,2)));

               bgrBilinear[1] = 
                    (int)channels[1].at<uchar>(candFinal2)*sqrt((pow(candFinal1.x-x,2)+pow(candFinal1.y-y,2))/(pow(candFinal2.x - candFinal1.x,2)+pow(candFinal2.y - candFinal1.y,2))) + 
                    (int)channels[1].at<uchar>(candFinal1)*sqrt((pow(candFinal2.x-x,2)+pow(candFinal2.y-y,2))/(pow(candFinal2.x - candFinal1.x,2)+pow(candFinal2.y - candFinal1.y,2)));

               bgrBilinear[2] = 
                    (int)channels[2].at<uchar>(candFinal2)*sqrt((pow(candFinal1.x-x,2)+pow(candFinal1.y-y,2))/(pow(candFinal2.x - candFinal1.x,2)+pow(candFinal2.y - candFinal1.y,2))) + 
                    (int)channels[2].at<uchar>(candFinal1)*sqrt((pow(candFinal2.x-x,2)+pow(candFinal2.y-y,2))/(pow(candFinal2.x - candFinal1.x,2)+pow(candFinal2.y - candFinal1.y,2)));
                    
                    //corrige valores negativos
                    if(bgrBilinear[0] < 0){
                        bgrBilinear[0] = (int)(channels[0].at<uchar>(candFinal1)+ channels[0].at<uchar>(candFinal2))/2;
                        bgrBilinear[1] = (int)(channels[1].at<uchar>(candFinal1)+ channels[1].at<uchar>(candFinal2))/2;
                        bgrBilinear[2] = (int)(channels[2].at<uchar>(candFinal1)+ channels[2].at<uchar>(candFinal2))/2;
                    }
                    
                   //substitui pixels (está errado por enquanto)
                   channels[0].at<uchar>(Point(x,y)) = bgrBilinear[0];
                   channels[1].at<uchar>(Point(x,y)) = bgrBilinear[1];
                   channels[2].at<uchar>(Point(x,y)) = bgrBilinear[2];
                   //deixa o cabelo branco
                   // channels[0].at<uchar>(Point(x,y)) = 255;
                   // channels[1].at<uchar>(Point(x,y)) = 255;
                   // channels[2].at<uchar>(Point(x,y)) = 255;

                  //  cout << "BGR: " << (int) channels[0].at<uchar>(Point(x,y)) << " " << bgrBilinear[0] << " , " << bgrBilinear[1] << " , " << bgrBilinear[2] << endl;
                                
            }
        }
    
    }
     
    // A matriz de cabelos que será dilatada é invertida para que a dilatação seja feita corretamente. 
    mat_hair_final_grossa = 255 - mat_hair_final;

//    dilate(mat_hair_final,mat_hair_final, kernelD);
//    test = 255 - test;
//    morphologyEx(mat_hair_final, mat_hair_final,cv::MORPH_OPEN, kernelD);
//    morphologyEx(mat_hair_final, mat_hair_final,cv::MORPH_CLOSE, kernelD);
//    morphologyEx(test, mat_hair_final,cv::MORPH_OPEN, kernelD);
//    morphologyEx(test, mat_hair_final,cv::MORPH_CLOSE, kernelD);
//    erode(test,test, kernelD);


//    mat_hair_final = mat_hair_final + test;
//    mat_hair_final = 255 - mat_hair_final;


    // Dilata a matriz de pelos para que estes fiquem mais grossos, utilizo aqui um kernel de 4x4 com 1's
    dilate(mat_hair_final_grossa,mat_hair_final_grossa, kernelD);
    mat_hair_final_grossa = 255 - mat_hair_final_grossa;

    
    medianFiltering(&channels[0],&channels[0],&mat_hair_final_grossa,5);
    medianFiltering(&channels[1],&channels[1],&mat_hair_final_grossa,5);
    medianFiltering(&channels[2],&channels[2],&mat_hair_final_grossa,5);
    
    Mat matFinal;
    merge(channels,3,matFinal);
    imwrite("imgFinal.png",matFinal);


    
//    imshow("Imagem Original", mat_colored);
//    imshow("Mediana Pelos", mat_mediana);
    // imshow("Sem cabelo", mat_out);
    imshow("Cabelos", mat_hair_final);
//    imshow("Cabelos dilatados", mat_hair_final_grossa);
    waitKey(0);
    destroyAllWindows();
    imwrite("pelo1_corrigido.png", mat_hair_final);
    return 0;
}
