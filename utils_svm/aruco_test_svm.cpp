/**
Copyright 2017 Rafael Muñoz Salinas. All rights reserved.

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
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include "aruco.h"
#include "cvdrawingutils.h"
#include <opencv2/highgui/highgui.hpp>
#include "markerlabeler.h"
using namespace cv;
using namespace aruco;
using namespace std;

std::string TheInputVideo;
std::string TheIntrinsicFile;
std::string TheSVMFile;
float TheMarkerSize = -1;
int dictionarySize = -1;
int ThePyrDownLevel;
MarkerDetector MDetector;
VideoCapture TheVideoCapturer;
std::vector< Marker > TheMarkers;
Mat TheInputImage, TheInputImageCopy;
CameraParameters TheCameraParameters;
void cvTackBarEvents(int pos, void *);
bool readCameraParameters(string TheIntrinsicFile, CameraParameters &CP, Size size);

pair< double, double > AvrgTime(0, 0); // determines the average time required for detection

int waitTime = 0;

/************************************
 *
 *
 *
 *
 ************************************/
bool readArguments(int argc, char **argv) {
    if (argc < 3) {
        cerr << "Invalid number of arguments" << endl;
        cerr << "Usage: (in.avi|live[:idx_cam=0]) svm_model.yml [intrinsics.yml] [size]" << endl;
        return false;
    }
    TheInputVideo = argv[1];
     TheSVMFile  = argv[2];
    if (argc >= 4)
        TheIntrinsicFile = argv[3];
    if (argc >= 5)
        TheMarkerSize = atof(argv[4]);

    if (argc == 4)
        cerr << "NOTE: You need makersize to see 3d info!!!!" << endl;
    return true;
}

int findParam(std::string param, int argc, char *argv[]) {
    for (int i = 0; i < argc; i++)
        if (string(argv[i]) == param)
            return i;

    return -1;
}
/************************************
 *
 *
 *
 *
 ************************************/
int main(int argc, char **argv) {
    try {
        if (readArguments(argc, argv) == false) {
            return 0;
        }
        // parse arguments

        // read from camera or from  file
        if (TheInputVideo.find("live") != string::npos) {
            int vIdx = 0;
            // check if the :idx is here
            char cad[100];
            if (TheInputVideo.find(":") != string::npos) {
                std::replace(TheInputVideo.begin(), TheInputVideo.end(), ':', ' ');
                sscanf(TheInputVideo.c_str(), "%s %d", cad, &vIdx);
            }
            cout << "Opening camera index " << vIdx << endl;
            TheVideoCapturer.open(vIdx);
            waitTime = 10;
        } else
            TheVideoCapturer.open(TheInputVideo);
        // check video is open
        if (!TheVideoCapturer.isOpened()) {
            cerr << "Could not open video" << endl;
            return -1;
        }
        bool isVideoFile = false;
        if (TheInputVideo.find(".avi") != std::string::npos || TheInputVideo.find("live") != string::npos)
            isVideoFile = true;
        // read first image to get the dimensions
        TheVideoCapturer >> TheInputImage;

        // read camera parameters if passed
        if (TheIntrinsicFile != "") {
            TheCameraParameters.readFromXMLFile(TheIntrinsicFile);
            TheCameraParameters.resize(TheInputImage.size());
        }

        // Create gui

        cv::namedWindow("thres", 1);
        cv::namedWindow("in", 1);


        char key = 0;
        int index = 0;
        // capture until press ESC or until the end of the video
        TheVideoCapturer.retrieve(TheInputImage);

        // Set SVM parameters
        MDetector.setMarkerLabeler(aruco::MarkerLabeler::create("SVM",TheSVMFile));

        while( TheVideoCapturer.grab() ) {

            // copy image
            TheVideoCapturer.retrieve(TheInputImage);
            TheInputImageCopy = TheInputImage.clone();

            index++; // number of images captured
            double tick = (double)getTickCount(); // for checking the speed
            // Detection of markers in the image passed
            MDetector.detect(TheInputImage, TheMarkers, TheCameraParameters, TheMarkerSize);
            // chekc the speed by calculating the mean speed of all iterations
            AvrgTime.first += ((double)getTickCount() - tick) / getTickFrequency();
            AvrgTime.second++;
            cout << "\rTime detection=" << 1000 * AvrgTime.first / AvrgTime.second << " milliseconds nmarkers=" << TheMarkers.size() << std::flush;

            // print marker info and draw the markers in image
            TheInputImage.copyTo(TheInputImageCopy);

            for (unsigned int i = 0; i < TheMarkers.size(); i++) {
                cout << endl << TheMarkers[i];
                TheMarkers[i].draw(TheInputImageCopy, Scalar(0, 0, 255), 1);
            }
            if (TheMarkers.size() != 0)
                cout << endl;
            // print other rectangles that contains no valid markers
            /**     for (unsigned int i=0;i<MDetector.getCandidates().size();i++) {
                     aruco::Marker m( MDetector.getCandidates()[i],999);
                     m.draw(TheInputImageCopy,cv::Scalar(255,0,0));
                 }*/



            // draw a 3d cube in each marker if there is 3d info
            if (TheCameraParameters.isValid())
                for (unsigned int i = 0; i < TheMarkers.size(); i++) {
                    CvDrawingUtils::draw3dCube(TheInputImageCopy, TheMarkers[i], TheCameraParameters);
                    CvDrawingUtils::draw3dAxis(TheInputImageCopy, TheMarkers[i], TheCameraParameters);
                }
            // DONE! Easy, right?
            // show input with augmented information and  the thresholded image
            cv::imshow("in", TheInputImageCopy);
            cv::imshow("thres", MDetector.getThresholdedImage());

            key = cv::waitKey(waitTime); // wait for key to be pressed
            if (isVideoFile)
                TheVideoCapturer.retrieve(TheInputImage);

            if(key == 27) break;

        }

    } catch (std::exception &ex)

    {
        cout << "Exception :" << ex.what() << endl;
    }
}
/************************************
 *
 *
 *
 *
 ************************************/

void cvTackBarEvents(int pos, void *) {
    // recompute
    MDetector.detect(TheInputImage, TheMarkers, TheCameraParameters);
    TheInputImage.copyTo(TheInputImageCopy);
    for (unsigned int i = 0; i < TheMarkers.size(); i++)
        TheMarkers[i].draw(TheInputImageCopy, Scalar(0, 0, 255), 1);
    // print other rectangles that contains no valid markers
    /*for (unsigned int i=0;i<MDetector.getCandidates().size();i++) {
        aruco::Marker m( MDetector.getCandidates()[i],999);
        m.draw(TheInputImageCopy,cv::Scalar(255,0,0));
    }*/

    // draw a 3d cube in each marker if there is 3d info
    if (TheCameraParameters.isValid())
        for (unsigned int i = 0; i < TheMarkers.size(); i++)
            CvDrawingUtils::draw3dCube(TheInputImageCopy, TheMarkers[i], TheCameraParameters);

    cv::imshow("in", TheInputImageCopy);
    cv::imshow("thres", MDetector.getThresholdedImage());
}
