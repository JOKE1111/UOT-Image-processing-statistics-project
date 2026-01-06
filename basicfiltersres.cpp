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
void makeFile(vector <Pixel> &pic, int WIDTH, int HEIGHT);
vector <double> makeGaussMask(int ans);
vector <Pixel> loadPic(ifstream &f);
vector <Pixel> gaussBlur(vector <Pixel> &pic, int WIDTH, int HEIGHT);
vector <Pixel> sobelOperator(vector <Pixel> &pic, int WIDTH, int HEIGHT);
vector <Pixel> canny(vector <Pixel> &pic, int WIDTH, int HEIGHT);
vector <Pixel> runMask(vector <double> mask, vector <double> SobelMask, vector <Pixel> image, int kernelSize, int WIDTH, int HEIGHT);

inline bool isInScopeOfImage (int x, int y) { return (x >= 0 && x < width && y >= 0 && y < height); }
inline void setAllPixelsTo (Pixel &p, double val) { p.r = p.g = p.b = val; }
inline void separator () { cout << "\n*********************************************\n\n"; }


int main()
{
    string filename = getFileName();

    ifstream f (filename, ios::binary); //file name , must be bmp
    if (!f) // if the file doesn't exist exit
    {
        cout << "couldn't read the file\n";
        return 1;
    }
    unsigned char signature[2];
    f.read(reinterpret_cast<char*>(signature), 2);
    if (!f || signature[0] != 'B' || signature[1] != 'M') // the signature is BM
    {
        cout << "not a bmp file\n";
        return 1;
    }

    vector <Pixel> pic = loadPic(f);

    separator();
    int choice = inpurVer <int> (1, 3, "What do you want to do to the image?:\n1.Blur\n2.Sobel Edge Detection\n3.Canny Edge Detection\n");
    switch (choice)
    {
        case 1:
            pic = gaussBlur(pic, width, height);
            break;
        case 2:
            pic = sobelOperator(pic, width, height);
            break;
        case 3:
            vector <Pixel> sobel = sobelOperator(pic, width, height);
            pic = canny(sobel, width, height);
            break;
    }
    makeFile(pic, width, height);
    return 0;
}


string getFileName()
{
    string filename;
    while (true)
    {
        cout << "enter the file name: ";
        if (cin >> filename)
            return filename;
        else
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "invalid file name please try again\n";
        }
    }
}

template <typename T>
T inpurVer(T mini, T maxi, string message)
{
    T i;
    while (true)
    {
        cout << message;
        if (cin >> i)
        {
            if (i < mini || i > maxi)
                cout << "Please choose between " << mini << " and "<< maxi << " !\n";
            else return i;
        }
        else
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid Please try again\n";
        }
    }
}


vector <Pixel> gaussBlur(vector <Pixel> &pic, int WIDTH, int HEIGHT)
{
    vector <Pixel> newpic (HEIGHT * WIDTH);

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
    vector <double> G;
    separator();
    if (inpurVer <int> (1, 2, "Do you want to use:\n1.Basic Blur\n2.Gaussian Blur\n") == 1)
    {
        separator();
        ans = inpurVer <int> (1, 10, "Choose the size of the mask:\n1.3X3\n2.5X5\n3.7X7\n4.9X9\n5.11X11\n6.13X13\n7.15X15\n8.17X17\n9.19X19\n10.21X21\n");
        for (int i = 0, n = (ans * 2 + 1) * (ans * 2 + 1); i < n; i++)
            G.push_back(1.0);
    }
    else
    {
        separator();
        ans = inpurVer <int> (1, 3, "Choose the size of the mask:\n1.3X3\n2.5X5\n3.7X7\n");
        separator();
        if (inpurVer <int> (1, 2, "Do you want to:\n1.Use a premade mask\n2.Approximate a mask by giving a value for sigma\n") == 1)
            G = gaussianMasks[ans - 1];
        else 
            G = makeGaussMask(ans);
    }

    separator();
    cout << "Making the Blur filter...\n";
    newpic = runMask(G, {}, pic, ans, WIDTH, HEIGHT);    
    cout << "Done the Blur filter!\n";
    return newpic;
}


