//------------------------------------------------------------------------------
// Copyright 2022 Dominik Salvet
// https://github.com/dominiksalvet/huffman-codec
//------------------------------------------------------------------------------
// Header file of functions working with compressed file headers.
//------------------------------------------------------------------------------

#pragma once

#include <vector>
#include <cstdint>
#include <tuple>
#include <deque>

using std::vector;
using std::tuple;
using std::deque;


// create header for adaptive RLE
// header parts: <64b-matrix-width><64b-matrix-height><64b-block-size><block-scan-dirs>
vector<uint8_t> createAdaptRLEHeader(
    uint64_t matrixWidth,
    uint64_t matrixHeight,
    uint64_t blockSize,
    vector<bool> scanDirs);
// extract adaptive RLE header from given deque of bytes (it removes the header items)
// it returns a tuple of:
//   * matrix width
//   * matrix height
//   * block size
//   * bit vector of block scan directions
tuple<uint64_t, uint64_t, uint64_t, vector<bool>> extractAdaptRLEHeader(deque<uint8_t> &deq);

// create header for Huffman coding (includes flags for used methods)
// header parts: <64b-byte-count><8b-flags>
vector<uint8_t> createHuffHeader(uint64_t byteCount, bool useDiffModel, bool useAdaptRLE);
