#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include "aruco.h"
#include "ippe.h"
using namespace std;
int main(int argc,char **argv){
    try{
         if (argc!=4){
            cerr<<"In: video camera_params dict "<<endl;
            return -1;
        }

        aruco::CameraParameters camp;
        cv::VideoCapture vcap(argv[1]);
        camp.readFromXMLFile(argv[2]);
        cv::Mat img;
        aruco::MarkerDetector md;
        md.setDictionary(argv[3]);
//        md.getParams().useLowresImageForRectDetection=true;
//        md.getParams().enclosedMarker=true;
//         md.getParams().maxThreads=-1;
//        md.getParams().minSize_pix=100;
//        md.getParams()._markerWarpPixSize=5;
//        if (string(argv[3])=="ARUCO" )
//        md_params._cornerMethod=aruco::MarkerDetector::LINES;
//        else
//            md_params._cornerMethod=aruco::MarkerDetector::SUBPIX;
         pair<double,uint32_t> avrg_ratio(0,0);
        bool finish=false;
        int nframes=0;
        while(vcap.grab() && !finish){
            vcap.retrieve(img);
            auto markers=md.detect(img);
            for(auto m:markers){
                m.draw(img,cv::Scalar(255,0,0));
                auto res= solvePnP_(0.125f,m,camp.CameraMatrix,camp.Distorsion);
                avrg_ratio.first+= res[0].second;
                avrg_ratio.second++;
            }
            cout<<"avrg ratio="<<avrg_ratio.first/double(avrg_ratio.second)<<endl;
            cv::imshow("iamge",img);
            char k=cv::waitKey(5);
            if (k==27)finish=true;
            if(nframes++>1000)finish=true;
        }
    }catch(std::exception &ex){
        cout<<ex.what()<<endl;
    }
}