vector <double> makeGaussMask(int ans)
{
    int k = 2 * ans + 1;
    vector <double> tempmask (k * k);
    double sigma;
    separator();
    cout << "Give a value to sigma: ";
    cin >> sigma;
    sigma *= sigma;
    double coeff = 1/(2 * sigma * pi);
    int index = 0;
    for (int i = -ans; i <= ans; i++)
    {
        for (int j = -ans; j <= ans; j++)
        {
            double power = -((i*i + j*j)/(2 * sigma));
            tempmask[index++] = coeff * exp(power);
        }
    }
    double divBy = 1.0/min_element(tempmask.begin(), tempmask.end())[0];
    vector <double> mask (k * k);
    
    for (int i = 0; i < k * k; i++)
        mask[i] = round(divBy * tempmask[i]);
    return mask;
}


vector <Pixel> sobelOperator(vector <Pixel> &pic, int WIDTH, int HEIGHT)
{
    separator();
    int useGaussOrNot = inpurVer <int> (1, 2, "Do you want to use the Gaussian Blur before applying the edge detector?:\n1.Yes\n2.No\n") - 1;
    if (!useGaussOrNot)
        pic = gaussBlur(pic, WIDTH, HEIGHT);
    separator();
    cout << "Making the Sobel filter...\n";

    vector <double> vertical = {
        -1.0, 0.0, 1.0,
        -2.0, 0.0, 2.0,
        -1.0, 0.0, 1.0
    };
    vector <double> horizontal = {
        -1.0, -2.0, -1.0,
         0.0,  0.0,  0.0,
         1.0,  2.0,  1.0
    };
    vector <Pixel> newpic = runMask(vertical, horizontal, pic, 1, WIDTH, HEIGHT);
    cout << "Done the Sobel filter!\n";
    return newpic;
}


