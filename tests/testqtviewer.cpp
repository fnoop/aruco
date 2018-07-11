#include "qtgl/qtgl.h"
#include <QtWidgets>
#include "markermap.h"
#include <opencv2/imgproc.hpp>
#include <thread>
#include "aruco.h"
#include <fstream>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sstream>
#include <string>
using namespace cv;
using namespace aruco;

string TheMarkerMapConfigFile;
bool The3DInfoAvailable = false;
float TheMarkerSize = -1;
VideoCapture TheVideoCapturer;
Mat TheInputImage, TheInputImageCopy;
CameraParameters TheCameraParameters;
MarkerMap TheMarkerMapConfig;
MarkerDetector TheMarkerDetector;
MarkerMapPoseTracker TheMSPoseTracker;
void cvTackBarEvents(int pos, void*);
double ThresParam1, ThresParam2;
int iThresParam1, iThresParam2;
int waitTime = 0;
std::map<int, cv::Mat> frame_pose_map;  // set of poses and the frames they were detected

class   QtCamera:public qtgl::Object{
    float KeyFrameSize= 0.1;
    float KeyFrameLineWidth=1;
cv::Mat _RT44;
public:
    QtCamera(){}
    void setPose(cv::Mat RT44){_RT44=RT44;}

    virtual std::string getType()const {return "Camera";}//return an string indicating the type of the subclass

    virtual void _draw_impl()  {
        if (_RT44.empty())
            return;
        const float &w = KeyFrameSize*1.5;
        const float h = w*0.75;
        const float z = w*0.6;
        cv::Mat TMatrix=_RT44.inv();
        cv::Mat Twc = TMatrix.t();
        glPushMatrix();
        glMultMatrixf(Twc.ptr<GLfloat>(0));
        glLineWidth(KeyFrameLineWidth+1);
        glColor3f(0.0f,1.0f,0.0f);
        drawPyramid(w,h,z);
        glPopMatrix();
    }
    void  drawPyramid(float w,float h,float z){
        glBegin(GL_LINES);
        glVertex3f(0,0,0);
        glVertex3f(w,h,z);
        glVertex3f(0,0,0);
        glVertex3f(w,-h,z);
        glVertex3f(0,0,0);
        glVertex3f(-w,-h,z);
        glVertex3f(0,0,0);
        glVertex3f(-w,h,z);

        glVertex3f(w,h,z);
        glVertex3f(w,-h,z);

        glVertex3f(-w,h,z);
        glVertex3f(-w,-h,z);

        glVertex3f(-w,h,z);
        glVertex3f(w,h,z);

        glVertex3f(-w,-h,z);
        glVertex3f(w,-h,z);
        glEnd();
    }
};

class   QtMarker:public qtgl::Object{

    aruco::Marker3DInfo minf3d;
public:
    QtMarker( ) {}
    QtMarker(const aruco::Marker3DInfo &mi) :minf3d(mi){}
    void operator=(const aruco::Marker3DInfo &mi){minf3d=mi;}
    virtual ~QtMarker(){}
    //has to be reimplemented
    virtual std::string getType()const {return "QtMarker";}//return an string indicating the type of the subclass

    int linewidth=2;
    vector<cv::Point3f> points;
protected:
    //you have to reimplement
    virtual void _draw_impl()  {
        glLineWidth(linewidth);
        glColor3f(0.0f,1.0f,0.0f);
        glBegin(GL_LINES);
        //a
        glVertex3f(minf3d[0].x,minf3d[0].y,minf3d[0].z);
        glVertex3f(minf3d[1].x,minf3d[1].y,minf3d[1].z);
        glColor3f(1.0f,0.0f,0.0f);
        //b
        glVertex3f(minf3d[1].x,minf3d[1].y,minf3d[1].z);
        glVertex3f(minf3d[2].x,minf3d[2].y,minf3d[2].z);

        glColor3f(0.0f,0.0f,1.0f);
        //c
        glVertex3f(minf3d[2].x,minf3d[2].y,minf3d[2].z);
        glVertex3f(minf3d[3].x,minf3d[3].y,minf3d[3].z);

        glColor3f(0.0f,1.0f,1.0f);
        //d
        glVertex3f(minf3d[3].x,minf3d[3].y,minf3d[3].z);
        glVertex3f(minf3d[0].x,minf3d[0].y,minf3d[0].z);

        //now, a small element indicating direction
        glEnd();
        if (points.size()==0)
            points=getMarkerIdPcd(minf3d);
        glBegin(GL_POINTS);
        for(auto p:points)
            glVertex3f (p.x,p.y,p.z );

        glEnd();
//        glVertex3f(0,0,0);
//        glVertex3f(0,0,size);


    }

