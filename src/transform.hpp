// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-26
// filename: transform.hpp
// summary: Header file of data transforming helper functions.

#pragma once

#include <vector>
#include <cstdint>
#include <queue>
#include <utility>

using std::vector;
using std::queue;
using std::pair;


// transform pixel values to their differences (in situ)
// this algorithm utilizes the properties of two's complement (underflow)
void applyDiffModel(vector<uint8_t> &vec);
// revert the differential model (in situ)
// also uses the two's complement properties (overflow)
void revertDiffModel(vector<uint8_t> &vec);

// apply run-length encoding without explicit tag (MNP-5 Microcom format)
vector<uint8_t> applyRLE(const vector<uint8_t> &vec);
// recover the given RLE-encoded data
vector<uint8_t> revertRLE(const vector<uint8_t> &vec);

// apply adaptive block RLE based on given arguments
// returns a pair of:
//   bit vector - scan directions, each block has its bit
//   vector of bytes - merge of all blocks to one vector
pair<vector<bool>, vector<uint8_t>> applyAdaptRLE(
    const vector<uint8_t> &vec,
    uint64_t matrixWidth,
    uint64_t matrixHeight,
    uint64_t blockSize);

// apply Huffman FGK coding and return bit vector
vector<bool> applyHuffman(const vector<uint8_t> &vec);
// revert Huffman coding of given bit vector and expected count of bytes
vector<uint8_t> revertHuffman(queue<bool> &vec, uint64_t byteCount);

// return vector of items in the given block with selected scan direction
// we give block index -> it returns vector of its items
vector<uint8_t> getBlockVector(
    const vector<uint8_t> &vec, // complete vector of data
    uint64_t matrixWidth,
    uint64_t blockSize,
    uint64_t blockIndex, // index of the block in the vector
    bool horScan); // horizontal or vertical scanning
