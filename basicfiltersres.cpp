#include <iostream>
#include <fstream>
#include <map>
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

int WIDTH, HEIGHT;
const double pi = 3.14159265358979323846264338327950288419716939937510;
const double transformFromRadToDeg = 180.0 / pi;

string getFileName();
int intVer(int mini, int maxi, string message);
float floatVer(float maxi, float mini, string message);
void makeFile(vector <Pixel> &pic);
vector <double> makeGaussMask(int ans);
vector <Pixel> loadPic(ifstream &f);
vector <Pixel> gaussBlur(ifstream &f);
vector <Pixel> sobelOperator(ifstream &f);
vector <Pixel> canny(ifstream &f);



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

    vector <Pixel> pic;

    int choice = intVer(1, 3, "What do you want to do to the image?:\n1.Blur\n2.Sobel Edge Detection\n3.Canny Edge Detection\n");
    switch (choice)
    {
        case 1:
            pic = gaussBlur(f);
            break;
        case 2:
            pic = sobelOperator(f);
            break;
        case 3:
            pic = canny(f);
            break;
    }
    makeFile(pic);
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


int intVer(int mini, int maxi, string message)
{
    int i;
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


void makeFile(vector <Pixel> &pic)
{
    cout << "\n***************\nMaking the file...\n";
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
        for (int x = 0; x < WIDTH; x++)
        {
            f.put((uint8_t) pic[i * WIDTH + x].b);
            f.put((uint8_t) pic[i * WIDTH + x].g);
            f.put((uint8_t) pic[i * WIDTH + x].r);
        }
        f.write((char*)pad, padding);
    }
    f.close();
    cout << "Done making the file!\n";
    return;
}


vector <Pixel> loadPic(ifstream &f)
{
    cout << "\n***************\nLoading the image...\n";
    int temph;
    
    f.seekg(18, fstream::beg);
    f.read((char*)&WIDTH, 4);
    f.read((char*)&temph, 4);
    HEIGHT = abs(temph);
    bool bu = (temph > 0);
    f.seekg(10, fstream::beg);   
    uint32_t offset;
    f.read((char*)& offset, 4);
    f.seekg(offset, fstream::beg);   
    
    vector <Pixel> pic (WIDTH * HEIGHT);
    int padding = (4 - ((WIDTH * 3) % 4)) % 4;
    for (int i = 0; i < HEIGHT; i++)
    {
        int y = (bu) ? i : HEIGHT - 1 - i;
        for (int j = 0; j < WIDTH; j++)
        {
            Pixel pixel;

            pixel.b = f.get();
            pixel.g = f.get();
            pixel.r = f.get();
            pic[y * WIDTH + j] = pixel;
        }
        f.ignore(padding);
    }
    cout << "File was loaded successfully\n";
    f.close();
    return pic;
}


