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

#define BITS_IN_SYMBOL 9
#define MAX_SYMBOLS 512
#define NYT_INDEX 511 // last index (would be unused otherwise)


struct HuffNode
{
    uint16_t nodeNum; // type must respect MAX_SYMBOLS
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
    HuffTree();
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
    // initialized as empty (includes NYT code)
    vector<bool> codes[MAX_SYMBOLS]; // codes of symbols
    
    // clean-up resources of the given node
    void deleteNode(const HuffNode *node);
};
