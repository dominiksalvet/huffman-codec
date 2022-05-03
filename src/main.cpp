// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-20
// filename: main.cpp
// summary: Adaptive Huffman codec with multiple options. It works with any file,
//          having extra features for 2D data (e.g., 8-bit grayscale images).

#include <iostream>
#include <unistd.h>
#include <fstream>
#include <climits>
#include <vector>
#include <deque>
#include <cstdint>

#include "transform.hpp"
#include "headers.hpp"

using namespace std;

const string HELP_MESSAGE =
"USAGE:\n"
"  huff_codec [-cma] [-w WIDTH] -i IFILE [-o OFILE]\n"
"  huff_codec -d -i IFILE [-o OFILE] | -h\n"
"\n"
"OPTION:\n"
"  -c/-d  perform compression/decompression\n"
"  -m     use differential model for preprocessing\n"
"  -a     use adaptive block RLE (default: RLE)\n"
"  -w     width of 2D data (default: 512, disable: 1)\n"
"  -i     input file path\n"
"  -o     output file path (default: b.out)\n"
"  -h     show this help\n";


// compress data based on several given options
vector<uint8_t> huffCompress(
    ifstream& ifs,
    bool useDiffModel,
    bool useAdaptRLE,
    uint64_t matrixWidth)
{
    // load input file to internal representation vector
    vector<uint8_t> inData; // input data for Huffman tree (may be transformed before)
    int c;
    while ((c = ifs.get()) != EOF) {
        inData.push_back(c);
    }
    ifs.close();

    // check valid matrix size (only when using adaptive block RLE)
    if (useAdaptRLE && (inData.size() % matrixWidth) != 0)
    {
        cerr << "ERROR: invalid size of input 2D data detected\n";
        exit(6);
    }
    uint64_t matrixHeight = inData.size() / matrixWidth;

    // perform required TRANSFORMATIONS
    if (useDiffModel) {
        applyDiffModel(inData);
    }
    if (useAdaptRLE) {
        inData = applyAdaptRLE(inData, matrixWidth, matrixHeight);
    }
    else {
        inData = applyRLE(inData);
    }
    vector<bool> outBits = applyHuffman(inData);

    vector<uint8_t> outData;
    // first header for Huffman coding
    outData = createHuffHeader(inData.size(), useDiffModel, useAdaptRLE);

    // then data; convert bit vector to byte array
    for (size_t i = 0; i < outBits.size(); i += CHAR_BIT) {
        uint8_t curByte = 0;
        for (int j = 0; j < CHAR_BIT; j++) {
            curByte = (curByte << 1) | outBits[i + j];
        }
        outData.push_back(curByte);
    }

    return outData;
}

// decompress data of the given input stream (based on its header)
vector<uint8_t> huffDecompress(ifstream &ifs)
{
    // read total byte count to decode using Huffman
    uint64_t byteCount;
    ifs.read((char *)&byteCount, sizeof(uint64_t));
    // read flags
    int c = ifs.get();
    bool diffModelUsed = (uint8_t(c) >> 7) & 0x01;
    bool adaptRLEUsed = (uint8_t(c) >> 6) & 0x01;
    if (c == EOF) // check if any errors during header reading
    {
        cerr << "ERROR: invalid or missing Huffman coding header\n";
        ifs.close(); // close file handler before exit
        exit(8);
    }

    // load input file
    deque<bool> inData;
    while ((c = ifs.get()) != EOF) {
        for (int i = CHAR_BIT; i > 0; i--) {
            inData.push_back((c >> (i - 1)) & 0x01);
        }
    }
    ifs.close();

    // revert appropriate TRANSFORMATIONS
    deque<uint8_t> huffDecoded = revertHuffman(inData, byteCount);
    vector<uint8_t> outData;
    if (adaptRLEUsed) {
        outData = revertAdaptRLE(huffDecoded);
    } else {
        outData = revertRLE(huffDecoded);
    }
    if (diffModelUsed) {
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
        ofs.close(); // close file handler before exit
        exit(7);
    }

    ofs.write((char *) vec.data(), vec.size());
    // ofs will be closed automatically (end of this scope)
}

// redirect input to stderr, but also print a help hint
void cerrh(const char *s) {
    cerr << s << "try 'huff_codec -h' for more information\n";
}

// entry point of program
int main(int argc, char *argv[])
{
    // default setup
    bool useCompr = true;
    bool useDiffModel = false;
    bool useAdaptRLE = false;

    string ifp; // input file path (empty by default constructor)
    string ofp = "b.out"; // default path
    uint64_t matrixWidth = 512; // default value

    // argument processing
    // options are designed to be more tolerant (yet they meet the assignment)
    int opt;
    while ((opt = getopt(argc, argv, ":cdmai:o:w:h")) != -1)
    {
        switch (opt)
        {
        case 'c': useCompr = true; break;
        case 'd': useCompr = false; break;
        case 'm': useDiffModel = true; break;
        case 'a': useAdaptRLE = true; break;
        case 'i': ifp = optarg; break;
        case 'o': ofp = optarg; break;
        case 'w': matrixWidth = stoull(optarg); break;
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
    if (useCompr && matrixWidth == 0)
    {
        cerrh("ERROR: invalid 2D data width\n");
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
        outData = huffCompress(ifs, useDiffModel, useAdaptRLE, matrixWidth);
    } else {
        outData = huffDecompress(ifs);
    }
    
    writeOutData(outData, ofp);
}