vector <Pixel> gaussBlur(ifstream &f)
{
    vector <Pixel> pic = loadPic(f);
    vector <Pixel> newpic (HEIGHT * WIDTH);

    vector <double> G3 = {
    1.0, 2.0, 1.0,
    2.0, 4.0, 2.0,
    1.0, 2.0, 1.0};
    vector <double> G5 = {
    1.0,  4.0,  7.0,  4.0,  1.0,
    4.0, 16.0, 26.0, 16.0,  4.0,
    7.0, 26.0, 41.0, 26.0,  7.0,
    4.0, 16.0, 26.0, 16.0,  4.0,
    1.0,  4.0,  7.0,  4.0,  1.0};
    vector <double> G7 = {
    0.0,  0.0,  1.0,  2.0,  1.0,  0.0,  0.0,
    0.0,  3.0, 13.0, 22.0, 13.0,  3.0,  0.0,
    1.0, 13.0, 59.0, 97.0, 59.0, 13.0,  1.0,
    2.0, 22.0, 97.0,159.0, 97.0, 22.0,  2.0,
    1.0, 13.0, 59.0, 97.0, 59.0, 13.0,  1.0,
    0.0,  3.0, 13.0, 22.0, 13.0,  3.0,  0.0,
    0.0,  0.0,  1.0,  2.0,  1.0,  0.0,  0.0};
    int ans;
    vector <double> G;
    int blurType = intVer(1, 2, "\n***************\nDo you want to use:\n1.Basic Blur\n2.Gaussian Blur\n");
    if (blurType == 1)
    {
        ans = intVer(1, 10, "\n***************\nChoose the size of the mask:\n1.3X3\n2.5X5\n3.7X7\n4.9X9\n5.11X11\n6.13X13\n7.15X15\n8.17X17\n9.19X19\n10.21X21\n");
        for (int i = 0, n = (ans * 2 + 1) * (ans * 2 + 1); i < n; i++)
            G.push_back(1.0);
    }
    else
    {
        ans        = intVer(1, 3, "\n***************\nChoose the size of the mask:\n1.3X3\n2.5X5\n3.7X7\n");
        int approx = intVer(1, 2, "\n***************\nDo you want to:\n1.Use a premade mask\n2.Approximate a mask by giving a value for sigma\n");
        
        if (approx == 1)
        {
            switch (ans)
            {
                case 1:
                    G = G3;
                    break;
                case 2:
                    G = G5;
                    break;
                case 3:
                    G = G7;
                    break;
            }
        }
        else 
            G = makeGaussMask(ans);
    }

    cout << "\n***************\nMaking the Blur filter...\n";
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            Pixel p;
            double r, g, b, nc;
            r = g = b = nc = 0.0;
            for (int ti = -ans; ti <= ans; ti++)
            {
                for (int tj = -ans; tj <= ans; tj++)
                {
                    int y = ti + i;
                    int x = tj + j;
                    if (y < HEIGHT && y >= 0 && x < WIDTH && x >= 0)
                    {
                        double tr = G[(ti + ans) * (ans * 2 + 1) + (tj + ans)];
                        int index = y * WIDTH + x;

                        r  += pic[index].r * tr;
                        g  += pic[index].g * tr;
                        b  += pic[index].b * tr;
                        nc += tr;
                    }
                }
            }
            p.r = r/nc;
            p.g = g/nc;
            p.b = b/nc;
            newpic[i * WIDTH + j] = p;
        }
    }
    cout << "\n***************\nDone the Gaussian filter!\n";
    return newpic;
}


vector <double> makeGaussMask(int ans)
{
    int k = 2 * ans + 1;
    vector <double> tempmask (k * k);
    double sigma;
    cout << "\n***************\nGive a value to sigma: ";
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


vector <Pixel> sobelOperator(ifstream &f)
{
    int useGaussOrNot = intVer(1, 2, "\n***************\nDo you want to use the Gaussian Blur before applying the edge detector?:\n1.Yes\n2.No\n") - 1;
    vector <Pixel> pic;
    if (!useGaussOrNot)
        pic = gaussBlur(f);
    else
        pic = loadPic(f);
    vector <Pixel> newpic (HEIGHT * WIDTH);

    cout << "\n***************\nMaking the Sobel filter...\n";

    static const double vertical[3][3] = {
        {-1.0, 0.0, 1.0},
        {-2.0, 0.0, 2.0},
        {-1.0, 0.0, 1.0}
    };
    static const double horizontal[3][3] = {
        {-1.0, -2.0, -1.0},
        { 0.0,  0.0,  0.0},
        { 1.0,  2.0,  1.0}
    };

    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            Pixel p;
            double Gx = 0.0;
            double Gy = 0.0;
            for (int ti : {-1, 0, 1})
            {
                for (int tj : {-1, 0, 1})
                {
                    int y = ti + i;
                    int x = tj + j;
                    if (y < HEIGHT && y >= 0 && x < WIDTH && x >= 0)
                    {
                        double trV = vertical[ti + 1][tj + 1];
                        double trH = horizontal[ti + 1][tj + 1];
                        int index = y * WIDTH + x;
                        double gray = (pic[index].r + pic[index].g + pic[index].b) / 3;
                        Gx += trV * gray;
                        Gy += trH * gray;
                    }
                }
            }
            double val = sqrt(Gx * Gx + Gy * Gy);
            if (val > 255)
                val = 255;
            int index = i * WIDTH + j;
            newpic[index].r = newpic[index].g = newpic[index].b = val;
            double angle = atan2(Gy, Gx) * transformFromRadToDeg;
            if (angle < 0)
                angle += 180.0;
            newpic[index].angle = angle;
        }
    }
    cout << "Done the Sobel filter!\n";
    return newpic;
}

