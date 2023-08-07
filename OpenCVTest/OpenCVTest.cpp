#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <filesystem>
#include <string>
#include <Windows.h>

using namespace cv;
using namespace std;

namespace fs = std::filesystem;

int xOff = 25;
int yOff = 25;

//-1: error 0: no matches 1: matches
int visualLocateMatch(Mat ss, Mat temp)
{
    int matchCount = 0;

    Mat ref = ss;
    Mat tpl = temp;
    Mat img2;
    Mat result;

    double threshhold = 0.999;//How accurate the template needs to be to the image

    double minVal;//The least likely match num
    double maxVal;//The most likely match num

    Point minLoc;//The location on the Mat of the minVal
    Point maxLoc;//The location on the Mat of the maxVal

    Point location;
    Point botton_right;

    if (ref.empty() || tpl.empty())
    {
        cout << "Error reading file(s)!" << endl;
        return -1;
    }

    TemplateMatchModes methods[6] = { TM_SQDIFF , TM_SQDIFF_NORMED , TM_CCORR, TM_CCORR_NORMED, TM_CCOEFF , TM_CCOEFF_NORMED };

    int method = 3;
    int tmp = 0;

    img2 = ref.clone();

    matchTemplate(img2, tpl, result, methods[method]);
        
    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

    while (true && (tmp <= 480)){


        if (maxVal >= threshhold) {

            minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

            if (methods[method] == TM_SQDIFF || methods[method] == TM_SQDIFF_NORMED) {
                location = minLoc;
            }
            else {
                location = maxLoc;
            }

            botton_right.x = location.x + tpl.cols;
            botton_right.y = location.y + tpl.rows;

            rectangle(img2, location, botton_right, 255, 1);
            result.at<float>(location) = 0;
            matchCount++;
        }
        else {
            break;
        }
        tmp++;
    }

    imshow("Result", img2);

    if (matchCount == 0) {
        return 0;
    }

    return 1;
}

void getMatchPixelLocations(list<Point>& listOfLocations,Mat ss, Mat temp, double threshhold) {
    Mat ref = ss;
    Mat tpl = temp;
    Mat result;

    double minVal;//The least likely match num
    double maxVal;//The most likely match num

    Point minLoc;//The location on the Mat of the minVal
    Point maxLoc;//The location on the Mat of the maxVal

    Point location;//Top left location of the match

    int tmp = 0;//Prevent an infinite loop

    //TM_CCORR_NORMED
    matchTemplate(ref, tpl, result, TM_CCORR_NORMED);

    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

    Mat img2 = ref.clone();//TEMP
    Point botton_right; //TEMP

    while (true && (tmp <= 480)) {
        if (maxVal >= threshhold) {

            minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

            location = maxLoc;

            botton_right.x = location.x + tpl.cols; //TEMP
            botton_right.y = location.y + tpl.rows; //TEMP
       
                listOfLocations.push_back(location);
                result.at<float>(location) = 0;

                location.x = ceil((int)(location.x / xOff) - 1); //TEMP
                location.y = ceil((int)(location.y / yOff) - 1); //TEMP

                location.x = (location.x + 1) * xOff; //TEMP
                location.y = (location.y + 1) * yOff; //TEMP

                rectangle(img2, location, botton_right, 100, 1);//TEMP (table to pos)

        }
        else {
            break;
        }
        tmp++;
    }
    imshow("Result", img2);//TMP
}

void fillTable(list<Point>& listOfLocations, int table[20][24], int num) {
    //Converts pixel locations to table locations
    for (Point p : listOfLocations)
    {
        table[(int)ceil((p.y / yOff))][(int)ceil((p.x / xOff))] = num;
    }
}

void leftClick(int x, int y) {
    SetCursorPos(x, y);
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}
void rightClick(int x, int y) {
    //Set Pos to top left to open tab with Minesweeper
    SetCursorPos(x, y);
    mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
    mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
}

BITMAPINFOHEADER createBitmapHeader(int width, int height)
{
    BITMAPINFOHEADER  bi;

    // create a bitmap
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;  //this is the line that makes it draw upside down or not
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    return bi;
}

