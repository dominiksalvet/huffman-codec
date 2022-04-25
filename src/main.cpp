// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-20
// filename: main.cpp
// summary: Simple Huffman codec with multiple options.
//          Assumes 8-bit grayscale format of input data.

#include <iostream>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <climits>

#include "huff.hpp"

using namespace std;

const string HELP_MESSAGE =
"USAGE:\n"
"  huff_codec [-cma] -i IFILE [-o OFILE] [-w WIDTH]\n"
"  huff_codec -d -i IFILE [-o OFILE] | -h\n"
"\n"
"OPTION:\n"
"  -c/-d  perform compression/decompression\n"
"  -m     use model for image preprocessing\n"
"  -a     use adaptive scanning\n"
"  -i     input file path\n"
"  -o     output file path (default: b.out)\n"
"  -w     used image width (default: 512)\n"
"  -h     show this help\n";


// redirect input to stderr, but also print a help hint
void cerrh(const char *s) {
    cerr << s << "try 'huff_codec -h' for more information\n";
}

// transform pixel values to their differences (in situ)
// this algorithm utilizes the properties of two's complement (underflow)
void applyDiffModel(vector<uint8_t> &vec)
{
    uint8_t prevVal = 0;
    for (size_t i = 0; i < vec.size(); i++)
    {
        uint8_t curVal = vec[i];
        vec[i] = (curVal - prevVal); // truncated result of underflow
        prevVal = curVal;
    }
}

// revert the differential model (in situ)
// also uses the two's complement properties (overflow)
void revertDiffModel(vector<uint8_t> &vec)
{
    uint8_t prevVal = 0;
    for (size_t i = 0; i < vec.size(); i++)
    {
        vec[i] += prevVal; // may overflow (truncated)
        prevVal = vec[i];
    }
}

// compress image based on several given options
// data is preceded with header: <64b-pixel-count><1b-diff-model>
vector<uint8_t> compress(ifstream& ifs, bool useModel, bool adaptScan, uint64_t imgWidth)
{
    // load input file to internal representation vector
    vector<uint8_t> inData;
    int c;
    while ((c = ifs.get()) != EOF) {
        inData.push_back(c);
    }
    ifs.close();

    // check valid image resolution
    if ((inData.size() % imgWidth) != 0)
    {
        cerr << "ERROR: invalid resolution of input image detected\n";
        exit(6);
    }

    // apply differential model
    if (useModel) {
        applyDiffModel(inData);
    }

    // create the Huffman FGK tree
    HuffTree huffTree; // call default contructor

    vector<bool> outBits;
    // header part <1b-diff-model> to indicate whether diff model was used
    outBits.push_back(useModel);

    // encode input data to bit vector
    for (uint8_t symbol : inData)
    {
        vector<bool> symbolCode = huffTree.encode(symbol);
        // append symbol code to the existing code
        outBits.insert(outBits.end(), symbolCode.begin(), symbolCode.end());
        huffTree.update(symbol);
    }
    // add remaining bits so their final count is divisible by bits in symbol
    while (outBits.size() % CHAR_BIT != 0) {
        outBits.push_back(0); // value does not matter
    }

    vector<uint8_t> outData;
    // header part <64b-pixel-count> to indicated total number of encoded pixels
    for (unsigned int i = 0; i < sizeof(uint64_t); i++) {
        outData.push_back(inData.size() >> (CHAR_BIT * i));
    }

    // convert bit vector to byte array
    for (size_t i = 0; i < outBits.size(); i += 8) {
        uint8_t curByte = 0;
        for (int j = 0; j < CHAR_BIT; j++) {
            curByte = (curByte << 1) | outBits[i + j];
        }
        outData.push_back(curByte);
    }

    return outData;
}

// decompress image of the given input stream (based on its header)
// it respects the header as described in the compress function comments
vector<uint8_t> decompress(ifstream &ifs)
{
    // read total pixel count
    uint64_t pixelCount;
    ifs.read((char *)&pixelCount, sizeof(uint64_t));
    if (!ifs)
    {
        cerr << "ERROR: invalid compressed file header\n";
        exit(8);
    }

    // load input file to internal representation vector
    queue<bool> inData;
    int c;
    while ((c = ifs.get()) != EOF) {
        for (int i = 0; i < CHAR_BIT; i++) {
            inData.push((c >> (CHAR_BIT - i - 1) & 0x01));
        }
    }
    ifs.close();
    // check where diff model was used
    bool modelUsed = inData.front(); inData.pop(); // remove from queue

    // create the Huffman FGK tree
    HuffTree huffTree;

    vector<uint8_t> outData;
    for (uint64_t i = 0; i < pixelCount; i++)
    {
        int decResult = huffTree.decode(&inData);
        if (decResult == -1)
        {
            cerr << "ERROR: invalid compressed file contents\n";
            exit(9);
        }
        uint8_t symbol = decResult;
    
        huffTree.update(symbol);
        outData.push_back(symbol);
    }

    if (modelUsed) {
        revertDiffModel(outData);
    }

    return outData;
}

// write final data from vector to given output file path
void writeOutData(const vector<uint8_t> &vec, const string &filePath)
{
    ofstream ofs(filePath, ios::out | ios::binary); // output file stream
    if (ofs.fail())
    {
        cerr << "ERROR: cannot write to " << filePath << " output file\n";
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
    string ofp = "b.out"; // default path
    uint64_t imgWidth = 512; // default value

    // argument processing
    // options are designed to be more tolerant (yet they meet the assignment)
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
        case 'w': imgWidth = stoull(optarg); break;
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
    if (useCompr && imgWidth == 0)
    {
        cerrh("ERROR: invalid image width\n");
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