vector <Pixel> canny(vector <Pixel> &pic, int WIDTH, int HEIGHT)
{
    vector <Pixel> newpic (HEIGHT * WIDTH);
    
    separator();
    float The_High_ThreshHold = inpurVer <double> (0.0, 1.0,                 "Thresh hold 1 (High): ");
    float The_Low_ThreshHold  = inpurVer <double> (0.0, The_High_ThreshHold, "Thresh hold 2 (Low): ");
    int Highest_Brightness = 0;

    separator();
    cout << "Making the canny operator...\n";
    for (int i = 0; i < HEIGHT; i++)
    {
        int row = i * WIDTH;
        for (int j = 0; j < WIDTH; j++)
        {
            int idx = row + j;
            double angle = pic[idx].angle;
            int dir;
            if (angle >= 157.5) dir = 0;
            else                dir = round(angle/45.0);
            double pix1 = 0.0, pix2 = 0.0;
            switch (dir)
            {
            case 0:
                if (j - 1 >= 0)                         pix1 = pic[i * WIDTH + j - 1].r;
                if (j + 1 < WIDTH)                      pix2 = pic[i * WIDTH + j + 1].r;                
                break;
            case 1:
                if (j + 1 < WIDTH && i + 1 < HEIGHT)    pix1 = pic[(i + 1) * WIDTH + j + 1].r;
                if (j - 1 >= 0 && i - 1 >= 0)           pix2 = pic[(i - 1) * WIDTH + j - 1].r;
                break;
            case 2:
                if (i - 1 >= 0)                         pix1 = pic[(i - 1) * WIDTH + j].r;
                if (i + 1 < HEIGHT)                     pix2 = pic[(i + 1) * WIDTH + j].r;
                break;
            case 3:
                if (j - 1 >= 0 && i + 1 < HEIGHT)       pix1 = pic[(i + 1) * WIDTH + j - 1].r;
                if (j + 1 < WIDTH && i - 1 >= 0)        pix2 = pic[(i - 1) * WIDTH + j + 1].r;
                break;
            }
            if (pic[idx].r > Highest_Brightness)
                Highest_Brightness = pic[idx].r;

            if (pic[idx].r < pix1 || pic[idx].r < pix2) setAllPixelsTo(newpic[idx], 0.0);
            else                                        setAllPixelsTo(newpic[idx], pic[idx].r);
        }
    }
    int High = round(The_High_ThreshHold * Highest_Brightness);
    int Low = round(The_Low_ThreshHold * Highest_Brightness);
    
    bool changes;
    do {
        changes = false;
        for (int i = 0; i < HEIGHT; i++)
        {
            int row = i * WIDTH;
            for (int j = 0; j < WIDTH; j++)
            {
                int idx = row + j;
                
                if (newpic[idx].r >= High) continue;
                else if (newpic[idx].r < Low)
                {
                    setAllPixelsTo(newpic[idx], 0.0);
                    continue;
                }

                for (int ni : {-1, 0, 1})
                {
                    bool connected = false;
                    for (int nj : {-1, 0, 1})
                    {
                        if (ni == 0 && nj == 0) continue;
                        int x = j + nj;
                        int y = i + ni;
                        if (isInScopeOfImage(x, y))
                        {
                            if (newpic[y * WIDTH + x].r >= High)
                            {
                                setAllPixelsTo(newpic[idx], High);
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
    } while (changes);
    cout << "Done making the canny Operator!";
    return newpic;
}

vector <Pixel> runMask(vector <double> mask, vector <double> SobelMask, vector <Pixel> image, int kernelSize, int WIDTH, int HEIGHT)
{
    vector <Pixel> newpic (WIDTH * HEIGHT);
    bool sobel = !SobelMask.empty();
    for (int i = 0; i < HEIGHT; i++)
    {
        int row = i * WIDTH;
        for (int j = 0; j < WIDTH; j++)
        {
            int idx = row + j;
            double r, g, b, nc, Gx, Gy;
            r = g = b = nc = Gx = Gy = 0.0;
            for (int ti = -kernelSize; ti <= kernelSize; ti++)
            {
                int y = i + ti;
                int maskRow = y * WIDTH;
                for (int tj = -kernelSize; tj <= kernelSize; tj++)
                {
                    int x = j + tj;
                    if (isInScopeOfImage(x, y))
                    {
                        int kernelTurn = (ti + kernelSize) * (kernelSize * 2 + 1) + (tj + kernelSize);
                        int theImageInTheMaskIdx = maskRow + x;
                        if (sobel)
                        {
                            double gray = (image[theImageInTheMaskIdx].r + image[theImageInTheMaskIdx].g + image[theImageInTheMaskIdx].b)/3.0;
                            Gx +=  gray * mask[kernelTurn];
                            Gy +=  gray * SobelMask[kernelTurn];
                        }
                        else
                        {
                            double tr = mask[kernelTurn];
                            r  += image[theImageInTheMaskIdx].r * tr;
                            g  += image[theImageInTheMaskIdx].g * tr;
                            b  += image[theImageInTheMaskIdx].b * tr;
                            nc += tr;
                        }
                    }
                }
            }
            if (sobel)
            {
                double val = sqrt(Gx * Gx + Gy * Gy);
                if (val > 255)
                    val = 255;
                setAllPixelsTo(newpic[idx], val);
                double angle = atan2(Gy, Gx) * transformFromRadToDeg;
                if (angle < 0)
                    angle += 180.0;
                newpic[idx].angle = angle;
            }
            else
            {
                newpic[idx].r = r/nc;
                newpic[idx].g = g/nc;
                newpic[idx].b = b/nc;
            }
        }
    }
    return newpic;
}


vector <Pixel> loadPic(ifstream &f)
{
    separator();
    cout << "Loading the image...\n";
    int temph;
    
    f.seekg(18, fstream::beg);
    f.read((char*)&width, 4);
    f.read((char*)&temph, 4);
    height = abs(temph);
    bool bu = (temph > 0);
    f.seekg(10, fstream::beg);   
    uint32_t offset;
    f.read((char*)& offset, 4);
    f.seekg(offset, fstream::beg);   
    
    vector <Pixel> pic (width * height);
    int padding = (4 - ((width * 3) % 4)) % 4;
    for (int i = 0; i < height; i++)
    {
        int y = (bu) ? i : height - 1 - i;
        int row = y * width;
        for (int j = 0; j < width; j++)
        {
            int idx = row + j;

            pic[idx].b = f.get();
            pic[idx].g = f.get();
            pic[idx].r = f.get();
        }
        f.ignore(padding);
    }
    cout << "File was loaded successfully\n";
    f.close();
    return pic;
}


void makeFile(vector <Pixel> &pic, int WIDTH, int HEIGHT)
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
    for (int i = 0; i < HEIGHT; i++)
    {
        int row = i * WIDTH;
        for (int x = 0; x < WIDTH; x++)
        {
            int idx = row + x;
            f.put((uint8_t) pic[idx].b);
            f.put((uint8_t) pic[idx].g);
            f.put((uint8_t) pic[idx].r);
        }
        f.write((char*)pad, padding);
    }
    f.close();
    cout << "Done making the file!\n";
}
