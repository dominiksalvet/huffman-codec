// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-26
// filename: transform.hpp
// summary: Header file of data transforming helper functions.

#pragma once

#include <vector>
#include <cstdint>
#include <deque>

using std::deque;
using std::vector;

#define INIT_RLE_BLOCK_SIZE 8
#define MAX_RLE_DOUBLING_STEPS 7 // for searching optimal block size


// transform pixel values to their differences (in situ)
// this algorithm utilizes the properties of two's complement (underflow)
void applyDiffModel(vector<uint8_t> &vec);
// revert the differential model (in situ)
// also uses the two's complement properties (overflow)
void revertDiffModel(vector<uint8_t> &vec);

// apply run-length encoding without explicit tag (MNP-5 Microcom format)
vector<uint8_t> applyRLE(const vector<uint8_t> &vec);
// recover the given RLE-encoded data
vector<uint8_t> revertRLE(const deque<uint8_t> &deq);

// apply adaptive block RLE with the best found block size (automatically)
// it also creates its header (besides others, block size is stored there)
vector<uint8_t> applyAdaptRLE(
    const vector<uint8_t> &matrix,
    uint64_t matrixWidth,
    uint64_t matrixHeight);
// revert adaptive block RLE, it also parses its header and set up
// configuration based on it (e.g., block size)
vector<uint8_t> revertAdaptRLE(deque<uint8_t> &deq);

// apply Huffman FGK coding and return bit vector
vector<bool> applyHuffman(const vector<uint8_t> &vec);
// revert Huffman coding of given bits and expected count of bytes
deque<uint8_t> revertHuffman(deque<bool> &deq, uint64_t byteCount);

// returns the total number of blocks in the matrix
uint64_t getBlockCount(uint64_t matrixWidth, uint64_t matrixHeight, uint64_t blockSize);
