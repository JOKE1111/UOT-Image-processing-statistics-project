#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <ranges>
#include <cmath>

using namespace std;

struct Pixel {
    double r, g, b;
    int angle;
};

int width, height;
const double pi = 3.14159265358979323846264338327950288419716939937510;
const double transformFromRadToDeg = 180.0 / pi;

string getFileName();
template <typename T>
T inpurVer(T mini, T maxi, string message);
void makeFile(vector <Pixel> &image, int WIDTH, int HEIGHT);
vector <double> makeGaussMask(int ans);
vector <Pixel> loadImage(ifstream &f);
vector <Pixel> Blur(vector <Pixel> &image, int WIDTH, int HEIGHT);
vector <Pixel> sobelOperator(vector <Pixel> &image, int WIDTH, int HEIGHT);
vector <Pixel> canny(vector <Pixel> &image, int WIDTH, int HEIGHT);
vector <Pixel> runMask(vector <double> mask, vector <double> SobelMask, vector <Pixel> image, int kernelSize, int WIDTH, int HEIGHT);

inline bool isInScopeOfImage (int x, int y, int WIDTH, int HEIGHT) { return (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT); } 
inline void setAllPixelsTo (Pixel &p, double val) { p.r = p.g = p.b = val; } 
inline void separator () { cout << "\n*********************************************\n\n"; } // used to print stars and newlines all around the code, edit it here and it'll be editied everywhere


int main()
{
    string filename = getFileName(); // get the name of the image file which must be in bmp

    ifstream f (filename, ios::binary); 
    if (!f) {
        cout << "couldn't read the file\n";
        return 1;
    }
    unsigned char signature[2];
    f.read(reinterpret_cast<char*>(signature), 2);
    if (!f || signature[0] != 'B' || signature[1] != 'M') {
        cout << "not a bmp file\n";
        return 1;
    }

    vector <Pixel> image = loadImage(f); 

    separator();
    int choice = inpurVer <int> (1, 3, "What do you want to do to the image?:\n1.Blur\n2.Sobel Edge Detection\n3.Canny Edge Detection\n"); // asking the user for the input
    switch (choice)
    {
        case 1:
            image = Blur(image, width, height); 
            break;
        case 2:
            image = sobelOperator(image, width, height); 
            break;
        case 3:
            vector <Pixel> sobel = sobelOperator(image, width, height);
            image = canny(sobel, width, height); // do the canny edge detection, we did the sobel first because the output of the sobel is the input of the canny
            break;
    }
    makeFile(image, width, height); 
    return 0;
}


string getFileName() // we use this function to get safe string input
{
    string filename;
    while (true) {
        cout << "enter the file name: ";
        if (cin >> filename)
            return filename;
        else {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "invalid file name please try again\n";
        }
    }
}

template <typename T> 
T inpurVer(T mini, T maxi, string message) // we use this function to get safe inpur wether it be int, float, double or any other number values
{
    T i;
    while (true) {
        cout << message; // prints out the given message (we assume that the message has /n at the end)
        if (cin >> i) {
            if (i < mini || i > maxi) // it checks the if the value violates the boundaries given 
                cout << "Please choose between " << mini << " and "<< maxi << " !\n";
            else return i;
        }
        else {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid Please try again\n";
        }
    }
}


