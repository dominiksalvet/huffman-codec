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

#include "transform.hpp"
#include "huffman.hpp"

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


// redirect input to stderr, but also print a help hint
void cerrh(const char *s) {
    cerr << s << "try 'huff_codec -h' for more information\n";
}

// compress data based on several given options
// data is preceded with header: <64b-byte-count><1b-diff-model>
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

    // perform required transformations
    if (useDiffModel) {
        applyDiffModel(inData);
    }
    inData = applyRLE(inData);

    // create the Huffman FGK tree
    HuffTree huffTree; // call default contructor

    vector<bool> outBits;
    // header part <1b-diff-model> to indicate whether diff model was used
    outBits.push_back(useDiffModel);

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
    // header part <64b-byte-count> to indicated total number of encoded bytes
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

// decompress data of the given input stream (based on its header)
// it respects the header as described in the compress function comments
vector<uint8_t> decompress(ifstream &ifs)
{
    // read total byte count to decode
    uint64_t byteCount;
    ifs.read((char *)&byteCount, sizeof(uint64_t));
    if (!ifs)
    {
        cerr << "ERROR: invalid compressed file header\n";
        ifs.close(); // close file handler before exit
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
    bool diffModelUsed = inData.front(); inData.pop(); // remove from queue

    // create the Huffman FGK tree
    HuffTree huffTree;

    vector<uint8_t> outData;
    for (uint64_t i = 0; i < byteCount; i++)
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

    // revert appropriate transformations
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
