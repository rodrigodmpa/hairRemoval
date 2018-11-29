//#include <opencv2/core/core.hpp> 
//#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/opencv.hpp>
#include <iostream>
#include <list>
#include <cmath>
#include <vector>
#include <algorithm>
// #define T 24
 
using namespace cv;
using namespace std;

//Struct que representam o tamanho das linhas que cruzam os pixels
//Será usada para percorrer
struct lines{
    /*
     * 0 - 0º
     * 1 - 45º
     * 2 - 90º
     * 3 - 135º
     */
    int lin[4];
};

//Seta todas as linhas da struct linhas para 1 (apenas o pixel atual)
void iniciaLinhas(struct lines *linhas){
    for(int i = 0 ; i < 4 ; i++){
        linhas->lin[i] = 1;
    }
}

//Filtro de mediana adaptado para só ser aplicado numa máscara hair
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

            if(hair->at<uchar>(Point(x,y)) == 0){ // Só faz a mediana se for região de cabelo
                out->at<uchar>(Point(x,y)) = kernelValues[(kernel*kernel -1)/2];
            }
            kernelValues.clear();


        }
    }
    
    return true;
}

int main(int argc , char *argv[])
{
    if(argc < 6){
        cout << "Erro no número de argumentos, devem ser 5:\n\n";
        cout << "1- largura maxima (use um valor grande 10-50)" << endl;
        cout << "2- Comprimento minimo (Só funcionou bem com valores 0-10)" << endl;
        cout << "3- Threshold de segmentação(20-50) " << endl;
        cout << "4- Quantos pixels de distancia pegar como candidatos da interpolação (use 1-30)" << endl;
        cout << "5- qual imagem (sem a extensão, ex: fig3)\n";
        exit(1);
    }

    //Argumentos (explicados acima)
    int largura = atoi(argv[1]);
    int comprimento = atoi(argv[2]);
    int T = atoi(argv[3]);
    int skip = atoi(argv[4]);
    string imagem = argv[5];


    Mat mat_colored, mat_out, mat_bilinear, matFinal; //Declaração da matriz da imagem colorida
    Mat mat_hair[3], mat_closed0[3],mat_closed45[3],mat_closed90[3],mat_closed[3]; // Matrizes auxiliares para o fechamento e para os cabelos
    Mat kernel0,kernel45,kernel90; //Declaração dos kernels para fazer closing
    Mat channels[3],channels_bilinear[3],channels_mediana[3]; // Matriz dos canais BRG separados
    Mat generalized_grayscale[3], mat_hair_final,mat_hair_final_grossa,mat_mediana;// Matrizes necessarias para calculo da matriz de cabelo final binaria

    mat_colored = imread("./" + imagem + ".png",CV_LOAD_IMAGE_COLOR); //carrega arquivo

    split(mat_colored,channels); // Separa a matriz colorida em 3 canais BGR

    channels[0].copyTo(channels_bilinear[0]);
    channels[1].copyTo(channels_bilinear[1]);
    channels[2].copyTo(channels_bilinear[2]);


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
    
    //inversão da matriz.
    mat_hair_final = 255 - mat_hair_final;
    
    //escreve o que temos até agora da segmentação num arquivo
    imwrite("output/pelo_fino_ruidoso.png", mat_hair_final);
    
    /* Removendo ruídos de mat_hair_final
     * A ideia é descartar pixels que não fazem parte de uma região preta fina e comprida (cabelo)*/

    struct lines linhas; //struct que armazena o tamanho das 4 linhas que cruzam um pixel
    //Variaveis que percorrem ao redor do pixel usadas para achar tamanho das linhas que cruzam o pixel
    int posX = 0;
    int posY = 0; 
    
    for(int y = 0 ; y < mat_hair_final.rows ; y++){
        for(int x = 0 ; x < mat_hair_final.cols ; x++){
            
            //se for cabelo
            if(mat_hair_final.at<uchar>(Point(x,y)) == 0){
               
               //inicia tamanho das linhas que cruzam o pixel para 1 (pixel atual)
               iniciaLinhas(&linhas);

               //Tamanho do segmento de 0º (calcula até sair do cabelo ou extrapolar a matriz)
               posX = x;
               posY = y;
               while(posX < mat_hair_final.cols){
                   posX++;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[0]++;
                   } else{
                       break;
                   } 
               }

               //Tamanho do segmento de 45º
               posX = x;
               posY = y;
               while((posX < mat_hair_final.cols) && (posY > 0)){
                   posX++;
                   posY--;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[1]++;
                   } else{
                       break;
                   } 
               }

               //Em 90º
               posX = x;
               posY = y;
               while(posY > 0 ){
                   posY--;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[2]++;
                   } else{
                       break;
                   } 
               }

               //Em 135º
               posX = x;
               posY = y;
               while((posX >= 0) && (posY >0)){
                   posX--;
                   posY--;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[3]++;
                   } else{
                       break;
                   } 
               }
               //Em 180º
               posX = x;
               posY = y;
               while(posX >= 0){
                   posX--;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[0]++;
                   } else{
                       break;
                   } 
               }
               //Em 225º
               posX = x;
               posY = y;
               while((posX >= 0) && (posY < mat_hair_final.rows)){
                   posX--;
                   posY++;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[1]++;
                   } else{
                       break;
                   } 
               }
               //Em 270º
               posX = x;
               posY = y;
               while(posY < mat_hair_final.rows){
                   posY++;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[2]++;
                   } else{
                       break;
                   } 
               }
               //Em 315º
               posX = x;
               posY = y;
               while((posX < mat_hair_final.cols) && (posY < mat_hair_final.rows)){
                   posX++;
                   posY++;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[3]++;
                   } else{
                       break;
                   } 
               }
                              
               //acha maior a maior linha ordenando os tamanhos de forma decrescente.
               int sortLinhas[4] = {linhas.lin[0], linhas.lin[1], linhas.lin[2], linhas.lin[3]};
               sort(sortLinhas,sortLinhas+4, greater<int>());
              
               // Condição para não descartar: a maior linha deve ser maior que o comprimento estabelecido
               // A segunda maior linha deve ser menor do que a largura estabelecida.
               if(!(sortLinhas[0] > comprimento && sortLinhas[1] < largura)){
                mat_hair_final.at<uchar>(Point(x,y)) = 255;
               }
                
            }
        }
    
    }
    
    /* Agora já temos uma segmentação sem ruídos,
     * passamos para a etapa de interpolação
     * 
     * A lógica é determinar se a linha horizontal é maior do que a vertical
     * e escolher pontos para interpolação que sejam perpendiculares à maior linha
     * Assim, realizamos a interpolação e substituimos o pixel.
     *
     * Faremos apenas uma análise vertical e horizontal
     * */

    //Pontos que são usados para determinar as bordas dos cabelos
    //candxP é o da extremidade Positiva do eixo X
    //candxM é o da extremidade negativa do eixo X
    //candyP é a extremidade de cima
    //candyM é o da extremidade de baixo
    Point2d candxP = Point(0,0), candxM = Point(0,0), candyP = Point(0,0), candyM = Point(0,0);

    //Armazenará os dois pontos candidatos finais para interpolação a partir dos 4 acima
    Point2d candFinal1, candFinal2;

    //Guarda valor final da interpolação
    int bgrBilinear[3] = {0,0,0};
    
    for(int y = 0 ; y < mat_hair_final.rows ; y++){
        for(int x = 0 ; x < mat_hair_final.cols ; x++){
            
            // se for cabelo
            if(mat_hair_final.at<uchar>(Point(x,y)) == 0){
               
               iniciaLinhas(&linhas);
               
               //determinar borda do segmento de 0º positivo e colocar em candxP
               posX = x;
               posY = y;
               while(posX < mat_hair_final.cols){
                   posX++;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[0]++;
                       candxP = Point(posX,posY);
                   } else{
                       break;
                   } 
               }

               //Em 90º
               posX = x;
               posY = y;
               while(posY > 0 ){
                   posY--;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[2]++;
                       candyP = Point(posX,posY);
                   } else{
                       break;
                   } 
               }

               //Em 180º
               posX = x;
               posY = y;
               while(posX >= 0){
                   posX--;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[0]++;
                       candxM = Point(posX,posY);
                   } else{
                       break;
                   } 
               }

               //Em 270º
               posX = x;
               posY = y;
               while(posY < mat_hair_final.rows){
                   posY++;
                   if(mat_hair_final.at<uchar>(Point(posX,posY)) == 0){
                       linhas.lin[2]++;
                       candyM = Point(posX,posY);
                   } else{
                       break;
                   } 
               }

               //Agora temos 4 pontos candidatos para interpolação
               //Agora somaremos eles com o valor skip para pegar pontos mais distantes 
               //Precisa de cuidado para não extrapolar a matriz
               
               if(candxP.x > (mat_hair_final.cols - (skip+1))){
                    candxP.x = mat_hair_final.cols - 1;
               } else{
                    candxP.x += skip;
               }

               if(candxM.x < skip){
                    candxM.x = 0;
               } else{
                    candxM.x -= skip;
               }

               if(candyM.y > (mat_hair_final.rows - (skip+1))){
                    candyM.y = mat_hair_final.rows - 1;
               } else{
                    candyM.y += skip;
               }

               if(candyP.y < skip){
                    candyP.y = 0;
               } else{
                    candyP.y -= skip;
               }
                
               //identifica candidato final dependendo da maior linha ser vertical (lin[2]) ou horizontal (lin[0])
               //Os pontos devem ser perpendiculares

               if(linhas.lin[0] > linhas.lin[2]){
                   candFinal1 = candyP;
                   candFinal2 = candyM;
               } else{
                   candFinal1 = candxP;
                   candFinal2 = candxM; 
               }

               //Calcula de fato a interpolação

               bgrBilinear[0] =
                    
                    channels[0].at<uchar>(candFinal2)*sqrt((pow(candFinal1.x-x,2)+pow(candFinal1.y-y,2))/(pow(candFinal2.x - candFinal1.x,2)+pow(candFinal2.y - candFinal1.y,2))) +
                    channels[0].at<uchar>(candFinal1)*sqrt((pow(candFinal2.x-x,2)+pow(candFinal2.y-y,2))/(pow(candFinal2.x - candFinal1.x,2)+pow(candFinal2.y - candFinal1.y,2)));

               bgrBilinear[1] =
                    (int)channels[1].at<uchar>(candFinal2)*sqrt((pow(candFinal1.x-x,2)+pow(candFinal1.y-y,2))/(pow(candFinal2.x - candFinal1.x,2)+pow(candFinal2.y - candFinal1.y,2))) +
                    (int)channels[1].at<uchar>(candFinal1)*sqrt((pow(candFinal2.x-x,2)+pow(candFinal2.y-y,2))/(pow(candFinal2.x - candFinal1.x,2)+pow(candFinal2.y - candFinal1.y,2)));

               bgrBilinear[2] =
                    (int)channels[2].at<uchar>(candFinal2)*sqrt((pow(candFinal1.x-x,2)+pow(candFinal1.y-y,2))/(pow(candFinal2.x - candFinal1.x,2)+pow(candFinal2.y - candFinal1.y,2))) +
                    (int)channels[2].at<uchar>(candFinal1)*sqrt((pow(candFinal2.x-x,2)+pow(candFinal2.y-y,2))/(pow(candFinal2.x - candFinal1.x,2)+pow(candFinal2.y - candFinal1.y,2)));
            
             // Coloca em cada canal
             channels_bilinear[0].at<uchar>(Point(x,y)) = bgrBilinear[0];
             channels_bilinear[1].at<uchar>(Point(x,y)) = bgrBilinear[1];
             channels_bilinear[2].at<uchar>(Point(x,y)) = bgrBilinear[2];
               
            }

        }
    }
    
    //Junta os canais em um Mat de 3 canais
    merge(channels_bilinear,3,mat_bilinear);

    // A matriz de cabelos que será dilatada é invertida para que a dilatação seja feita corretamente. 
    mat_hair_final_grossa = 255 - mat_hair_final;

    // Dilata a matriz de pelos para que estes fiquem mais grossos, utilizo aqui um kernel de 4x4 com 1's
    dilate(mat_hair_final_grossa,mat_hair_final_grossa, kernelD);
    mat_hair_final_grossa = 255 - mat_hair_final_grossa;

    channels_bilinear[0].copyTo(channels_mediana[0]);
    channels_bilinear[1].copyTo(channels_mediana[1]);
    channels_bilinear[2].copyTo(channels_mediana[2]);
    
    // Aplica a mediana nos canais interpolados
    medianFiltering(&channels_bilinear[0],&channels_mediana[0],&mat_hair_final_grossa,5);
    medianFiltering(&channels_bilinear[1],&channels_mediana[1],&mat_hair_final_grossa,5);
    medianFiltering(&channels_bilinear[2],&channels_mediana[2],&mat_hair_final_grossa,5);

   
    // Junta os 3 canais na imagem final
    merge(channels_mediana,3,matFinal);
    
    //Descomente e adicione mais linhas para ver resultados na hora
    // imshow("Imagem Original", mat_colored);
    // imshow("Sem cabelo", mat_out);
    // imshow("Cabelos", mat_hair_final);
    // imshow("Cabelos dilatados", mat_hair_final_grossa);
    // waitKey(0);
    // destroyAllWindows();
    
    //salva nos arquivos
    imwrite("output/imgFinal.png",matFinal);
    imwrite("output/bilinear.png",mat_bilinear);
    imwrite("output/GeneralizedBluei.png", generalized_grayscale[0]);
    imwrite("output/GeneralizedGreeni.png", generalized_grayscale[1]);
    imwrite("output/GeneralizedRedi.png", generalized_grayscale[2]);
    imwrite("output/ChannelBluei.png", channels[0]);
    imwrite("output/ChannelGreeni.png", channels[1]);
    imwrite("output/ChannelRedi.png", channels[2]);
    imwrite("output/ClosedBluei.png", mat_closed[0]);
    imwrite("output/ClosedGreeni.png", mat_closed[1]);
    imwrite("output/ClosedRedi.png", mat_closed[2]);
    imwrite("output/HairBluei.png", mat_hair[0]);
    imwrite("output/HairGreeni.png", mat_hair[1]);
    imwrite("output/HairRedi.png", mat_hair[2]);
    imwrite("output/pelo_fino_sem_ruido.png", mat_hair_final);
    imwrite("output/pelo_grosso.png", mat_hair_final_grossa);
    return 0;
}