vector <Pixel> Blur(vector <Pixel> &image, int WIDTH, int HEIGHT) // the blur function that does the basic and gaussian blur
{
    vector <Pixel> newImage (WIDTH * HEIGHT); 

    // these are some common 3X3, 5X5 and 7X7 gaussian blur masks
    vector <vector <double>> gaussianMasks = {{ 
    1.0, 2.0, 1.0,
    2.0, 4.0, 2.0,
    1.0, 2.0, 1.0},
    {
    1.0,  4.0,  7.0,  4.0,  1.0,
    4.0, 16.0, 26.0, 16.0,  4.0,
    7.0, 26.0, 41.0, 26.0,  7.0,
    4.0, 16.0, 26.0, 16.0,  4.0,
    1.0,  4.0,  7.0,  4.0,  1.0},
    {
    0.0,  0.0,  1.0,  2.0,  1.0,  0.0,  0.0,
    0.0,  3.0, 13.0, 22.0, 13.0,  3.0,  0.0,
    1.0, 13.0, 59.0, 97.0, 59.0, 13.0,  1.0,
    2.0, 22.0, 97.0,159.0, 97.0, 22.0,  2.0,
    1.0, 13.0, 59.0, 97.0, 59.0, 13.0,  1.0,
    0.0,  3.0, 13.0, 22.0, 13.0,  3.0,  0.0,
    0.0,  0.0,  1.0,  2.0,  1.0,  0.0,  0.0}};

    int ans;
    vector <double> theBlurMask; 
    separator();
    // ask the user what type of blur do they want to apply
    if (inpurVer <int> (1, 2, "Do you want to use:\n1.Basic Blur\n2.Gaussian Blur\n") == 1) {
        // Basic Blur
        separator();

        // the dimentions of the blur mask, the higher the more blurred the image will be
        ans = inpurVer <int> (1, 10, "Choose the size of the mask:\n1.3X3\n2.5X5\n3.7X7\n4.9X9\n5.11X11\n6.13X13\n7.15X15\n8.17X17\n9.19X19\n10.21X21\n");

        // making the basic blur mask, which is all 1s
        for (int i = 0, n = (ans * 2 + 1) * (ans * 2 + 1); i < n; i++)
            theBlurMask.push_back(1.0);
    }
    else {
        // Gaussian Blur
        separator();

        ans = inpurVer <int> (1, 3, "Choose the size of the mask:\n1.3X3\n2.5X5\n3.7X7\n"); 
        separator();

        if (inpurVer <int> (1, 2, "Do you want to:\n1.Use a premade mask\n2.Approximate a mask by giving a value for sigma\n") == 1) {
            theBlurMask = gaussianMasks[ans - 1]; }
        else { 
            theBlurMask = makeGaussMask(ans); }
    }

    separator();
    cout << "Making the Blur filter...\n";
    newImage = runMask(theBlurMask, {}, image, ans, WIDTH, HEIGHT); 
    cout << "Done the Blur filter!\n";
    return newImage;
}


vector <double> makeGaussMask(int ans) // this function is used to make a gaussian mask with a given sigma
{
    /* the user will choose 1, 2, or 3, 
    when we double it and add one, we will get the width or height of the mask:
    1: 1*2 + 1 = 3
    2: 2*2 + 1 = 5
    3: 3*2 + 1 = 7 */
    int k = 2 * ans + 1;

    // make a vector that holds the mask's floating-point values, of length k^2, because it is also flat, and square, that why the k^2
    vector <double> tempmask (k * k); 
    
    double sigma;
    separator();
    // this is the sigma value for the equation:
    // G(x, y) = 1/(2*π*σ^2) * e^(-(x^2 + y^2)/(2*σ^2))
    cout << "Give a value to sigma: ";
    cin >> sigma;
    sigma *= sigma;
    // calculating 1/(2*π*σ^2)
    double coeff = 1/(2 * sigma * pi);
    int index = 0;
    // then for each value in the mask we'll calculate the G of it
    for (int i = -ans; i <= ans; i++) {
        for (int j = -ans; j <= ans; j++) {
            // calculating -(x^2 + y^2)/(2*σ^2)
            double power = -((i*i + j*j)/(2 * sigma));
            // then pluging everything together
            tempmask[index++] = coeff * exp(power);
        }
    }
    // the reuslting values will be floating points, so we'll divide by the smallest value to get integer point values (not standard)
    double divBy = 1.0/min_element(tempmask.begin(), tempmask.end())[0];
    // the last mask that holds the integer point values
    vector <double> mask (k * k); 
    // we just do val/smallestVal
    for (int i = 0; i < k * k; i++)
        mask[i] = round(divBy * tempmask[i]);
    return mask;
}


