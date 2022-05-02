// author: Dominik Salvet
// login: xsalve03
// date: 2022-05-02
// filename: headers.hpp
// summary: Header file of functions working with compressed file headers.

#pragma once

#include <vector>
#include <cstdint>

using std::vector;


// create header for adaptive RLE
// header parts: <64b-matrix-width><64b-matrix-height><block-scan-dirs>
vector<uint8_t> createAdaptRLEHeader(uint64_t matrixWidth, uint64_t matrixHeight, vector<bool> scanDirs);
// create header for Huffman coding (includes flags for used methods)
// header parts: <64b-byte-count><8b-flags>
vector<uint8_t> createHuffHeader(uint64_t byteCount, bool useDiffModel, bool useAdaptRLE);
