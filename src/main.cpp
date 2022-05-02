// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-20
// filename: main.cpp
// summary: Adaptive Huffman codec with multiple options. It works with any file,
//          having extra features for 2D data (e.g., 8-bit grayscale images).

#include <iostream>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <climits>
#include <utility>

#include "transform.hpp"
#include "huffman.hpp"

using namespace std;

#define RLE_BLOCK_SIZE 8

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


// adds bytes of compressed file header to given vector, the format is following:
//   ordinary parts: <64b-byte-count><8b-flags>
//   optional following parts: <64b-matrix-width><block-scan-dirs>
void createHeader(
    vector<uint8_t> &vec,
    uint64_t byteCount,
    bool useDiffModel,
    bool useAdaptRLE,
    uint64_t matrixWidth,
    vector<bool> scanDirs)
{
    // header part <64b-byte-count> to indicate total number of encoded bytes
    for (unsigned int i = 0; i < sizeof(uint64_t); i++) {
        vec.push_back(byteCount >> (CHAR_BIT * i));
    }
    vec.push_back(
        // header part <8b-flags> [x-------] to indicate whether diff model was used
        uint8_t(useDiffModel) << 7 |
        // header part <8b-flags> [-x------] to indicate whether adaptive RLE was used
        uint8_t(useAdaptRLE) << 6
    );
    // TODO: continue here
    if (useAdaptRLE) // optional
    {
        // header part <64b-matrix-width> to indicate 2D data width
        for (unsigned int i = 0; i < sizeof(uint64_t); i++) {
            vec.push_back(matrixWidth >> (CHAR_BIT * i));
        }
    }
}

// compress data based on several given options
vector<uint8_t> compress(
    ifstream& ifs,
    bool useDiffModel,
    bool useAdaptRLE,
    uint64_t matrixWidth)
{
    // load input file to internal representation vector
    vector<uint8_t> inData;
    int c;
    while ((c = ifs.get()) != EOF) {
        inData.push_back(c);
    }
    ifs.close();

    // check valid matrix size
    if ((inData.size() % matrixWidth) != 0)
    {
        cerr << "ERROR: invalid size of input 2D data detected\n";
        exit(6);
    }
    uint64_t matrixHeight = inData.size() / matrixWidth;

    // perform required transformations
    if (useDiffModel) {
        applyDiffModel(inData);
    }
    pair<vector<bool>, vector<uint8_t>> adaptRLEPair;
    if (useAdaptRLE) {
        adaptRLEPair = applyAdaptRLE(inData, matrixWidth, matrixHeight, RLE_BLOCK_SIZE);
        inData = get<1>(adaptRLEPair); // get first item from the pair
    }
    else {
        inData = applyRLE(inData);
    }
    vector<bool> outBits = applyHuffman(inData);

    vector<uint8_t> outData;
    // first create header (see the function comment for header format)
    createHeader(
        outData,
        inData.size(),
        useDiffModel, useAdaptRLE, // flags
        matrixWidth,
        get<0>(adaptRLEPair));

    // then data; convert bit vector to byte array
    for (size_t i = 0; i < outBits.size(); i += 8) {
        uint8_t curByte = 0;
        for (int j = 0; j < CHAR_BIT; j++) {
            curByte = (curByte << 1) | outBits[i + j];
        }
        outData.push_back(curByte);
    }

    return outData;
}

// decompress data of the given input stream (based on its header)
// it respects the header format as described sooner
vector<uint8_t> decompress(ifstream &ifs)
{
    // read total byte count to decode
    uint64_t byteCount;
    ifs.read((char *)&byteCount, sizeof(uint64_t));
    // read diff model flag
    int c = ifs.get();
    if (c == EOF)
    {
        cerr << "ERROR: invalid compressed file header\n";
        ifs.close(); // close file handler before exit
        exit(8);
    }
    bool diffModelUsed = c >> 7;

    // load input file to internal representation vector
    queue<bool> inData;
    while ((c = ifs.get()) != EOF) {
        for (int i = 0; i < CHAR_BIT; i++) {
            inData.push((c >> (CHAR_BIT - i - 1) & 0x01));
        }
    }
    ifs.close();

    // revert appropriate transformations
    vector<uint8_t> outData = revertHuffman(inData, byteCount);
    outData = revertRLE(outData);
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
        outData = compress(ifs, useDiffModel, useAdaptRLE, matrixWidth);
    } else {
        outData = decompress(ifs);
    }
    
    writeOutData(outData, ofp);
}
