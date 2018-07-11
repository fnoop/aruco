#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>
#include <map>
using namespace std;




cv::Mat  getMatrix(double tx,double ty ,double tz,double qx,double qy, double qz,double qw){

    double qw2 = qw*qw;
    double qx2 = qx*qx;
    double qy2 = qy*qy;
    double qz2 = qz*qz;


    cv::Mat m=cv::Mat::eye(4,4,CV_32F);

    m.at<float>(0,0)=1 - 2*qy2 - 2*qz2;
    m.at<float>(0,1)=2*qx*qy - 2*qz*qw;
    m.at<float>(0,2)=2*qx*qz + 2*qy*qw;
    m.at<float>(0,3)=tx;

    m.at<float>(1,0)=2*qx*qy + 2*qz*qw;
    m.at<float>(1,1)=1 - 2*qx2 - 2*qz2;
    m.at<float>(1,2)=2*qy*qz - 2*qx*qw;
    m.at<float>(1,3)=ty;

    m.at<float>(2,0)=2*qx*qz - 2*qy*qw	;
    m.at<float>(2,1)=2*qy*qz + 2*qx*qw	;
    m.at<float>(2,2)=1 - 2*qx2 - 2*qy2;
    m.at<float>(2,3)=tz;
    return m;
}


std::map<uint32_t,cv::Mat> loadFile(std::string fp,bool invert=false){
    std::map<uint32_t,cv::Mat> fmap;
    ifstream file(fp);
    float stamp;
    float tx,ty,tz,qx,qy,qz,qw;
    while(!file.eof()){
         if (file>>stamp>>tx>>ty>>tz>>qx>>qy>>qz>>qw){
             auto m=getMatrix(tx,ty,tz,qx,qy,qz,qw);
             if (invert)m=m.inv();
            fmap.insert(make_pair (stamp,  m));
        }
     }
    return fmap;
}

int main(int argc,char **argv){
    try{
        if (argc!=3){cerr<<"In.file out.pcd"<<endl;return -1;}

        auto posemap=loadFile(argv[1]);

        vector<cv::Vec4f> points2write;

        int counter=0;
        for(auto p:posemap){
            cv::Vec4f point;
            for(int i=0;i<3;i++)
                point[i]=p.second.at<float>(i,3);
            cv::Vec4b color;
            if (counter==0){
                color=cv::Vec4b (0,255,0,0);
            }
            else{
                uchar c= 255* float(counter)/float(posemap.size());
                color=cv::Vec4b (255-c,0,c,c);
            }
            counter++;
            memcpy(&point[3],&color,sizeof(float));
            points2write.push_back(point);

        }

        std::ofstream filePCD ( argv[2], std::ios::binary );

        filePCD<<"# .PCD v.7 - Point Cloud Data file format\nVERSION .7\nFIELDS x y z rgb\nSIZE 4 4 4 4\nTYPE F F F F\nCOUNT 1 1 1 1\nVIEWPOINT 0 0 0 1 0 0 0\nWIDTH "<<points2write.size()<<"\nHEIGHT 1\nPOINTS "<<points2write.size()<<"\nDATA binary\n";


        filePCD.write((char*)&points2write[0],points2write.size()*sizeof(points2write[0]));

    }catch(std::exception &ex){
        cerr<<ex.what()<<endl;
    }
}
