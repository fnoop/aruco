#include "../src/markerdetector.h"
#include <mex.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    /* Declarations*/
    if (nrhs != 1) {
        mexErrMsgTxt("One input arguments required.\n");
    } else if (nlhs > 2) {
        mexErrMsgTxt("Too many output arguments.\n");
    }/* We only handle doubles */
    if (!mxIsUint8(prhs[0])) {
        mexErrMsgTxt("Input image should be double.\n");
    }
   auto img = mxGetPr(prhs[0]);
   auto cols = mxGetN(prhs[0]);
   auto rows = mxGetM(prhs[0]);
    int nd=mxGetNumberOfDimensions(prhs[0]);  
 	 mwSize nnz=0;
             auto aux=mxCreateDoubleMatrix(nnz,nd,mxREAL);
aruco::Marker m;
     /* Get the number of dimensions in the input argument. */
    auto dim_array=mxGetDimensions(prhs[0]);
    mexPrintf("img %d %d %d %d!\n",cols,rows,nd,dim_array[2]);
      cv::Mat image;
    if (dim_array[2]==1)
        image=cv::Mat(cols,rows,CV_8UC1);
    else    if (dim_array[2]==3)

         image=cv::Mat(cols,rows,CV_8UC3);
    else         { mexErrMsgTxt("Input image must have 1 or 3 channels.\n");}
 
    
}
