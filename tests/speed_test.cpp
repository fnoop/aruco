/*****************************************************************************************
Copyright 2011 Rafael Muñoz Salinas. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Rafael Muñoz Salinas ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Rafael Muñoz Salinas OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Rafael Muñoz Salinas.
********************************************************************************************/
#include "aruco.h"
#include "cvdrawingutils.h"
#include <fstream>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "timers.h"
#include <sstream>
#include <string>
using namespace std;
using namespace cv;
using namespace aruco;

MarkerDetector MDetector;
VideoCapture TheVideoCapturer;
vector<Marker> TheMarkers;
Mat TheInputImage,TheInputImageGrey, TheInputImageCopy;
CameraParameters TheCameraParameters;
void cvTackBarEvents(int pos, void*);
string dictionaryString;
 int  iEnclosedMarkers=0,iCorrectionRate=0,iShowAllCandidates=0,iMinMarkerSize=20,iNumThreads=1;
int iShowedTestImage=0 ,iCornerUpsample=0;
int waitTime = 0,iAutoSize=0;
bool showMennu=false,bPrintHelp=false,isVideo=false;
class CmdLineParser{int argc;char** argv;public:CmdLineParser(int _argc, char** _argv): argc(_argc), argv(_argv){}   bool operator[](string param)    {int idx = -1;  for (int i = 0; i < argc && idx == -1; i++)if (string(argv[i]) == param)idx = i;return (idx != -1);}    string operator()(string param, string defvalue = "-1")    {int idx = -1;for (int i = 0; i < argc && idx == -1; i++)if (string(argv[i]) == param)idx = i;if (idx == -1)return defvalue;else return (argv[idx + 1]);}};
struct   TimerAvrg{std::vector<double> times;size_t curr=0,n; std::chrono::high_resolution_clock::time_point begin,end;   TimerAvrg(int _n=30){n=_n;times.reserve(n);   }inline void start(){begin= std::chrono::high_resolution_clock::now();    }inline void stop(){end= std::chrono::high_resolution_clock::now();double duration=double(std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count())*1e-6;if ( times.size()<n) times.push_back(duration);else{ times[curr]=duration; curr++;if (curr>=times.size()) curr=0;}}double getAvrg(){double sum=0;for(auto t:times) sum+=t;return sum/double(times.size());}};
pair<double, double> AvrgTime(0, 0);  // determines the average time required for detection
TimerAvrg Fps;
cv::Mat resize(const cv::Mat& in, int width)
{
    if (in.size().width <= width)
        return in;
    float yf = float(width) / float(in.size().width);
    cv::Mat im2;
    cv::resize(in, im2, cv::Size(width, static_cast<int>(in.size().height * yf)));
    return im2;
}
/************************************
 *
 *
 *
 *
 ************************************/
void setParamsFromGlobalVariables(aruco::MarkerDetector &md){
    if (iEnclosedMarkers){
        md.getParams().enclosedMarker=true;
     }
    else{
        md.getParams().enclosedMarker=false;
     }
    md.getParams().minSize=float(iMinMarkerSize)/1000.;
    md.getParams().maxThreads=iNumThreads;
    md.setDictionary(dictionaryString,float(iCorrectionRate)/10. );  // sets the dictionary to be employed (ARUCO,APRILTAGS,ARTOOLKIT,etc)
    md.getParams().setAutoSizeSpeedUp(iAutoSize);

}

void createMenu(){
   cv::createTrackbar("AutoSize", "in", &iAutoSize, 1, cvTackBarEvents);
   cv::createTrackbar("MinSize", "in", &iMinMarkerSize, 1000, cvTackBarEvents);
   cv::createTrackbar("nThreads", "in", &iNumThreads, 6, cvTackBarEvents);
   cv::createTrackbar("Enclosed", "in", &iEnclosedMarkers, 1, cvTackBarEvents);
   cv::createTrackbar("ErrorRate", "in", &iCorrectionRate, 10, cvTackBarEvents);
   cv::createTrackbar("ShowAll", "in", &iShowAllCandidates, 1, cvTackBarEvents);
   cv::createTrackbar("ThresImg", "in", &iShowedTestImage, 100, cvTackBarEvents);
}

