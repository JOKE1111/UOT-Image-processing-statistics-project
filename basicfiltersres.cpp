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
    uint8_t r, g, b;
};

int WIDTH, HEIGHT;
bool bu;



string getFileName();
vector <Pixel> blur(ifstream &f);
void makeFile(vector <Pixel> &pic);
vector <Pixel> loadPic(ifstream &f);
int intVer(int mini, int maxi, string message);
vector <Pixel> gaussBlur(ifstream &f);
vector <Pixel> sobelOperator(ifstream &f);
vector <int> makeGaussMask(int ans);




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
    int choice = intVer(1, 3, "What do you want to do to the image?: \n1.Normal Blur\n2.Gaussian Blur\n3.Edge Detection\n");

    switch (choice)
    {
        case 1:
            pic = blur(f);
            break;
        case 2:
            pic = gaussBlur(f);
            break;
        case 3:
            pic = sobelOperator(f);
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
            cout << "invalid file name please try again";
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


vector <Pixel> blur(ifstream &f)
{
    vector <Pixel> pic = loadPic(f);
    vector <Pixel> newpic (HEIGHT * WIDTH);
    int mag = intVer(1, 10, "Choose the size of the mask:\n1.3X3\n2.5X5\n3.7X7\n4.9X9\n5.11X11\n6.13X13\n7.15X15\n8.17X17\n9.19X19\n10.21X21\n");
    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            Pixel midPix = {};
            int rsum, gsum, bsum, nc;
            rsum = gsum = bsum = nc = 0;
            for (int ti = -mag; ti <= mag; ti++)
            {
                for (int tj = -mag; tj <= mag; tj++)
                {
                    int x = i + ti;
                    int y = j + tj;
                    if (x < WIDTH && x >= 0 && y < HEIGHT && y >= 0)
                    {
                        rsum += pic[y * WIDTH + x].r;
                        gsum += pic[y * WIDTH + x].g;
                        bsum += pic[y * WIDTH + x].b;
                        nc++;
                    }
                }
            }
            midPix.r = rsum/nc;
            midPix.g = gsum/nc;
            midPix.b = bsum/nc;
            newpic[j * WIDTH + i] = midPix;
        }
    }    
    return newpic;
    
    /*  from here you should try to implement the filters, 
        and if you have connection i'd advise reading more about cin safety 
        Eyad/
    */
}


void makeFile(vector <Pixel> &pic)
{
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
        int y = bu ? i : (HEIGHT - 1 - i);
        for (int x = 0; x < WIDTH; x++)
        {
            f.put(pic[y * WIDTH + x].b);
            f.put(pic[y * WIDTH + x].g);
            f.put(pic[y * WIDTH + x].r);
        }
        f.write((char*)pad, padding);
    }
    f.close();
    return;
}


vector <Pixel> loadPic(ifstream &f)
{
    cout << "Loading the image...\n";
    int temph;
    
    f.seekg(18, fstream::beg);
    f.read((char*)&WIDTH, 4);
    f.read((char*)&temph, 4);
    HEIGHT = abs(temph);
    bu = (temph > 0);
    f.seekg(10, fstream::beg);   
    uint32_t offset;
    f.read((char*)& offset, 4);
    f.seekg(offset, fstream::beg);   
    
    vector <Pixel> pic (WIDTH * HEIGHT);
    int padding = (4 - ((WIDTH * 3) % 4)) % 4;
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            Pixel pixel;

            pixel.b = f.get();
            pixel.g = f.get();
            pixel.r = f.get();
            pic[i * WIDTH + j] = pixel;
        }
        f.ignore(padding);
    }
    cout << "file was loaded successfully\n";
    f.close();
    return pic;
}


