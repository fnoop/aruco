#include "aruco.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>

using namespace cv;
using namespace aruco;
using namespace std;

struct Cube{
    int id;
    cv::Point3f pos;
    float size;
    bool isInCorrectPosition;

    vector<cv::Point3f> getCorners()const{
        vector<cv::Point3f> corners3d={
            cv::Point3f(0,0,0), //0
            cv::Point3f(0, size,0),//1
            cv::Point3f(0,size, size),//2
            cv::Point3f(0,0, size),//3

            cv::Point3f( size,0,0), //4
            cv::Point3f( size, size,0),//5
            cv::Point3f( size, size, size),//6
            cv::Point3f( size,0,size),//7
        };
        for(auto &corner:corners3d)
            corner+=pos;
        return         corners3d;
    }

    float dist(const Cube &c)const{
        auto c1=getCorners();
        auto c2=c.getCorners();
        float minD=std::numeric_limits<float>::max();
        for(int i=0;i<8;i++)
            for(int j=0;j<8;j++){
                float d=cv::norm(c1[i]-c2[j]);
                if (d<minD)minD=d;
            }
        return minD;
    }

    bool isInto(cv::Point3f p)const{

        p-=pos;
        if(0<=p.x &&  p.x<=size &&0<=p.y &&  p.y<=size &&0<=p.z &&  p.z<=size ) return true;
        return false;
    }


};

std::vector<Cube> Cubes;


void drawCube(const Cube&c,cv::Mat &im,const CameraParameters &cmp,const cv::Mat &rvec,const cv::Mat &tvec,cv::Scalar color,int thickline=1){

    //get corners of cube and project them

    vector<cv::Point3f> corners3d= c.getCorners();

    vector<cv::Point2f>     corners2d;
    cv::projectPoints(corners3d,rvec,tvec,cmp.CameraMatrix,cmp.Distorsion,corners2d);
     //now, draw lines
    cv::line(  im,corners2d[0],corners2d[1],color,thickline);
    cv::line(  im,corners2d[1],corners2d[2],color,thickline);
    cv::line(  im,corners2d[2],corners2d[3],color,thickline);
    cv::line(  im,corners2d[3],corners2d[0],color,thickline);

    cv::line(  im,corners2d[4],corners2d[5],color,thickline);
    cv::line(  im,corners2d[5],corners2d[6],color,thickline);
    cv::line(  im,corners2d[6],corners2d[7],color,thickline);
    cv::line(  im,corners2d[7],corners2d[4],color,thickline);

    cv::line(  im,corners2d[0],corners2d[4],color,thickline);
    cv::line(  im,corners2d[1],corners2d[5],color,thickline);
    cv::line(  im,corners2d[2],corners2d[6],color,thickline);
    cv::line(  im,corners2d[3],corners2d[7],color,thickline);

}
void putText(cv::Mat &im,string text,cv::Point p,float size){
    float fact=float(im.cols)/float(640);
    if (fact<1) fact=1;

    cv::putText(im,text,p,FONT_HERSHEY_SIMPLEX, size,cv::Scalar(0,0,0),3*fact);
    cv::putText(im,text,p,FONT_HERSHEY_SIMPLEX, size,cv::Scalar(125,255,255),1*fact);

}

float distanceToDesired(Cube &c,const MarkerMap &mmap){
    return cv::norm(mmap.getMarker3DInfo(c.id)[3] -c.pos);

}


bool isThereColision( cv::Point3f newPos,const std::vector<Cube> &Cubes){

    if (Cubes.size()==1)return false;
    auto cubeNewPosition=Cubes.back();
    cubeNewPosition.pos=newPos;
    //distance to any
    for(size_t i=0;i<Cubes.size()-1;i++){
        for(auto p:Cubes[i].getCorners())
            if ( cubeNewPosition.isInto(p)) return true;

    }
    return false;
}