void putText(cv::Mat &im,string text,cv::Point p,float size){
    float fact=float(im.cols)/float(640);
    if (fact<1) fact=1;

    cv::putText(im,text,p,FONT_HERSHEY_SIMPLEX, size,cv::Scalar(0,0,0),3*fact);
    cv::putText(im,text,p,FONT_HERSHEY_SIMPLEX, size,cv::Scalar(125,255,255),1*fact);

}
void printHelp(cv::Mat &im)
{
    (void)im;
    cv::putText(im,"'m': show/hide menu",cv::Point(10,40),FONT_HERSHEY_SIMPLEX, 0.5f,cv::Scalar(125,255,255),1);
    cv::putText(im,"'w': write image to file",cv::Point(10,60),FONT_HERSHEY_SIMPLEX, 0.5f,cv::Scalar(125,255,255),1);
    cv::putText(im,"'t': do a speed test ",cv::Point(10,80),FONT_HERSHEY_SIMPLEX, 0.5f,cv::Scalar(125,255,255),1);

}

void printInfo(cv::Mat &im){
    float fs=float(im.cols)/float(800);
    putText(im,"fps="+to_string(1./Fps.getAvrg()),cv::Point(10,fs*20),fs*0.5f);
    putText(im,"'h': show/hide help",cv::Point(10,fs*40),fs*0.5f);
    if(bPrintHelp) printHelp(im);
    else
    {
        putText(im,"'h': show/hide help",cv::Point(10,fs*40),fs*0.5f);
        putText(im,"'m': show/hide menu",cv::Point(10,fs*60),fs*0.5f);
    }
}

/************************************
 *
 *
 *
 *
 ************************************/