    /**********************
     *
     *
     **********************/
    cv::Mat rigidBodyTransformation_Horn1987(const vector<cv::Point3f>& POrg, const vector<cv::Point3f>& PDst)
    {
        struct Quaternion
        {
            Quaternion(float q0, float q1, float q2, float q3)
            {
                q[0] = q0;
                q[1] = q1;
                q[2] = q2;
                q[3] = q3;
            }
            cv::Mat getRotation() const
            {
                cv::Mat R(3, 3, CV_32F);
                R.at<float>(0, 0) = q[0] * q[0] + q[1] * q[1] - q[2] * q[2] - q[3] * q[3];
                R.at<float>(0, 1) = 2.f * (q[1] * q[2] - q[0] * q[3]);
                R.at<float>(0, 2) = 2.f * (q[1] * q[3] + q[0] * q[2]);

                R.at<float>(1, 0) = 2.f * (q[1] * q[2] + q[0] * q[3]);
                R.at<float>(1, 1) = q[0] * q[0] + q[2] * q[2] - q[1] * q[1] - q[3] * q[3];
                R.at<float>(1, 2) = 2.f * (q[2] * q[3] - q[0] * q[1]);

                R.at<float>(2, 0) = 2.f * (q[1] * q[3] - q[0] * q[2]);
                R.at<float>(2, 1) = 2.f * (q[2] * q[3] + q[0] * q[1]);
                R.at<float>(2, 2) = q[0] * q[0] + q[3] * q[3] - q[1] * q[1] - q[2] * q[2];
                return R;
            }
            float q[4];
        };
        assert(POrg.size()== PDst.size());

        cv::Mat _org(POrg.size(),3,CV_32F,(float*)&POrg[0]);
        cv::Mat _dst(PDst.size(),3,CV_32F,(float*)&PDst[0]);


    //    _org = _org.reshape(1);
    //    _dst = _dst.reshape(1);
        cv::Mat Mu_s = cv::Mat::zeros(1, 3, CV_32F);
        cv::Mat Mu_m = cv::Mat::zeros(1, 3, CV_32F);
        //         cout<<_s<<endl<<_m<<endl;
        // calculate means
        for (int i = 0; i < _org.rows; i++)
        {
            Mu_s += _org(cv::Range(i, i + 1), cv::Range(0, 3));
            Mu_m += _dst(cv::Range(i, i + 1), cv::Range(0, 3));
        }
        // now, divide
        for (int i = 0; i < 3; i++)
        {
            Mu_s.ptr<float>(0)[i] /= float(_org.rows);
            Mu_m.ptr<float>(0)[i] /= float(_dst.rows);
        }

        // cout<<"Mu_s="<<Mu_s<<endl;
        // cout<<"Mu_m="<<Mu_m<<endl;

        cv::Mat Mu_st = Mu_s.t() * Mu_m;
        // cout<<"Mu_st="<<Mu_st<<endl;
        cv::Mat Var_sm = cv::Mat::zeros(3, 3, CV_32F);
        for (int i = 0; i < _org.rows; i++)
            Var_sm += (_org(cv::Range(i, i + 1), cv::Range(0, 3)).t() * _dst(cv::Range(i, i + 1), cv::Range(0, 3))) - Mu_st;
        //   cout<<"Var_sm="<<Var_sm<<endl;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                Var_sm.at<float>(i, j) /= float(_org.rows);
        //   cout<<"Var_sm="<<Var_sm<<endl;

        cv::Mat AA = Var_sm - Var_sm.t();
        //     cout<<"AA="<<AA<<endl;
        cv::Mat A(3, 1, CV_32F);
        A.at<float>(0, 0) = AA.at<float>(1, 2);
        A.at<float>(1, 0) = AA.at<float>(2, 0);
        A.at<float>(2, 0) = AA.at<float>(0, 1);
        //     cout<<"A ="<<A <<endl;
        cv::Mat Q_Var_sm(4, 4, CV_32F);
        Q_Var_sm.at<float>(0, 0) = static_cast<float>(trace(Var_sm)[0]);
        for (int i = 1; i < 4; i++)
        {
            Q_Var_sm.at<float>(0, i) = A.ptr<float>(0)[i - 1];
            Q_Var_sm.at<float>(i, 0) = A.ptr<float>(0)[i - 1];
        }
        cv::Mat q33 = Var_sm + Var_sm.t() - (trace(Var_sm)[0] * cv::Mat::eye(3, 3, CV_32F));

        cv::Mat Q33 = Q_Var_sm(cv::Range(1, 4), cv::Range(1, 4));
        q33.copyTo(Q33);
        // cout<<"Q_Var_sm"<<endl<< Q_Var_sm<<endl;
        cv::Mat eigenvalues, eigenvectors;
        eigen(Q_Var_sm, eigenvalues, eigenvectors);
        // cout<<"EEI="<<eigenvalues<<endl;
        // cout<<"V="<<(eigenvectors.type()==CV_32F)<<" "<<eigenvectors<<endl;

        Quaternion rot(eigenvectors.at<float>(0, 0), eigenvectors.at<float>(0, 1), eigenvectors.at<float>(0, 2),
                       eigenvectors.at<float>(0, 3));
        cv::Mat RR = rot.getRotation();
        //  cout<<"RESULT="<<endl<<RR<<endl;
        cv::Mat T = Mu_m.t() - RR * Mu_s.t();
        //  cout<<"T="<<T<<endl;

        cv::Mat RT_4x4 = cv::Mat::eye(4, 4, CV_32F);
        cv::Mat r33 = RT_4x4(cv::Range(0, 3), cv::Range(0, 3));
        RR.copyTo(r33);
        for (int i = 0; i < 3; i++)
            RT_4x4.at<float>(i, 3) = T.ptr<float>(0)[i];
        return RT_4x4;
    }

