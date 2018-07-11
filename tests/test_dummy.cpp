#include <iostream>
#include "aruco.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace std;

void splitView (cv::Mat &io){
    int w2=io.cols/2;
    cv::Mat row(1,io.cols,CV_8UC4);
    cv::Vec4b *rowPtr=row.ptr<cv::Vec4b>(0);
    for(int y=0;y<io.rows;y++){
        cv::Vec4b *ptr=io.ptr<cv::Vec4b>(y);
        memcpy(rowPtr,ptr,io.cols*4);
        memset(ptr,0,io.cols*4);
        for(int x=0;x<w2;x++){
           ptr[x+w2]=ptr[x]=rowPtr[x*2];
        }

    }
}
int main(int argc,char **argv)
{
   try
   {
       if (argc!=2) throw std::runtime_error("Usage: incamera");


       cv::Mat im=cv::imread(argv[1]);
       cv::Mat im4;
       cv::cvtColor(im,im4,CV_BGR2BGRA);
       splitView(im4);
       cv::imshow("im",im4);
       cv::waitKey(0);

   } catch (std::exception &ex)
   {
       std::cout<<"Exception :"<<ex.what()<<std::endl;
   }
}
