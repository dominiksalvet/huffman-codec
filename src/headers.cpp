// author: Dominik Salvet
// login: xsalve03
// date: 2022-05-02
// filename: headers.cpp
// summary: Implementation of functions working with compressed file headers.

#include "headers.hpp"

#include <climits>


vector<uint8_t> createAdaptRLEHeader(
    uint64_t matrixWidth,
    uint64_t matrixHeight,
    vector<bool> scanDirs)
{
    vector<uint8_t> finalVec;

    // header part <64b-matrix-width> to indicate 2D data width
    for (unsigned int i = 0; i < sizeof(uint64_t); i++) {
        finalVec.push_back(matrixWidth >> (CHAR_BIT * i));
    }

    // header part <64b-matrix-height> to indicate 2D data height
    for (unsigned int i = 0; i < sizeof(uint64_t); i++) {
        finalVec.push_back(matrixHeight >> (CHAR_BIT * i));
    }

    // header part <block-scan-dirs> to indicate scan direction for each block
    // horizontal scan - 1, vertical scan - 0
    uint8_t curByte = 0;
    uint64_t bitCount = 0;
    for (bool scanDir : scanDirs) // scan direction bits to bytes
    {
        curByte = (curByte << 1) | scanDir;
        bitCount++;

        if (bitCount % CHAR_BIT == 0) {
            finalVec.push_back(curByte);
        }
    }
    // scale to byte resolution, so adding remaining bits if needed
    if (bitCount % CHAR_BIT != 0)
    {
        do {
            curByte = (curByte << 1) | 0; // value does not matter
            bitCount++;
        } while (bitCount % CHAR_BIT != 0);
        finalVec.push_back(curByte);
    }

    return finalVec;
}

vector<uint8_t> createHuffHeader(uint64_t byteCount, bool useDiffModel, bool useAdaptRLE)
{
    vector<uint8_t> finalVec;

    // header part <64b-byte-count> to indicate total number of encoded bytes
    for (unsigned int i = 0; i < sizeof(uint64_t); i++) {
        finalVec.push_back(byteCount >> (CHAR_BIT * i));
    }

    // flags
    finalVec.push_back(
        // header part <8b-flags> [x-------] to indicate whether diff model was used
        uint8_t(useDiffModel) << 7 |
        // header part <8b-flags> [-x------] to indicate whether adaptive RLE was used
        uint8_t(useAdaptRLE) << 6
    );

    return finalVec;
}
