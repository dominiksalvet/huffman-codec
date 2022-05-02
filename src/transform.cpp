// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-26
// filename: transform.cpp
// summary: Implementation of data transforming helper functions.

#include "transform.hpp"

#include <iostream>
#include <climits>

#include "huffman.hpp"
#include "headers.hpp"

using std::cerr;

// -------------------------- TRANSFORMATION ---------------------------------

void applyDiffModel(vector<uint8_t> &vec)
{
    uint8_t prevVal = 0;
    for (uint8_t &vecItem : vec)
    {
        uint8_t curVal = vecItem;
        vecItem = (curVal - prevVal); // truncated result of underflow
        prevVal = curVal;
    }
}

void revertDiffModel(vector<uint8_t> &vec)
{
    uint8_t prevVal = 0;
    for (uint8_t &vecItem : vec)
    {
        vecItem += prevVal; // may overflow (truncated)
        prevVal = vecItem;
    }
}

vector<uint8_t> applyRLE(const vector<uint8_t> &vec)
{
    vector<uint8_t> finalVec; // new vector
    
    uint8_t matchByte = 0;
    int matchCount = 0;
    for (auto it = vec.begin(); it != vec.end(); ++it)
    {
        uint8_t curByte = *it;

        // exclude the first (or reset) and last iteration from matching
        if (curByte == matchByte && matchCount != 0 && next(it) != vec.end())
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

vector<uint8_t> revertRLE(const deque<uint8_t> &deq)
{
    vector<uint8_t> finalVec; // new vector

    uint8_t matchByte = 0;
    int matchCount = 0;
    for (uint8_t curByte : deq)
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

vector<uint8_t> applyAdaptRLE(
    const vector<uint8_t> &vec,
    uint64_t matrixWidth,
    uint64_t matrixHeight)
{
    vector<bool> scanDirs; // scan directions
    vector<uint8_t> blockData;

    uint64_t blockCount = getBlockCount(matrixWidth, matrixHeight, RLE_BLOCK_SIZE);
    vector<uint8_t> horVec, verVec; // horizontal, vertical order
    for (uint64_t i = 0; i < blockCount; i++)
    {
        horVec = applyRLE(getBlockVector(vec, matrixWidth, RLE_BLOCK_SIZE, i, true));
        verVec = applyRLE(getBlockVector(vec, matrixWidth, RLE_BLOCK_SIZE, i, false));

        // check which scan direction is better
        if (horVec.size() <= verVec.size())
        {
            scanDirs.push_back(1);
            blockData.insert(blockData.end(), horVec.begin(), horVec.end());
        }
        else
        {
            scanDirs.push_back(0);
            blockData.insert(blockData.end(), verVec.begin(), verVec.end());
        }
    }

    // first create header for adaptive RLE
    vector<uint8_t> finalVec = createAdaptRLEHeader(
        matrixWidth, matrixHeight, RLE_BLOCK_SIZE, scanDirs);
    
    // then append block data
    finalVec.insert(finalVec.end(), blockData.begin(), blockData.end());

    return finalVec;
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

deque<uint8_t> revertHuffman(deque<bool> &deq, uint64_t byteCount)
{
    HuffTree huffTree; // create the Huffman FGK tree

    deque<uint8_t> finalDeq;
    for (uint64_t i = 0; i < byteCount; i++)
    {
        int decResult = huffTree.decode(&deq);
        if (decResult == -1)
        {
            cerr << "ERROR: invalid compressed file contents\n";
            exit(9);
        }
        uint8_t symbol = decResult;
    
        huffTree.update(symbol);
        finalDeq.push_back(symbol);
    }

    return finalDeq;
}

// -------------------------- HELPER FUNCTIONS ---------------------------------

uint64_t getBlockCount(
    uint64_t matrixWidth,
    uint64_t matrixHeight,
    uint64_t blockSize)
{
    return (matrixWidth / blockSize) * (matrixHeight / blockSize);
}

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
