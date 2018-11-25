#include "HairRemoval.hpp"


//converte para preto e branco
bool HairRemoval::toGrayscale(Mat* in, Mat* out){
    
    //argumentos devem conter dados
    if( !in->data || !out->data ) {
        printf("Entrada invalida!\n");
        return false;
    }
    
    //percorre pixel a pixel e preenche a imagem em preto e branco com a soma B*bgr[0] + G*bgr[1] + R*bgr[2]
    for (int i=0; i< in->cols ; i++){
        for (int j=0 ; j< in->rows ; j++){ 

            Vec3b rgbColor = in->at<Vec3b>(Point(i,j));
            Scalar grayColor = out->at<uchar>(Point(i,j));
            grayColor.val[0] = bgr[0]*rgbColor.val[0]+ bgr[1]*rgbColor.val[1]+ bgr[2]*rgbColor.val[2];
     
            out->at<uchar>(Point(i,j)) = grayColor.val[0];
        }
    }

    return true; 
}


//calcula distancia em linha reta do inicio até o fim
double HairRemoval::euclidianDistance(list<Point2d>& l){
    Point2d start, end;
    start = Point(l.front().x, l.front().y);
    end = Point(l.back().x,l.back().y);

    return(sqrt(pow(start.x - end.x,2) + pow(start.y-end.y,2)));
}