vector <Pixel> sobelOperator(vector <Pixel> &image, int WIDTH, int HEIGHT) // this function applies the Sobel Operator on the vector that holds the pixels
{
    separator();
    // ask the user if they want to apply blur for smoothing the images before we apply the Sobel Operator
    if (inpurVer <int> (1, 2, "Do you want to use Blur before applying the edge detector?:\n1.Yes\n2.No\n") == 1) {
        image = Blur(image, WIDTH, HEIGHT); }

    vector <double> verticalSobelMaskGx = {
        -1.0, 0.0, 1.0,
        -2.0, 0.0, 2.0,
        -1.0, 0.0, 1.0
    };
    vector <double> horizontalSobelMaskGy = {
        -1.0, -2.0, -1.0,
         0.0,  0.0,  0.0,
         1.0,  2.0,  1.0
    };
     
    separator();
    cout << "Making the Sobel filter...\n";
    vector <Pixel> newImage = runMask(verticalSobelMaskGx, horizontalSobelMaskGy, image, 1, WIDTH, HEIGHT);
    cout << "Done the Sobel filter!\n";
    return newImage;
}


vector <Pixel> canny(vector <Pixel> &image, int WIDTH, int HEIGHT) // the Canny Edge detection
{
    vector <Pixel> newImage (WIDTH * HEIGHT);
    
    separator();
    // promtp the user to know the high threshold and the low one, knowing that it is between 0 and 1
    // because it is relative as per the brightest pixel
    float The_High_ThreshHold = inpurVer <double> (0.0, 1.0,                 "Thresh hold 1 (High): ");
    float The_Low_ThreshHold  = inpurVer <double> (0.0, The_High_ThreshHold, "Thresh hold 2 (Low): ");
    int Highest_Brightness = 0;

    separator();
    /* this is for pixels in the edge direction: -, /, |, \ */
    const int dirX[4][2] = {{-1,1},{1,-1},{0,0},{-1,1}};
    const int dirY[4][2] = {{0,0},{1,-1},{-1,1},{1,-1}};
    cout << "Making the canny operator...\n";
    
    /*  =============================
        Non-Maximum Suppression (NMS):
        ============================= */
    for (int i = 0; i < HEIGHT; i++) {
        int row = i * WIDTH;
        for (int j = 0; j < WIDTH; j++) {
            int idx = row + j;
            // take the angle of the edge that is stored in the pixel, add 22.5 and divide by 45 to get a value that represents the angle, we do %4 so angles >= 157.5 wrap around to 0
            int dir = int((image[idx].angle + 22.5) / 45.0) % 4;
            int x1 = j + dirX[dir][0];
            int x2 = j + dirX[dir][1];
            int y1 = i + dirY[dir][0];
            int y2 = i + dirY[dir][1];
            double pix1 = 0.0, pix2 = 0.0;
            
            if (isInScopeOfImage(x1, y1, WIDTH, HEIGHT)) 
                pix1 = image[y1 * WIDTH + x1].r;
            if (isInScopeOfImage(x2, y2, WIDTH, HEIGHT))
                pix2 = image[y2 * WIDTH + x2].r;

            // keep a log of the brightest pixel
            if (image[idx].r > Highest_Brightness)
                Highest_Brightness = image[idx].r;

            // if the pixel is not local-maximum; delete it (set it to 0)
            if (image[idx].r < pix1 || image[idx].r < pix2) setAllPixelsTo(newImage[idx], 0.0);
            // if it is local maximum, keep it
            else                                            setAllPixelsTo(newImage[idx], image[idx].r);
        }
    }
    // the relative values of the higher and lower thresholds
    int High = round(The_High_ThreshHold * Highest_Brightness);
    int Low = round(The_Low_ThreshHold * Highest_Brightness);
    

    bool changes;
    do {
        changes = false;
        for (int i = 0; i < HEIGHT; i++) {
            int row = i * WIDTH;
            for (int j = 0; j < WIDTH; j++) {
                int idx = row + j;
                /*  ================
                    Double Threshold:
                    ================ */
                // (strong edge)
                if (newImage[idx].r >= High) continue; 
                else if (newImage[idx].r < Low) {
                    // (no edge)
                    setAllPixelsTo(newImage[idx], 0.0);
                    continue;
                }
                // (weak edge)
                /*  ========================
                    Hysteresis Edge Tracking:
                    ========================*/
                for (int ni : {-1, 0, 1}) {
                    bool connected = false;
                    for (int nj : {-1, 0, 1}) {
                        if (ni == 0 && nj == 0) continue;
                        int x = j + nj;
                        int y = i + ni;
                        if (isInScopeOfImage(x, y, WIDTH, HEIGHT)) {
                            // check if the weak edge pixel is connected to a strong edge, if so set it to the high threshold
                            if (newImage[y * WIDTH + x].r >= High) {
                                setAllPixelsTo(newImage[idx], High);
                                connected = true;
                                changes = true;
                                break;
                            }
                        }
                    }
                    if (connected) break;
                }
            }
        }
    } while (changes); // after doing the Hysteresis Edge Tracking recursively, many times perhaps, and there are not changes to be done, terminate the loop
    cout << "Done making the canny Operator!";
    return newImage;
}