Mat captureScreenMat(HWND hwnd)
{
    Mat src;

    // get handles to a device context (DC)
    HDC hwindowDC = GetDC(hwnd);
    HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

    // define scale, height and width
    int screenx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int screeny = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    // create mat object
    src.create(height, width, CV_8UC4);

    // create a bitmap
    HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
    BITMAPINFOHEADER bi = createBitmapHeader(width, height);

    // use the previously created device context with the bitmap
    SelectObject(hwindowCompatibleDC, hbwindow);

    // copy from the window device context to the bitmap device context
    StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, screenx, screeny, width, height, SRCCOPY);  //change SRCCOPY to NOTSRCCOPY for wacky colors !
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);            //copy from hwindowCompatibleDC to hbwindow

    // avoid memory leak
    DeleteObject(hbwindow);
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(hwnd, hwindowDC);

    return src;
}

void toggle(bool &x) {
    if (x) {
        x = false;
    }
    else {
        x = true;
    }
}

int main() {
    //Set Pos to top left to open tab with Minesweeper
    leftClick(0,20);

    // capture image
    HWND hwnd = GetDesktopWindow();
    Mat src = captureScreenMat(hwnd);

    // save img
    imwrite("Screenshot.jpg", src);
    
    //Find Image and store them into Mat
    fs::path currentDirectoryPath = fs::current_path();
    string stringpath = currentDirectoryPath.generic_string(); //Convert Path to string

    //Get paths of screenshot and referance images
    string screenshotPath = stringpath + "/Screenshot.jpg";
    Mat screenshot = imread(screenshotPath, IMREAD_GRAYSCALE);

    //                               height  (500)             width (600)
    Mat cropped_image = screenshot(Range(515, 1015), Range(980, 1580));

    //                            0                                      1                                   2                                   3                                    4
    string numPaths[5] = {stringpath + "/dependencies/1.jpg", stringpath + "/dependencies/2.jpg", stringpath + "/dependencies/3.jpg", stringpath + "/dependencies/4.jpg", stringpath + "/dependencies/5.jpg" };
    Mat nums[5];

    int i = 0;
    for (string nPath : numPaths) {
        nums[i] = imread(nPath, IMREAD_GRAYSCALE);
        i++;
    }

    //       Y   X
    int table[20][24] = { {0} };

    list<Point> pixelLocations;


    /*
    Optimal Threshholds:
    1: 0.996
    2: 0.995
    3: 0.995
    4: 0.995
    */

   
    getMatchPixelLocations(pixelLocations, cropped_image, nums[2], 0.99845);//TMP
    fillTable(pixelLocations, table, 1);//TMP
    
    
    /*
    for (int i = 0; i < 4; i++) {
        getMatchPixelLocations(pixelLocations, cropped_image, nums[i], 0.996);
        fillTable(pixelLocations, table, i + 1);
        pixelLocations.clear();
    }
    */

    
    /*
    
    //Fill in the "0" tiles with "-1"
    bool set = false;
    int toggleX = 0;

    for (int y = 0; y < 20; y++) {
        for (int x = 0; x < 24; x++) {
            if (set) {
                if (table[y][x] != 0) {
                    toggle(set);
                }
                else {
                    table[y][x] = -1;
                }
            }
            else if (table[y][x] != 0) {
                if (table[y][x+1] == 0) {
                    toggle(set);
                    toggleX = x;
                }
            }
        }
        if (set) {
            for (int i = 24; i > toggleX; i--) {
                table[y][i] = 0;
            }
        }
        set = false;
    }
    */


    /*
    int unknowns = 0;
    Point save;

    //Go through the table from left to right
    for (int y = 0; y < 20; y++) {
        for (int x = 0; x < 24; x++) {

            //Check if selected pixel is 1
            if (table[y][x] == 1) {

                //Search surrounding cells
                for(int i = -1; i < 1; i++){
                    for (int j = -1; j < 1; j++) {

                        //Check a pixel of the surrounded cell
                        if ((table[y + i][x + j] == 0) && i != 0 && j != 0) {
                            unknowns++;
                            save.x = x + j;
                            save.y = y + i;
                        }

                    }
                }
                if (unknowns == 1) {
                    table[save.y][save.x] = -2;
                    cout << "Corner";
                }
                unknowns = 0;
            }

        }
    }

    */


    
    //Print the table to console

    for (auto& rows : table){
        for (auto& elem : rows)
        {
            cout << elem << " ";
        }
        cout << "\n";
    } 

    waitKey(0);
    return 0;
}