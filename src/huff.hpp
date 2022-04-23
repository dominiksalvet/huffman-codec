// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-20
// filename: huff.hpp
// summary: Header file for Huffman FGK tree and helper functions.

#pragma once

#include <vector>
#include <cstdint>
#include <queue>

using std::vector;
using std::queue;

// this define is valid when diff model is not used
#define MAX_SYMBOLS 256 // max possible symbols


struct HuffNode
{
    uint16_t nodeNum;
    uint32_t freq; // min 4 GB of data per one tree
    uint16_t symbol; // for leaf nodes only

    HuffNode *parent;
    HuffNode *left, *right;
};

// check if the given node is a leaf node
bool isLeaf(const HuffNode *node);
// check whether the given node is root
bool isRoot(const HuffNode *node);
// check whether the given node is NYT
bool isNYTNode(const HuffNode *node);


// symbol is something to be encoded
// code is something to be decoded

class HuffTree
{
public:
    // initialize the Huffman FGK tree
    HuffTree(bool useDiffSymbols);
    // clean-up the tree
    ~HuffTree();

    // encode given symbol based on current tree
    vector<bool> encode(uint16_t symbol);
    // decode and extract one symbol from given code
    // return -1 when end of input stream
    int decode(queue<bool> &code);

    // update the tree based on given symbol
    void update(uint16_t symbol);

private:
    HuffNode *root;
    int bitsInSymbol; // number of bits in one symbol

    // these vectors are initialized as empty
    vector<bool> codeNYT; // code of NYT node
    // codes of symbols, some may be unsed when diff model is not used
    // nevertheless, it has no effect on the final result
    vector<bool> codes[2 * MAX_SYMBOLS];
    
    // clean-up resources of the given node
    void deleteNode(const HuffNode *node);
};
