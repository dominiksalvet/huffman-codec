// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-26
// filename: transform.cpp
// summary: Implementation of data transforming helper functions.

#include "transform.hpp"

#include <iostream>
#include <climits>
#include <utility>
#include <tuple>

#include "huffman.hpp"
#include "headers.hpp"

using std::cerr;
using std::get;
using std::swap;

// -------------------------- HIDDEN HELPER FUNCTIONS ------------------------------

// return the base address of the given block
uint64_t getBlockBase(uint64_t matrixWidth, uint64_t blockSize, uint64_t blockIndex)
{
    uint64_t blocksInLine = matrixWidth / blockSize;
    uint64_t blockColumnBase = (blockIndex % blocksInLine) * blockSize;
    uint64_t blockRowBase = (blockIndex / blocksInLine) * matrixWidth * blockSize;

    return blockRowBase + blockColumnBase;
}

// return vector of items in the given block with selected scan direction
// we give block index -> it returns vector of its items
vector<uint8_t> getBlockVector(
    const vector<uint8_t> &vec,
    uint64_t matrixWidth,
    uint64_t blockSize,
    uint64_t blockIndex,
    bool horScan)
{
    // compute block base address
    uint64_t blockBase = getBlockBase(matrixWidth, blockSize, blockIndex);

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

// apply adaptive block RLE based on given arguments (also creates its header)
vector<uint8_t> applyAdaptRLE(
    const vector<uint8_t> &vec,
    uint64_t matrixWidth,
    uint64_t matrixHeight,
    uint64_t blockSize)
{
    vector<bool> scanDirs; // scan directions
    vector<uint8_t> blockData;

    uint64_t blockCount = getBlockCount(matrixWidth, matrixHeight, blockSize);
    vector<uint8_t> horVec, verVec; // horizontal, vertical order
    for (uint64_t i = 0; i < blockCount; i++)
    {
        horVec = applyRLE(getBlockVector(vec, matrixWidth, blockSize, i, true));
        verVec = applyRLE(getBlockVector(vec, matrixWidth, blockSize, i, false));

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
        matrixWidth, matrixHeight, blockSize, scanDirs);
    
    // then append block data
    finalVec.insert(finalVec.end(), blockData.begin(), blockData.end());

    return finalVec;
}

// perform one step of decoding RLE, appending the result to target vector
void revertRLEStep(vector<uint8_t> &tarVec, uint8_t &matchByte, int &matchCount, uint8_t curByte)
{
    if (matchCount == 3)
    {
        // unroll the encoded number of bytes
        for (int i = 0; i < curByte; i++) {
            tarVec.push_back(matchByte);
        }
        matchCount = 0;
    }
    else
    {
        tarVec.push_back(curByte);

        if (matchByte == curByte) {
            matchCount++;
        } else
        {
            matchByte = curByte;
            matchCount = 1;
        }
    }
}

// extract and decode one block encoded in RLE (boundaries checks included)
vector<uint8_t> revertRLEBlock(deque<uint8_t> &deq, uint64_t reqResultSize)
{
    vector<uint8_t> finalVec; // new vector

    uint8_t matchByte = 0;
    int matchCount = 0;
    while (finalVec.size() < reqResultSize)
    {
        if (deq.empty())
        {
            cerr << "ERROR: unexpected end of adaptive block RLE data\n";
            exit(14);
        }

        uint8_t curByte = deq.front(); deq.pop_front(); // extract
        revertRLEStep(finalVec, matchByte, matchCount, curByte); // decode
    }

    if (finalVec.size() != reqResultSize)
    {
        cerr << "ERROR: invalid adaptive block RLE file contents\n";
        exit(13);
    }

    return finalVec;
}

// insert given block vector to given target matrix vector based on given arguments
// we given block index -> it inserts it where it should be in the final matrix
void insertBlockVector(
    vector<uint8_t> &tarVec,
    const vector<uint8_t> &blockVec,
    uint64_t matrixWidth,
    uint64_t blockSize,
    uint64_t blockIndex,
    bool horScan)
{
    // compute block base address
    uint64_t blockBase = getBlockBase(matrixWidth, blockSize, blockIndex);

    for (uint64_t i = 0; i < blockSize; i++)
    {
        for (uint64_t j = 0; j < blockSize; j++)
        {
            // current matrix vector address
            uint64_t tarVecAddr = blockBase + (i * matrixWidth) + j;

            uint64_t srcX = horScan ? j : i;
            uint64_t srcY = horScan ? i : j;

            tarVec[tarVecAddr] = blockVec[(srcY * blockSize) + srcX];
        }
    }
}

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
    for (uint8_t curByte : deq) {
        revertRLEStep(finalVec, matchByte, matchCount, curByte);
    }

    return finalVec;
}

vector<uint8_t> applyAdaptRLE(
    const vector<uint8_t> &vec,
    uint64_t matrixWidth,
    uint64_t matrixHeight)
{
    uint64_t curBlockSize = INIT_RLE_BLOCK_SIZE;
    if (matrixWidth < curBlockSize || matrixHeight < curBlockSize)
    {
        cerr << "ERROR: too small 2D data dimensions\n";
        exit(12);
    }

    // we will find the most optimal block size
    vector<uint8_t> bestVec;
    // first step before the loop
    bestVec = applyAdaptRLE(vec, matrixWidth, matrixHeight, curBlockSize);

    curBlockSize *= 2;
    int doublingSteps = 1; // number of doubling block size
    vector<uint8_t> curVec;
    while (doublingSteps <= MAX_RLE_DOUBLING_STEPS &&
           curBlockSize <= matrixWidth && curBlockSize <= matrixHeight)
    {
        curVec = applyAdaptRLE(vec, matrixWidth, matrixHeight, curBlockSize);

        if (curVec.size() < bestVec.size()) {
            bestVec = curVec;
        }

        curBlockSize *= 2;
        doublingSteps++;
    }

    return bestVec;
}

vector<uint8_t> revertAdaptRLE(deque<uint8_t> &deq)
{
    tuple<uint64_t, uint64_t, uint64_t, vector<bool>> adaptRLETuple;
    adaptRLETuple = extractAdaptRLEHeader(deq);

    uint64_t matrixWidth = get<0>(adaptRLETuple);
    uint64_t matrixHeight = get<1>(adaptRLETuple);
    uint64_t blockSize = get<2>(adaptRLETuple);
    vector<bool> scanDirs = get<3>(adaptRLETuple);

    vector<uint8_t> finalVec(matrixWidth * matrixHeight);
    uint64_t blockCount = getBlockCount(matrixWidth, matrixHeight, blockSize);

    for (uint i = 0; i < blockCount; i++)
    {
        vector<uint8_t> curBlock = revertRLEBlock(deq, blockSize * blockSize);
        insertBlockVector(finalVec, curBlock, matrixWidth, blockSize, i, scanDirs[i]);
    }

    if (!deq.empty())
    {
        cerr << "ERROR: leftover data of adaptive block RLE detected\n";
        exit(15);
    }

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
            cerr << "ERROR: invalid Huffman coding file contents\n";
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