vector <Pixel> runMask(vector <double> mask, vector <double> SobelMask, vector <Pixel> image, int kernelSize, int WIDTH, int HEIGHT) // this function is for running a mask on an image flat vector
{
    vector <Pixel> newImage (WIDTH * HEIGHT);
    // check if the second mask for the Sobel Operator is empty (blur) or not (Sobel)
    // because this function is used by both the blur and the Sobel Operator
    bool sobel = !SobelMask.empty();
    for (int i = 0; i < HEIGHT; i++) {
        int row = i * WIDTH;
        for (int j = 0; j < WIDTH; j++) {
            int idx = row + j;
            double r, g, b, nc, Gx, Gy;
            r = g = b = nc = Gx = Gy = 0.0;
            for (int ti = -kernelSize; ti <= kernelSize; ti++) {
                int y = i + ti;
                int maskRow = y * WIDTH;
                for (int tj = -kernelSize; tj <= kernelSize; tj++) {
                    int x = j + tj;
                    if (isInScopeOfImage(x, y, WIDTH, HEIGHT)) {
                        int kernelTurn = (ti + kernelSize) * (kernelSize * 2 + 1) + (tj + kernelSize);
                        int theImageInTheMaskIdx = maskRow + x;
                        // check the type of the mask
                        if (sobel) {
                            // Sobel
                            double gray = (image[theImageInTheMaskIdx].r + image[theImageInTheMaskIdx].g + image[theImageInTheMaskIdx].b)/3.0;
                            Gx +=  gray * mask[kernelTurn];
                            Gy +=  gray * SobelMask[kernelTurn];
                        }
                        else {
                            // Blur
                            double tr = mask[kernelTurn];
                            r  += image[theImageInTheMaskIdx].r * tr;
                            g  += image[theImageInTheMaskIdx].g * tr;
                            b  += image[theImageInTheMaskIdx].b * tr;
                            nc += tr;
                        }
                    }
                }
            }
            if (sobel) {
                // Sobel
                double val = sqrt(Gx * Gx + Gy * Gy); 
                // check if the pixel value is higher than 255 which is the maximum value
                if (val > 255)
                    val = 255;
                setAllPixelsTo(newImage[idx], val);
                // calculate the angel of the edge for the canny edge detector
                double angle = atan2(Gy, Gx) * transformFromRadToDeg; // notice that we turn the result to Degrees
                // if the angle is negative, just add 180 so we get the responding positive value
                if (angle < 0)
                    angle += 180.0;
                newImage[idx].angle = angle;
            }
            else {
                // Blur
                // get the weighted or normal mean for the blur types
                newImage[idx].r = r/nc;
                newImage[idx].g = g/nc;
                newImage[idx].b = b/nc;
            }
        }
    }
    return newImage;
}