    vector<cv::Point3f> getMarkerIdPcd(aruco::Marker3DInfo& minfo )
    {
       auto  mult=[](const cv::Mat& m, cv::Point3f p)
        {
            assert(m.isContinuous());
            assert(m.type()==CV_32F);
            cv::Point3f res;
                const float* ptr = m.ptr<float>(0);
                res.x = ptr[0] * p.x + ptr[1] * p.y + ptr[2] * p.z + ptr[3];
                res.y = ptr[4] * p.x + ptr[5] * p.y + ptr[6] * p.z + ptr[7];
                res.z = ptr[8] * p.x + ptr[9] * p.y + ptr[10] * p.z + ptr[11];
            return res;
        };

        int id = minfo.id;
        float markerSize = minfo.getMarkerSize();
        cv::Mat rt_g2m = rigidBodyTransformation_Horn1987(aruco::Marker::get3DPoints(markerSize), minfo.points);
        cout<<rt_g2m <<endl;
        // marker id as a set of points
        string text = std::to_string(id);
        int fontFace = cv::FONT_HERSHEY_SCRIPT_SIMPLEX;
        double fontScale = 2;
        int thickness = 3;
        int baseline = 0;
        float markerSize_2 = markerSize / 2;
        cv::Size textSize = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
        cv::Mat img(textSize + cv::Size(0, baseline / 2), CV_8UC1, cv::Scalar::all(0));
        // center the text
        // then put the text itself
        cv::putText(img, text, cv::Point(0, textSize.height + baseline / 4), fontFace, fontScale, cv::Scalar::all(255),
                    thickness, 8);
        // raster 2d points as 3d points
        vector<cv::Point3f> points_id;
        for (int y = 0; y < img.rows; y++)
            for (int x = 0; x < img.cols; x++)
                if (img.at<uchar>(y, x) != 0)
                    points_id.push_back(
                        cv::Point3f((float(x) / float(img.cols)) - 0.5f, (float(img.rows - y) / float(img.rows)) - 0.5f, 0.f));

        // now,scale
        for (auto& p : points_id)
            p *= markerSize_2;
        // finally, translate
        for (auto& p : points_id)
            p = mult(rt_g2m, p);
        return points_id;
    }


};


