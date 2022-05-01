// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-26
// filename: transform.cpp
// summary: Implementation of byte transforming helper functions.

#include "transform.hpp"


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
