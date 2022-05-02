// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-26
// filename: transform.cpp
// summary: Implementation of data transforming helper functions.

#include "transform.hpp"

#include <iostream>
#include <climits>
#include <utility>

#include "huffman.hpp"

using std::cerr;
using std::pair;


// -------------------------- TRANSFORMATION ---------------------------------

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

void revertDiffModel(vector<uint8_t> &vec)
{
    uint8_t prevVal = 0;
    for (size_t i = 0; i < vec.size(); i++)
    {
        vec[i] += prevVal; // may overflow (truncated)
        prevVal = vec[i];
    }
}

vector<uint8_t> applyRLE(const vector<uint8_t> &vec)
{
    vector<uint8_t> finalVec; // new vector
    
    uint8_t matchByte = 0;
    int matchCount = 0;
    for (size_t i = 0; i < vec.size(); i++)
    {
        uint8_t curByte = vec[i];

        // exclude the first (or reset) and last iteration from matching
        if (curByte == matchByte && matchCount != 0 && i != vec.size() - 1)
        {
            matchCount++;

            if (matchCount <= 3) {
                finalVec.push_back(curByte);
            }
            else if (matchCount == 258) // 255 + 3
            {
                finalVec.push_back(255);
                matchCount = 0; // reset
            }
        }
        else
        {
            if (matchCount >= 3) {
                // preceding three characters are encoded directly
                finalVec.push_back(matchCount - 3);
            }

            finalVec.push_back(curByte);
            matchByte = curByte;
            matchCount = 1;
        }
    }

    return finalVec;
}

vector<uint8_t> revertRLE(const vector<uint8_t> &vec)
{
    vector<uint8_t> finalVec; // new vector

    uint8_t matchByte = 0;
    int matchCount = 0;
    for (uint8_t curByte : vec)
    {
        if (matchCount == 3)
        {
            // unroll the encoded number of bytes
            for (int i = 0; i < curByte; i++) {
                finalVec.push_back(matchByte);
            }
            matchCount = 0;
        }
        else
        {
            finalVec.push_back(curByte);

            if (matchByte == curByte) {
                matchCount++;
            } else
            {
                matchByte = curByte;
                matchCount = 1;
            }
        }
    }

    return finalVec;
}

pair<vector<bool>, vector<uint8_t>> applyAdaptRLE(
    const vector<uint8_t> &vec,
    uint64_t matrixWidth,
    uint64_t matrixHeight,
    uint64_t blockSize)
{
    vector<bool> scanDirs; // scan directions
    vector<uint8_t> finalVec;

    uint64_t blockCount = (matrixWidth / blockSize) * (matrixHeight / blockSize);
    vector<uint8_t> horVec, verVec; // horizontal, vertical order
    for (uint64_t i = 0; i < blockCount; i++)
    {
        horVec = applyRLE(getBlockVector(vec, matrixWidth, blockSize, i, true));
        verVec = applyRLE(getBlockVector(vec, matrixWidth, blockSize, i, false));

        // check which scan direction is better
        if (horVec.size() <= verVec.size())
        {
            scanDirs.push_back(1);
            finalVec.insert(finalVec.end(), horVec.begin(), horVec.end());
        }
        else
        {
            scanDirs.push_back(0);
            finalVec.insert(finalVec.end(), verVec.begin(), verVec.end());
        }
    }

    return make_pair(scanDirs, finalVec);
}

vector<bool> applyHuffman(const vector<uint8_t> &vec)
{
    // create the Huffman FGK tree
    HuffTree huffTree; // call default contructor

    vector<bool> finalVec;
    // encode input data to bit vector
    for (uint8_t symbol : vec)
    {
        vector<bool> symbolCode = huffTree.encode(symbol);
        // append symbol code to the existing code
        finalVec.insert(finalVec.end(), symbolCode.begin(), symbolCode.end());
        huffTree.update(symbol);
    }

    // add remaining bits so their final count is divisible by bits in symbol
    while (finalVec.size() % CHAR_BIT != 0) {
        finalVec.push_back(0); // value does not matter
    }

    return finalVec;
}

vector<uint8_t> revertHuffman(queue<bool> &vec, uint64_t byteCount)
{
    HuffTree huffTree; // create the Huffman FGK tree

    vector<uint8_t> finalVec;
    for (uint64_t i = 0; i < byteCount; i++)
    {
        int decResult = huffTree.decode(&vec);
        if (decResult == -1)
        {
            cerr << "ERROR: invalid compressed file contents\n";
            exit(9);
        }
        uint8_t symbol = decResult;
    
        huffTree.update(symbol);
        finalVec.push_back(symbol);
    }

    return finalVec;
}

// -------------------------- HELPER FUNCTIONS ---------------------------------

vector<uint8_t> getBlockVector(
    const vector<uint8_t> &vec,
    uint64_t matrixWidth,
    uint64_t blockSize,
    uint64_t blockIndex,
    bool horScan)
{
    // compute block base address
    uint64_t blocksInLine = matrixWidth / blockSize;
    uint64_t blockColumnBase = (blockIndex % blocksInLine) * blockSize;
    uint64_t blockRowBase = (blockIndex / blocksInLine) * matrixWidth * blockSize;
    uint64_t blockBase = blockRowBase + blockColumnBase;

    vector<uint8_t> blockVec;
    for (uint64_t i = 0; i < blockSize; i++)
    {
        for (uint64_t j = 0; j < blockSize; j++)
        {
            uint64_t xIndex = horScan ? j : i;
            uint64_t yIndex = horScan ? i : j;

            blockVec.push_back(vec[blockBase + (yIndex * matrixWidth) + xIndex]);
        }
    }

    return blockVec;
}
