#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <fstream>
using namespace cv;
using namespace std;
vector<int> avg_rgb(int k, int h, Mat_<Vec3d> image){
    int r = image(h,k)[2];
    int g = image(h,k)[1];
    int b = image(h,k)[0];
    for(int i = 1;i<10;i++){
        r += image(h+i,k)[2];
        r += image(h+i,k+i)[2];
        r += image(h,k+i)[2];
        r += image(h,k-i)[2];
        g += image(h+i,k)[1];
        g += image(h+i,k+i)[1];
        g += image(h,k+i)[1];
        g += image(h,k-i)[1];
        b += image(h+i,k)[0];
        b += image(h+i,k+i)[0];
        b += image(h,k+i)[0];
        b += image(h,k-i)[0];
    }
    r = r/41;
    b = b/41;
    g = g/41;
    return {r, g, b};
}
int main(int argc, const char* argv[])
{
    Mat gray;
    Mat image;
    Mat_<Vec3d> im;
    int ht = 100; //higher threshold value for canny edge, lower is twice as small (ht/2)
    int ct = 30; //center threshold, votes
    int minRad = 80; //minimum considered detection
    int maxRad = 150;
    int mdd = 30; //min distance denominator: what the min distance between centers is divided into (larger value = smaller mindist)
    bool morethanq = false; //there are coins higher in value than quarter
    for(int i =0; i<argc; i++){ 
        if(strcmp(argv[i], "-f") == 0){
            image = imread(argv[i+1],1);
            im =imread(argv[i+1],1);
            gray = imread(argv[i+1],IMREAD_GRAYSCALE); //grayscale input image
        }
        if(strcmp(argv[i], "-ht") == 0){ //cannyedge thresh
            ht = stoi(argv[i+1]);
        }
        if(strcmp(argv[i], "-ct") == 0){ //center voting thresh
            ct = stoi(argv[i+1]);
        }
        if(strcmp(argv[i], "-minr") == 0){ //minimum radius detected by hough circles
            minRad = stoi(argv[i+1]);
        }
        if(strcmp(argv[i], "-maxr") == 0){ //maximum radius 
            maxRad = stoi(argv[i+1]);
        }
        if(strcmp(argv[i], "-mdd") == 0){
            mdd = stoi(argv[i+1]);
        }
        if(strcmp(argv[i], "-highestcoin") == 0){
            if(strcmp(argv[i+1], "quarter") !=0){
                morethanq = true;
            }
        }
    }
    imwrite("./imageg.jpg", gray); //create grayscale image
    medianBlur(gray, gray, 5);
    GaussianBlur(gray, gray, Size(3, 3), 0, 0, BORDER_DEFAULT);
    //canny edge
    Mat edges, cannyout;
    Canny(gray, cannyout, ht/1.4, ht, 3); //grayscale image, edges out, low thresh, high thresh, sobel kernel (3x3)
    edges = Scalar::all(0);
    gray.copyTo(edges, cannyout);
    imwrite("./imagef.jpg", edges);
  //  hough transform: identify the centers and radiuses using OpenCV and hough transform
    vector<Vec3f> circles;
    HoughCircles(gray, circles, HOUGH_GRADIENT, 1, //grayscale image, vector w xyrad for circles, gradient, inverse resolution ratio, min distance between centers, threshold for cannyedge, threshold for centers, minrad, maxrad;
                 gray.rows/mdd,  
                 ht, ct, minRad, maxRad);
    int maxdetectrad = 0; //the biggest detected radius
    for( size_t i = 0; i < circles.size(); i++ ) //display centers and outlines
    {
        Vec3i c = circles[i];
        Point center = Point(c[0], c[1]); // c = h, k, rad
        // circle center: #circle(image, center, 1, Scalar(0,100,100), 3, LINE_AA);
        int radius = c[2];
        if(radius>maxdetectrad){
            maxdetectrad = radius;
        }
        //pennies red, quarters purple, nickel yellow
        circle(image, center, radius, Scalar(0,0,255), 4, LINE_AA); //detected circles in red, thickness = 4 TEMPORARILY BLACK
    }
    imwrite("./imageCircles.jpg",image);
    int pennycount = 0;
    int nickelcount= 0; 
    int dimecount= 0;
    int quartercount= 0; 
    int halfdolcount = 0; 
    int silvercount = 0;
    for( size_t i = 0; i < circles.size(); i++ ) //display centers and outlines
    {
        Vec3i c = circles[i];
        Point center = Point(c[0], c[1]); // c = h, k, rad
        cout<<"rad: "<<c[2]<<"\n";
        // circle center: #circle(image, center, 1, Scalar(0,100,100), 3, LINE_AA);
        int radius = c[2];
        //pennies red, quarters purple, nickel yellow
        circle(image, center, radius, Scalar(0,0,255), 4, LINE_AA); //detected circles in red, thickness = 4 TEMPORARILY BLACK
        int r = image.at<Vec3b>(c[1], c[0])[2];//getting the pixel values//
        if(!morethanq){ //categorizing
            if(radius >=0.9*maxdetectrad){ //quarter = purple
            circle(image, center, radius, Scalar(255,0,150), 4, LINE_AA); 
            quartercount+=1;
            }
            else if(radius>=0.84*maxdetectrad){ //nickels fo sho 
                circle(image, center, radius, Scalar(0,255,255), 4, LINE_AA); //yellow 
                nickelcount+=1;
            }
            else{ //separate nickels dimes and pennies
                vector<int> rgb = avg_rgb(c[0],c[1],im);
                int r = rgb[0]; int g = rgb[1]; int b = rgb[2];
                if(r-(g+b)/2 > 20 or(r+g+b)/3 <=65){ //pennies, either red or super dark
                    circle(image, center, radius, Scalar(0,0,255), 4, LINE_AA);
                    pennycount+=1;
                }
                else if(radius<.69*maxdetectrad){ //dime blue
                    circle(image, center, radius, Scalar(255,0,0), 4, LINE_AA);
                    dimecount+=1;
                }
                else{
                    circle(image, center, radius, Scalar(0,255,255), 4, LINE_AA);
                    nickelcount+=1;
                }
            }
        } else{
            cout<<"hard"<<"\n";
            if(radius>0.95*maxdetectrad){ //one/silver dollar, pink
                circle(image, center, radius, Scalar(255,0,255), 4, LINE_AA); 
                silvercount+=1;
            }
            else if(radius>0.7*maxdetectrad){ //half dollar
                circle(image, center, radius, Scalar(0,255,0), 4, LINE_AA); 
                halfdolcount+=1;
            }
            else if(radius >=0.63*maxdetectrad){ //quarter = purple
                circle(image, center, radius, Scalar(255,0,150), 4, LINE_AA); 
                quartercount+=1;
            }
            else if(radius>=0.54*maxdetectrad){ //nickels fo sho 
                circle(image, center, radius, Scalar(0,255,255), 4, LINE_AA); //yellow 
                nickelcount+=1;
            }
            else{ //separate nickels dimes and pennies
                vector<int> rgb = avg_rgb(c[0],c[1],im);
                int r = rgb[0]; int g = rgb[1]; int b = rgb[2];
                if(r-(g+b)/2 > 20 or(r+g+b)/3 <=65){ //pennies, either red or super dark
                    circle(image, center, radius, Scalar(0,0,255), 4, LINE_AA);
                    pennycount+=1;
                }
                else if(radius<.45*maxdetectrad){ //dime blue
                    circle(image, center, radius, Scalar(255,0,0), 4, LINE_AA);
                    dimecount+=1;
                }
                else{
                    circle(image, center, radius, Scalar(0,255,255), 4, LINE_AA);
                    nickelcount+=1;
                }
            }
        }
    }
    imwrite("./imageCoins.jpg", image);
    //generate text file with cost
    ofstream outfile("results.txt");
    outfile<<"Detected Change: \n"<<silvercount<<" silver coins, "<<halfdolcount<<" half dollars, "<<quartercount<<" quarters, "<<
        nickelcount<<" nickels, "<<dimecount<<" dimes, and "<<pennycount<<" pennies. \n";
    cout<<"Detected Change: \n"<<silvercount<<" silver coins, "<<halfdolcount<<" half dollars, "<<quartercount<<" quarters, "<<
        nickelcount<<" nickels, "<<dimecount<<" dimes, and "<<pennycount<<" pennies. \n";
    double total = silvercount+ 0.5*halfdolcount + .25*quartercount + .05*nickelcount + 0.1*dimecount + 0.01*pennycount;
    outfile<<"Total: $"<<total<<"\n";
    cout<<"Total: $"<<total<<"\n";
    outfile.close();
    waitKey(0);
    return 0;
}

