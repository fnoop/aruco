#include <vector>
#include <iostream>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include "markerdetector.h"
#include <opencv2/flann.hpp>
#include "picoflann.h"
using namespace std;

int main(int argc,char **argv)
{


      struct PicoFlann_Array3f_Adapter{
          inline   float operator( )(const cv::Point3f &elem, int dim)const{ return ((float*)&elem) [dim]; }
      };
      struct PicoFlann_Array3f_Container{

          const cv::Point3f* _mref;
          size_t _size;
          PicoFlann_Array3f_Container(const cv::Mat  &m){
              assert(m.cols==3);
              assert(m.type()==CV_32F);
              _mref=m.ptr<cv::Point3f>();
              _size=m.rows;
          }
          inline size_t size()const{return _size;}
          inline const cv::Point3f &at(int idx)const{ return _mref [idx];}
      };
      std::default_random_engine generator;
      std::uniform_real_distribution<double> distribution(-1000.0,1000.0);
      int nPoints=1000;
      int nTimes=1000;
      int nDimensions=3;
      cv::Mat points(nPoints,nDimensions,CV_32F);
      float *array=new float[nPoints*nDimensions];
      for(size_t i=0;i<nTimes*nDimensions;i++)
          points.ptr<float>(0)[i]= distribution(generator);

      ///------------------------------------------------------------
      picoflann::KdTreeIndex<3,PicoFlann_Array3f_Adapter> kdtree;// 3 is the number of dimensions, L2 is the type of distance
      PicoFlann_Array3f_Container p3container(points);
      double t1=cv::getTickCount();
      kdtree.build( p3container);
      double t2=cv::getTickCount();
      cout<<"pico flann creation="<< 1000.*double(t2-t1) / cv::getTickFrequency()<<endl;;

      t1=cv::getTickCount();
      cv::flann::Index flann_index(points,cv::flann::KDTreeIndexParams(1), cvflann::FLANN_DIST_EUCLIDEAN );
      t2=cv::getTickCount();
      cout<<"OpenCv creation="<< 1000.*double(t2-t1) / cv::getTickFrequency()<<endl;;



      ////////////////////////////////////////
      ///////////// KNN
      ////////////////////////////////////////
      int knn=1000;
      ////////    PicoFlann
      std::vector<std::pair<uint32_t,double> > res;
      t1=cv::getTickCount();
      for(int i=0;i<nTimes;i++)
         res=kdtree.searchKnn(p3container,cv::Point3f(0,0,0),knn);
      t2=cv::getTickCount();
      cout<<"pico flann knn(10)="<< 1000.*double(t2-t1) / cv::getTickFrequency()<<endl;


      //print result
      cout<<"picoflann Nearest neighbors:";
       for(auto i:res)cout<<i.first<<" ";cout<<endl;


      ////////    OpenCv

      cv::Mat query(1,3,CV_32F);
      cv::Mat dists,indices;
      query.ptr<cv::Point3f>(0)[0]=cv::Point3f(0,0,0);
      t1=cv::getTickCount();
      for(int i=0;i<nTimes;i++)
          flann_index.knnSearch(query,indices,dists,knn,cv::flann::SearchParams(-1));
      t2=cv::getTickCount();
      cout<<"OpenCv knn(10)="<< 1000.*double(t2-t1) / cv::getTickFrequency()<<endl;;
      cout<<"OpenCv Nearest neighbors:";
      cout<<indices<<endl;

      ////////////////////////////////////////
      ///////////// Radius search
      ////////////////////////////////////////
      ////////    PicoFlann
      t1=cv::getTickCount();
      for(int i=0;i<nTimes;i++)
          kdtree.radiusSearch(p3container,res,cv::Point3f(0,0,0),300);
      t2=cv::getTickCount();
      cout<<"pico flann radius(300)="<< 1000.*double(t2-t1) / cv::getTickFrequency()<<endl;
      //print
    //  cout<<"pico RadiusSearch result:";
      //for(auto i:res)cout<<i.first<<" ";cout<<endl;


      ////////    OpenCv

      int nrs;
      t1=cv::getTickCount();
      for(int i=0;i<nTimes;i++){
          nrs=flann_index.radiusSearch(query,indices,dists,300*300,points.rows,cv::flann::SearchParams(-1));
      }
      t2=cv::getTickCount();
      cout<<"OpenCv  radius(300)="<< 1000.*double(t2-t1) / cv::getTickFrequency()<<endl;
    //  cout<<indices.colRange(0,nrs)<<endl;


}


