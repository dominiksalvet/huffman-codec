// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-20
// filename: main.cpp
// summary: Simple Huffman codec with multiple options.
//          Assumes 8-bit grayscale format of input data.

#include <iostream>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <vector>

#include "huff.hpp"

using namespace std;

const string HELP_MESSAGE =
"USAGE:\n"
"  huff_codec [-cma] -w WIDTH -i IFILE [-o OFILE]\n"
"  huff_codec -d -i IFILE [-o OFILE] | -h\n"
"\n"
"OPTION:\n"
"  -c/-d  perform compression/decompression\n"
"  -m     use model for image preprocessing\n"
"  -a     use adaptive scanning\n"
"  -i/-o  input/output file path\n"
"  -w     used image width\n"
"  -h     show this help\n";


// redirect input to stderr, but also print a help hint
void cerrh(const char *s)
{
    cerr << s << "try 'huff_codec -h' for more information\n";
}

// transform pixel values to their differences (in situ)
// to keep unsigned values, they are normalized to 0-510 range
void applyDiffModel(vector<uint16_t> &vec)
{
    int16_t prevVal = 0;
    for (size_t i = 0; i < vec.size(); i++)
    {
        int16_t curVal = vec[i];
        vec[i] = (curVal - prevVal) + 255; // normalization
        prevVal = curVal;
    }
}

// TODO: compress image based on several given options
vector<uint8_t> compress(
    ifstream& ifs,
    bool useModel,
    bool adaptScan,
    int32_t imgWidth)
{
    // load input file to internal representation vector
    vector<uint16_t> inData; // 16 bits for simpler later processing
    int c;
    while ((c = ifs.get()) != EOF) {
        inData.push_back(c); // implicit conversion
    }
    ifs.close();

    // derive image height
    if ((inData.size() % imgWidth) != 0)
    {
        cerr << "ERROR: invalid resolution of input image detected\n";
        exit(6);
    }

    if (useModel) {
        applyDiffModel(inData);
    }

    vector<uint8_t> a;
    for (uint16_t item : inData) {
        a.push_back(item);
    }
    return a;
}

// TODO: decompress image of the given input stream
vector<uint8_t> decompress(ifstream &ifs)
{
    vector<uint8_t> inData;
    int c;
    while ((c = ifs.get()) != EOF) {
        inData.push_back(c);
    }
    ifs.close();

    return inData;
}

// write final data from vector to given output file path
void writeOutData(vector<uint8_t> &vec, const string &ofp)
{
    ofstream ofs(ofp, ios::out | ios::binary); // output file stream
    if (ofs.fail())
    {
        cerr << "ERROR: cannot write to " << ofp << " output file\n";
        exit(7);
    }

    ofs.write((char *) vec.data(), vec.size());
    ofs.close();
}


// entry point of program
int main(int argc, char *argv[])
{
    // default setup
    bool useCompr = true;
    bool useModel = false;
    bool adaptScan = false;

    string ifp; // input file path (empty by default constructor)
    string ofp = "a.out"; // default path
    int32_t imgWidth = 0;

    // argument processing
    int opt;
    while ((opt = getopt(argc, argv, ":cdmai:o:w:h")) != -1)
    {
        switch (opt)
        {
        case 'c': useCompr = true; break;
        case 'd': useCompr = false; break;
        case 'm': useModel = true; break;
        case 'a': adaptScan = true; break;
        case 'i': ifp = optarg; break;
        case 'o': ofp = optarg; break;
        case 'w': imgWidth = stoi(optarg); break;
        case 'h':
            cout << HELP_MESSAGE;
            return 0; break;
        case ':':
            cerrh("ERROR: missing additional argument\n");
            return 1; break;
        case '?':
            cerrh("ERROR: unrecognized option used\n");
            return 2; break;
        }
    }

    // mandatory arguments check
    if (ifp.empty())
    {
        cerrh("ERROR: no input file path provided\n");
        return 3;
    }
    if (useCompr && imgWidth <= 0)
    {
        cerrh("ERROR: invalid or missing image width\n");
        return 4;
    }

    // reading input file
    ifstream ifs(ifp, ios::in | ios::binary); // input file stream
    if (ifs.fail())
    {
        cerr << "ERROR: given input file does not exist\n";
        return 5;
    }

    // perform required operation
    vector<uint8_t> outData; // alway array of bytes
    if (useCompr) {
        outData = compress(ifs, useModel, adaptScan, imgWidth);
    } else {
        outData = decompress(ifs);
    }
    
    writeOutData(outData, ofp);
}
