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