int main(int argc, char** argv)
{
    try
    {
        CmdLineParser cml(argc, argv);
        if (argc < 2 || cml["-h"])
        {
            cerr << "Invalid number of arguments" << endl;
            cerr << "Usage: (in.avi|live[:camera_index(e.g 0 or 1)]) [-c camera_params.yml] [-s  marker_size_in_meters] [-d "
                    "dictionary:ARUCO by default] [-h]"
                 << endl;
            cerr << "\tDictionaries: ";
            for (auto dict : aruco::Dictionary::getDicTypes())
                cerr << dict << " ";
            cerr << endl;
            cerr << "\t Instead of these, you can directly indicate the path to a file with your own generated "
                    "dictionary"
                 << endl;
            return false;
        }

        ///////////  PARSE ARGUMENTS
        cv::VideoCapture TheVideoCapturer(argv[1]);
        if (!TheVideoCapturer.isOpened())
        {
            cerr << "Could not open input" << endl;
            return -1;
        }

        std::ofstream times(argv[2]);
        if (!times)throw std::runtime_error("Could not open file times");
        times << "frame,nmarkers,area,minSize,tmean,tmin" << std::endl;

        int width, height;
        bool res = false;
        if(cml["-r"]) {
            if (sscanf(cml("-r").c_str(), "%d:%d", &width, &height) != 2)
            {
                cerr << "Incorrect X:Y specification" << endl;
                return -1;
            }
            res = true;
        }

        // read camera parameters if passed
        if (cml["-c"])
            TheCameraParameters.readFromXMLFile(cml("-c"));

        float TheMarkerSize = std::stof(cml("-s", "-1"));
        // aruco::Dictionary::DICT_TYPES  TheDictionary= Dictionary::getTypeFromString( cml("-d","ARUCO") );


        ///// CONFIGURE DATA
        // read first image to get the dimensions
        if (TheCameraParameters.isValid())
            TheCameraParameters.resize(TheInputImage.size());
        dictionaryString=cml("-d", "ARUCO");
        MDetector.setDictionary(dictionaryString,float(iCorrectionRate)/10. );  // sets the dictionary to be employed (ARUCO,APRILTAGS,ARTOOLKIT,etc)
        MDetector.getParams().setThresholdMethod(aruco::THRES_AUTO_FIXED);
        MDetector.getParams().setAutoSizeSpeedUp(true,0.2);
        iAutoSize=MDetector.getParams().getAutoSizeSpeedUp();
        //cv::namedWindow("in",cv::WINDOW_NORMAL);
        //cv::resizeWindow("in",640,480);
        //cv::createTrackbar("iCornerUpsample", "in", &iCornerUpsample, 1, cvTackBarEvents);
        setParamsFromGlobalVariables(MDetector);

        {
        float w=std::min(int(1920),int(TheInputImage.cols));
        float f=w/float(TheInputImage.cols);
        resizeWindow("in",w,float(TheInputImage.rows)*f);

        }

        int iterations=50;

        //MDetector.getParams().minSize=-1;
        while (TheVideoCapturer.grab())
        {
            TheVideoCapturer.retrieve(TheInputImage);
            if (res) resize(TheInputImage, TheInputImage, cv::Size(width, height), 0, 0, cv::INTER_CUBIC);

            cv::Mat greyImage;
            cv::cvtColor(TheInputImage, greyImage, CV_BGR2GRAY);

            double tmin = 99999;

            TheMarkers = MDetector.detect(greyImage, TheCameraParameters, TheMarkerSize);
            cout << "Aruco3 - Frame: " << TheVideoCapturer.get(1) << ", Dectected:"<< TheMarkers.size() << std::endl;

            if(TheMarkers.size()>0)
            {
                //Calculo de tiempos
                AvrgTime.first =0;
                AvrgTime.second=0;

                for(int i=0; i<iterations; i++) {
                    // copy image
                    double tick = (double)getTickCount();  // for checking the speed
                    // Detection of markers in the image passed
                    TheMarkers = MDetector.detect(greyImage, TheCameraParameters, TheMarkerSize, TheVideoCapturer.get(1));
                    // chekc the speed by calculating the mean speed of all iterations
                    double time = ((double)getTickCount() - tick) / getTickFrequency();
                    AvrgTime.first += time;
                    AvrgTime.second++;

                    if(tmin > time) tmin = time;
                }

                //Calculo del area de los marcadores
                float area = 0;
                for(auto m:TheMarkers){
                    float dig1 = sqrt(pow(m[0].x - m[2].x, 2) + pow(m[0].y - m[2].y, 2));
                    float dig2 = sqrt(pow(m[1].x - m[3].x, 2) + pow(m[1].y - m[3].y, 2));
                    area += dig1*dig2/2;
                }

                times << TheVideoCapturer.get(1) << ","  << TheMarkers.size() << "," << area << "," << MDetector.getParams().minSize
                      << "," << 1000 * AvrgTime.first / AvrgTime.second << "," << 1000 * tmin << std::endl;
            }

        }
    }
    catch (std::exception& ex)

    {
        cout << "Exception :" << ex.what() << endl;
    }
}


void cvTackBarEvents(int pos, void*)
{
    (void)(pos);


    setParamsFromGlobalVariables(MDetector);

    // recompute
        Fps.start();
        MDetector.detect(TheInputImage, TheMarkers, TheCameraParameters);
        Fps.stop();
    // chekc the speed by calculating the mean speed of all iterations
    TheInputImage.copyTo(TheInputImageCopy);
    if (iShowAllCandidates){
        auto candidates=MDetector.getCandidates();
        for(auto cand:candidates)
            Marker(cand,-1).draw(TheInputImageCopy, Scalar(255, 0, 255),1);
    }

    for (unsigned int i = 0; i < TheMarkers.size(); i++){
        cout << TheMarkers[i] << endl;
        TheMarkers[i].draw(TheInputImageCopy, Scalar(0, 0, 255),2);
    }

    // draw a 3d cube in each marker if there is 3d info
    if (TheCameraParameters.isValid())
        for (unsigned int i = 0; i < TheMarkers.size(); i++)
            CvDrawingUtils::draw3dCube(TheInputImageCopy, TheMarkers[i], TheCameraParameters);
    //cv::putText(TheInputImageCopy,"fps="+to_string(1./Fps.getAvrg() ),cv::Point(10,20),FONT_HERSHEY_SIMPLEX, 0.5f,cv::Scalar(125,255,255),2,CV_AA);

    //cv::imshow("in",  TheInputImageCopy );
    //cv::imshow("thres", resize(MDetector.getThresholdedImage(iShowedTestImage), 1024));
}