std::shared_ptr<qtgl::GlViewer> glViewer;
std::shared_ptr<QtCamera> cameraQtObject;
void qtThread(int argc, char** argv){

    aruco::MarkerMap mmap;
    mmap.readFromFile(argv[2]);


    //check if the settings file is not created and create it
    QApplication QApp(argc, argv);
    QLocale::setDefault(QLocale::Spanish);
    glViewer=std::make_shared<qtgl::GlViewer>();
    glViewer->setZNearFar(0.01,40);
    glViewer->setWheelMovementFactor(20);
    qtgl::ViewPoint vp(true);
    vp.doZRot(180);
    glViewer->setViewPoint(vp);
    for(auto m:mmap)
        glViewer->insert(std::make_shared<QtMarker>(m));
    cameraQtObject=std::make_shared<QtCamera>();
    glViewer->insert(cameraQtObject,"camera");
    glViewer->show();
    QApp.exec();
}
/************************************
 *
 *
 *
 *
 ************************************/

void processKey(char k)
{
    switch (k)
    {
    case 's':
        if (waitTime == 0)
            waitTime = 10;
        else
            waitTime = 0;
        break;
    case 'c':
        glViewer->clear();
        break;
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

    std::thread t(qtThread,argc,argv);

    try
    {
        if (argc < 4  )
        {
            cerr << "Invalid number of arguments" << endl;
            cerr << "Usage: (in.avi|live) marksetconfig.yml camera_intrinsics.yml "<< endl;
            return false;
        }
        TheMarkerMapConfig.readFromFile(argv[2]);

        TheMarkerMapConfigFile = argv[2];
         // read from camera or from  file
        if (string(argv[1]) == "live")
            TheVideoCapturer.open(0);
        else
            TheVideoCapturer.open(argv[1]);
        // check video is open
        if (!TheVideoCapturer.isOpened())
            throw std::runtime_error("Could not open video");

        // read first image to get the dimensions
        TheVideoCapturer >> TheInputImage;

        // read camera parameters if passed
            TheCameraParameters.readFromXMLFile(argv[3]);
            TheCameraParameters.resize(TheInputImage.size());
        // prepare the detector
        string dict =TheMarkerMapConfig.getDictionary();  // see if the dictrionary is already indicated in the configuration file. It should!
        if (dict.empty()) dict = "ARUCO";
        TheMarkerDetector.setDictionary(dict);  /// DO NOT FORGET THAT!!! Otherwise, the ARUCO dictionary will be used by default!
        TheMarkerDetector.getParams().lowResMarkerSize=false;
        // prepare the pose tracker if possible
        // if the camera parameers are avaiable, and the markerset can be expressed in meters, then go

        if (TheMarkerMapConfig.isExpressedInPixels() && TheMarkerSize > 0)
            TheMarkerMapConfig = TheMarkerMapConfig.convertToMeters(TheMarkerSize);
        cout << "TheCameraParameters.isValid()=" << TheCameraParameters.isValid() << " "
             << TheMarkerMapConfig.isExpressedInMeters() << endl;
        if (TheCameraParameters.isValid() && TheMarkerMapConfig.isExpressedInMeters())
            TheMSPoseTracker.setParams(TheCameraParameters, TheMarkerMapConfig);

        // Create gui

         cv::namedWindow("in", cv::WINDOW_NORMAL);


            char key = 0;
        int index = 0;
        // capture until press ESC or until the end of the video
        cout << "Press 's' to start/stop video" << endl;
        do
        {
            TheVideoCapturer.retrieve(TheInputImage);
            TheInputImage.copyTo(TheInputImageCopy);
            index++;  // number of images captured
            // Detection of the board
            vector<aruco::Marker> detected_markers = TheMarkerDetector.detect(TheInputImage);
            // print the markers detected that belongs to the markerset
            for (auto idx : TheMarkerMapConfig.getIndices(detected_markers))
                detected_markers[idx].draw(TheInputImageCopy, Scalar(0, 0, 255), 2);

            auto succeedTracking=TheMSPoseTracker.estimatePose(detected_markers);
            if (succeedTracking){
                frame_pose_map.insert(make_pair(index, TheMSPoseTracker.getRTMatrix()));
                cout << "pose rt=" << TheMSPoseTracker.getRvec() << " " << TheMSPoseTracker.getTvec() << endl;
            }
            else{
                cerr<<"lost"<<endl;
            }
            cameraQtObject->setPose(TheMSPoseTracker.getRTMatrix());
            glViewer->updateScene();


            // show input with augmented information and  the thresholded image
            cv::imshow("in", TheInputImageCopy);

            key = cv::waitKey(waitTime);  // wait for key to be pressed
            processKey(key);

        } while (key != 27 && TheVideoCapturer.grab());


    }
    catch (std::exception& ex)

    {
        cout << "Exception :" << ex.what() << endl;
    }
}