vector <Pixel> loadImage(ifstream &f) // this function is used to read a bmp formatted file into a flat vector of pixels
{
    separator();
    cout << "Loading the image...\n";
    int temph;
    /* The bmp file headers for clearer understanding
        ============
        BITMAPFILEHEADER (14 bytes)
        signature "BM" (2) <- we checked for this in the filename in main
        file size in bytes (4)
        reserved1 (2) = 0
        reserved2 (2) = 0
        pixel data offset (4) <- we'll use this later to skip the headers in case they change the header's size
        ============
        BITMAPINFOHEADER (40 bytes)
        header size (4) = 40
        image width in pixels (4) <- the Width of the image
        image height in pixels (4)  (+ bottom-up, − top-down) <- the height of the image
        color planes (2) = 1
        bits per pixel (2)
        compression method (4) = 0 (BI_RGB)
        image data size (4)
        horizontal resolution (4)
        vertical resolution (4)
        colors used (4)
        important colors (4)
        ============ */

    f.seekg(18, fstream::beg);
    f.read((char*)&width, 4);
    f.read((char*)&temph, 4);
    height = abs(temph);
    // check for flipped height 
    bool InvertedHeight = (temph > 0);
    f.seekg(10, fstream::beg);   
    uint32_t offset;
    f.read((char*)& offset, 4);
    f.seekg(offset, fstream::beg);   
    
    vector <Pixel> image (width * height);
    int padding = (4 - ((width * 3) % 4)) % 4;
    for (int i = 0; i < height; i++) {
        // we check if the height is flipped and reverse it back ourselves
        int y = (InvertedHeight) ? i : height - 1 - i;
        int row = y * width;
        for (int j = 0; j < width; j++) {
            int idx = row + j;

            // the color table in bmp is bgr not rgb
            image[idx].b = f.get();
            image[idx].g = f.get();
            image[idx].r = f.get();
        }
        f.ignore(padding);
    }
    cout << "File was loaded successfully\n";
    f.close();
    return image;
}


void makeFile(vector <Pixel> &image, int WIDTH, int HEIGHT) // this function is given a flat vector and it constructs a bmp file
{
    separator();
    cout << "Making the file...\n";
    int realh = abs(HEIGHT);
    int rowSize = WIDTH * 3;
    int padding = (4 - rowSize % 4) % 4;
    int dataSize = (rowSize + padding) * HEIGHT;
    int fileSize = 54 + dataSize;

    ofstream f ("output.bmp", ios::binary | ios::trunc);
    f.put('B'); f.put('M');
    f.write((char*)& fileSize, 4);
    uint32_t zero = 0;
    f.write((char*)& zero, 4);
    uint32_t offset = 54;
    f.write((char*)& offset, 4);

    uint32_t infoSize = 40;
    f.write((char*)& infoSize, 4);
    f.write((char*)& WIDTH, 4);
    f.write((char*)& realh, 4);
    uint16_t planes = 1;
    uint16_t bpp = 24;
    f.write((char*)& planes, 2);
    f.write((char*)& bpp, 2);
    f.write((char*)& zero, 4);
    f.write((char*)& dataSize, 4);
    f.write((char*)& zero, 16);

    uint8_t pad[3] = {0,0,0};
    for (int i = 0; i < HEIGHT; i++) {
        int row = i * WIDTH;
        for (int x = 0; x < WIDTH; x++) {
            int idx = row + x;
            f.put((uint8_t) image[idx].b);
            f.put((uint8_t) image[idx].g);
            f.put((uint8_t) image[idx].r);
        }
        f.write((char*)pad, padding);
    }
    f.close();
    cout << "Done making the file!\n";
}