vector <Pixel> gaussBlur(ifstream &f)
{
    vector <Pixel> pic = loadPic(f);
    vector <Pixel> newpic (HEIGHT * WIDTH);

    vector <int> G3 = {
    1, 2, 1,
    2, 4, 2,
    1, 2, 1};
    vector <int> G5 = {
    1,  4,  7,  4,  1,
    4, 16, 26, 16,  4,
    7, 26, 41, 26,  7,
    4, 16, 26, 16,  4,
    1,  4,  7,  4,  1};
    vector <int> G7 = {
    0,  0,  1,  2,  1,  0,  0,
    0,  3, 13, 22, 13,  3,  0,
    1, 13, 59, 97, 59, 13,  1,
    2, 22, 97,159, 97, 22,  2,
    1, 13, 59, 97, 59, 13,  1,
    0,  3, 13, 22, 13,  3,  0,
    0,  0,  1,  2,  1,  0,  0};

    int ans = intVer(1, 3, "Choose the size of the mask:\n1.3X3\n2.5X5\n3.7X7\n");
    
    vector <int> G;
    int approx = intVer(1, 2, "Do you want to:\n1.Use a premade mask\n2.Approximate a mask by giving a value for sigma\n");
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
    cout << "Making the filter...\n";
    for (int i = 0; i < HEIGHT; i++)
    {
        int yi = (bu) ? i : HEIGHT - 1 - i;
        for (int j = 0; j < WIDTH; j++)
        {
            Pixel p;
            int r, g, b, nc;
            r = g = b = nc = 0;
            for (int ti = -ans; ti <= ans; ti++)
            {
                for (int tj = -ans; tj <= ans; tj++)
                {
                    int y = ti + yi;
                    int x = tj + j;
                    if (y < HEIGHT && y >= 0 && x < WIDTH && x >= 0)
                    {
                        int tr = G[(ti + ans) * (ans * 2 + 1) + (tj + ans)];
                        int index = y * WIDTH + x;

                        r += pic[index].r * tr;
                        g += pic[index].g * tr;
                        b += pic[index].b * tr;
                        nc += tr;
                    }
                }
            }
            p.r = r/nc;
            p.g = g/nc;
            p.b = b/nc;
            newpic[yi * WIDTH + j] = p;
        }
    }
    cout << "Done the filter!\n";
    return newpic;
}


vector <int> makeGaussMask(int ans)
{
    int k = 2 * ans + 1;
    vector <double> tempmask (k * k);
    double pi = 3.14159265358979323846264338327950288419716939937510;
    double sigma;
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
    vector <int> mask (k * k);
    
    for (int i = 0; i < k * k; i++)
        mask[i] = round(divBy * tempmask[i]);
    return mask;
}


vector <Pixel> sobelOperator(ifstream &f)
{
    int useGaussOrNot = intVer(1, 2, "Do you want to use the Gaussian Blur before applying the edge detector?:\n1.Yes\n2.No\n") - 1;
    vector <Pixel> pic;
    if (!useGaussOrNot)
        pic = gaussBlur(f);
    else
        pic = loadPic(f);
    vector <Pixel> newpic (HEIGHT * WIDTH);

    cout << "Making the filter...\n";

    static const int vertical[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    static const int horizontal[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1}
    };

    for (int i = 0; i < HEIGHT; i++)
    {
        int yi = (bu) ? i : HEIGHT - 1 - i;
        for (int j = 0; j < WIDTH; j++)
        {
            Pixel p;
            int avgV = 0;
            int avgH = 0;
            for (int ti : {-1, 0, 1})
            {
                for (int tj : {-1, 0, 1})
                {
                    int y = ti + yi;
                    int x = tj + j;
                    if (y < HEIGHT && y >= 0 && x < WIDTH && x >= 0)
                    {
                        int trV = vertical[ti + 1][tj + 1];
                        int trH = horizontal[ti + 1][tj + 1];
                        int index = y * WIDTH + x;
                        int gray = (pic[index].r + pic[index].g + pic[index].b) / 3;
                        avgV += trV * gray;
                        avgH += trH * gray;
                    }
                }
            }
            if (abs(avgV) > 255)
                avgV = 255;
            if (abs(avgH) > 255)
                avgH = 255;
            int val = sqrt(avgV * avgV + avgH * avgH);
            if (val > 255)
                val = 255;
            int index = yi * WIDTH + j;
            newpic[index].r = newpic[index].g = newpic[index].b = val;
        }
    }
    cout << "Done the filter!\n";
    return newpic;
}