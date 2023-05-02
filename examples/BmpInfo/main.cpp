#include "EasyBMP.h"
#include <sstream>
#include "cxxopts.hpp"
#include <filesystem>
// #include <iostream>
#include <fstream>

using namespace std;
namespace fs = std::filesystem;

void print_stats(const string& input);
void print_stats(const string& input) {
    BMP img;
    RGBApixel pixel;
    img.ReadFromFile(input);
    for (int i=0; i<abs(img.TellHeight()); i++) {
        pixel = img(0,i);
        cout << "Pixel(0," << dec << i << ")"
        << " Red="   << hex << (int) pixel.Red
        << " Green=" << hex << (int) pixel.Green
        << " Blue="  << hex << (int) pixel.Blue
        << dec << endl;
    }
}


void copy_bmp(const string& input, const string& output);
void copy_bmp(const string& input, const string& output) {
    BMP img;
    img.ReadFromFile(input);
    img.WriteToFile(output);
}


void dump_pixel_data(const string& input, const string& output, bool invert);
void dump_pixel_data(const string& input, const string& output, bool invert) {
    BMP img;
    img.ReadFromFile(input);

    if (img.TellBitDepth() != 8) {
        cerr << "Can only dump 8-bit BMP files currently" << endl;
        exit(1);
    }

    int width = img.AbsWidth();
    int height = img.AbsHeight();

    vector<uint8_t> pixel_data;
    if (invert) {
        for (int row=height-1; row>=0; row--) {
            for (int col=0; col<width; col++) {
                RGBApixel pixel = img(col,row);
                pixel_data.push_back(pixel.Red);
            }
        }
    } else {
        for (int row=0; row<height; row++) {
            for (int col=0; col<width; col++) {
                RGBApixel pixel = img(col,row);
                pixel_data.push_back(pixel.Red);
            }
        }
    }

    ofstream of(output, ios::binary);
    of.write((char *)&pixel_data[0], pixel_data.size()*sizeof(uint8_t));
    of.close();
}


void print_first_row(const string& input, int row);
void print_first_row(const string& input, int row) {
    BMP img;
    img.ReadFromFile(input);

    int width = img.AbsWidth();

    for (int i=0; i<width; i++) {
        RGBApixel pixel = img(i, row);
        cout << hex << (int) pixel.Red << " ";
    }
    cout << endl;
}


int main (int argc, char *argv[]) {

    cxxopts::Options options(argv[0], "A simple BMP multitool");

    options.add_options()
    ("h,help", "Display the help text")
    ("i,info", "Print stats on the input bitmap")
    ("c,copy", "Copy a BMP")
    ("d,dump_pixel_data", "Dump a hexfile containing the pixel buffer")
    ("p,print", "Print a row of pixels", cxxopts::value<int>())
    ("invert", "flip the image vertically")
    ("f,force", "Overwrite any output file if it already exists")
    ("input", "The input bitfile", cxxopts::value<string>())
    ("output", "The input bitfile", cxxopts::value<string>())
    ;

    options.parse_positional({"input", "output"});

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        cout << options.help() << endl;
        exit(0);
    }

    if (not result.count("input")) {
        cerr << "error: no input file supplied" << endl;
        exit(1);
    }
    auto input = result["input"].as<string>();

    if (result.count("copy") or result.count("dump_pixel_data")) {
        if (not result.count("output")) {
            cerr << "error: no output file supplied" << endl;
            exit(1);
        }
        auto output = result["output"].as<string>();
        if (fs::exists(output) and not result.count("force")) {
            cerr << "error: output file " << output 
                 << " exists; use --force  to overwrite" << endl;
            exit(1);
        }
    }
        
    if (result.count("info")) {
        DisplayBitmapInfo(input);
    }

    if (result.count("copy")) {
        auto output = result["output"].as<string>();
        copy_bmp(input, output);
    }

    if (result.count("dump_pixel_data")) {
        auto output = result["output"].as<string>();
        auto invert = result["invert"].as<bool>();
        dump_pixel_data(input, output, invert);
    }

    if (result.count("print")) {
        auto row = result["print"].as<int>();
        print_first_row(input, row);
    }
    
    return 0;
}