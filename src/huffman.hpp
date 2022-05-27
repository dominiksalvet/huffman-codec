//------------------------------------------------------------------------------
// Copyright 2022 Dominik Salvet
// https://github.com/dominiksalvet/huffman-codec
//------------------------------------------------------------------------------
// Header file for Huffman FGK tree and helper functions.
//------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <ostream>
#include <deque>
#include <vector>

using std::deque;
using std::ostream;
using std::vector;

#define MAX_SYMBOLS 256 // max possible symbols
#define BITS_IN_SYMBOL 8 // number of bits in one symbol


struct HuffNode
{
    uint16_t nodeNum;
    uint64_t freq; // range is big enough for any real data
    uint8_t symbol; // for leaf nodes only

    HuffNode *parent;
    HuffNode *left, *right;
};

// check if the given node is a leaf node
bool isLeaf(const HuffNode *node);


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
    vector<bool> encode(uint8_t symbol);
    // decode and extract one symbol from given code
    // return -1 when unexpected end of input stream from the code
    int decode(deque<bool> *const code);

    // update the tree based on given symbol
    void update(uint8_t symbol);

    // print internal representation of tree to given stream (for debugging)
    void print(ostream &os);

private:
    // pointers to root and NYT node
    HuffNode *root;
    HuffNode *nodeNYT;

    // pointers to symbol nodes
    HuffNode *symbolNodes[MAX_SYMBOLS] = {}; // initialized with nullptrs
    
    // go through the tree up to the root to provide the code of the node symbol
    vector<bool> nodeToCode(HuffNode *const node);
    // recursively search given node for greatest node number with the given frequency
    HuffNode* findSuccNode(HuffNode *const node, uint64_t freq);
    // swap two given nodes (must not be called on the root node)
    void swapNodes(HuffNode *const node1, HuffNode *const node2);

    // clean-up resources of the given node
    void deleteNode(const HuffNode *node);
    // print recursively given node to given stream (for debugging)
    void printNode(const HuffNode *node, ostream &os);
};