void addRandomCubeInEmptyLocation( std::vector<Cube>&cubes ,const MarkerMap &mmap,float markersize)
{

    //create list of places
    std::set<int> ids;
    for(auto &m:mmap) ids.insert(m.id);
    ///remove used ones
    for(auto &c:cubes )
        ids.erase(c.id);
    //now select one randomly

    int r=rand()%ids.size();
    cout<<"r="<<r<<endl;
    int selectedId;
    for(auto it:ids){
        r--;
        if (r==0) {selectedId=it;break;}
    }
    cout<<"selectedId="<<selectedId<<endl;
 //now, create
    float sig1=rand()%2==0?1:-1;
    float sig2=rand()%2==0?1:-1;

    Cubes.push_back(Cube{selectedId,cv::Point3f(sig1*3.5*markersize,sig2*markersize,0),markersize,false});

}

int main(int argc,char **argv)
{
    if (argc!=4){
        cerr<<"Usage: map camera msize"<<endl;
        return -1;
    }

     int nTries=0;

    VideoCapture cap;
    cap.open(1);
    Mat frame;
    aruco::MarkerMap mmap;
    mmap.readFromFile(argv[1]);
    aruco::CameraParameters camp;
    camp.readFromXMLFile(argv[2]);

    MarkerDetector MDetector;
    MDetector.setDictionary(mmap.getDictionary());
    float markersize=stof(argv[3]);



    srand(time(NULL));
   // Cubes.push_back(Cube{mmap[ rand()%mmap.size()].id,cv::Point3f(-3.5*markersize,-3.5*markersize,0),markersize,true});
addRandomCubeInEmptyLocation(Cubes,mmap,markersize);


    aruco::MarkerMapPoseTracker poseTracker;
    mmap=mmap.convertToMeters(std::stof(argv[3]));
    poseTracker.setParams(camp,mmap);


    ///compte max distance between any cube
    float maxD=0;
    for(size_t i=0;i<mmap.size();i++)
        for(size_t j=i+1;j<mmap.size();j++){
            float d=cv::norm( mmap.getMarker3DInfo(mmap[i].id)[0]-mmap.getMarker3DInfo(mmap[j].id)[0]);
                    if (d>maxD)maxD=d;
        }

    vector<Marker> Markers;
    char k=0;
    while( k!=27)
    {
        cap  >> frame;
        Markers = MDetector.detect(frame);

        if (poseTracker.estimatePose(Markers)){
            //find distance to position
            float dist=distanceToDesired(Cubes.back(),mmap);
            float dColor= 255*dist/maxD;
            cv::Scalar color(dColor,0,255-dColor);
            putText(frame,"Encuentra la posicion correcta. Pasos: "+to_string(nTries),cv::Point(20,20),0.5);

            if(dist<1e-3){
                Cubes.back().isInCorrectPosition=true;
                addRandomCubeInEmptyLocation(Cubes,mmap,markersize);
                color=cv::Scalar(55,255,55);
            }

            drawCube(Cubes.back(),frame,camp,poseTracker.getRvec(),poseTracker.getTvec(),color,2);
            //draw the others
            for(int c=0;c<Cubes.size()-1;c++)
                drawCube(Cubes[c] ,frame,camp,poseTracker.getRvec(),poseTracker.getTvec(),cv::Scalar(55,255,55),2);

        }

        namedWindow("in", 1);
        imshow("in", frame);
        k=waitKey(30);

        if (k!=-1){
            cv::Point3f newPos=Cubes.back().pos;
            switch (k) {
            case 84:
                newPos+=cv::Point3f(markersize*0.1,0,0);
                break;
            case 82:
                newPos+=cv::Point3f(-markersize*0.1,0,0);
                break;
            case 83:
                newPos+=cv::Point3f(0,markersize*0.1,0);
                break;
            case 81:
                newPos+=cv::Point3f(0,-markersize*0.1,0);
                break;
            default:
                break;
            };
            if (!isThereColision(newPos,Cubes) ){
                Cubes.back().pos=newPos;
                nTries++;
            }
        }
    }
}