float floatVer(float mini, float maxi, string message)
{
    float i;
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




vector <Pixel> canny(ifstream &f)
{
    vector <Pixel> pic = sobelOperator(f);
    vector <Pixel> newpic (HEIGHT * WIDTH);

    float The_High_ThreshHold = floatVer(0.0, 1.0,                 "\n***************\nThresh hold 1 (High): ");
    float The_Low_ThreshHold  = floatVer(0.0, The_High_ThreshHold, "Thresh hold 2 (Low): ");
    int Highest_Brightness = 0;

    cout << "\n***************\nMaking the canny operator...\n";
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            int idx = i * WIDTH + j;
            int dir;
            double angle = pic[idx].angle;
            if ((angle >= 0) && (angle < 22.5) || (angle >= 157.5 && angle <= 180))
                dir = 0;
            else if (angle >= 22.5 && angle < 67.5)
                dir = 45;
            else if (angle >= 67.5 && angle < 112.5)
                dir = 90;
            else
                dir = 135;
            double pix1 = 0.0, pix2 = 0.0;
            switch (dir)
            {
            case 0:
                if (j - 1 >= 0)
                    pix1 = pic[i * WIDTH + j - 1].r;
                if (j + 1 < WIDTH)
                    pix2 = pic[i * WIDTH + j + 1].r;                
                break;
            case 45:
                if (j + 1 < WIDTH && i + 1 < HEIGHT)
                    pix1 = pic[(i + 1) * WIDTH + j + 1].r;
                if (j - 1 >= 0 && i - 1 >= 0)
                    pix2 = pic[(i - 1) * WIDTH + j - 1].r;
                break;
            case 90:
                if (i - 1 >= 0)
                    pix1 = pic[(i - 1) * WIDTH + j].r;
                if (i + 1 < HEIGHT)
                    pix2 = pic[(i + 1) * WIDTH + j].r;
                break;
            case 135:
                if (j - 1 >= 0 && i + 1 < HEIGHT)
                    pix1 = pic[(i + 1) * WIDTH + j - 1].r;
                if (j + 1 < WIDTH && i - 1 >= 0)
                    pix2 = pic[(i - 1) * WIDTH + j + 1].r;
                break;
            }
            if (pic[idx].r > Highest_Brightness)
                Highest_Brightness = pic[idx].r;
            if (pic[idx].r < pix1 || pic[idx].r < pix2)
                newpic[idx].r = newpic[idx].g = newpic[idx].b = 0;
            else
                newpic[idx].r = newpic[idx].g = newpic[idx].b = pic[idx].r;
        }
    }
    bool changes;
    int High = round(The_High_ThreshHold * Highest_Brightness);
    int Low = round(The_Low_ThreshHold * Highest_Brightness);
    
    do {
        changes = false;
        for (int i = 0; i < HEIGHT; i++)
        {
            for (int j = 0; j < WIDTH; j++)
            {
                int idx = i * WIDTH + j;
                
                if (newpic[idx].r >= High) continue;
                else if (newpic[idx].r < Low)
                {
                    newpic[idx].r = newpic[idx].g = newpic[idx].b = 0;
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
                        if (y >= 0 && y < HEIGHT && x >= 0 && x < WIDTH)
                        {
                            if (newpic[y * WIDTH + x].r >= High)
                            {
                                newpic[idx].r = newpic[idx].g = newpic[idx].b = High;
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