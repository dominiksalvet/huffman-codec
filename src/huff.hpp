// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-20
// filename: huff.hpp
// summary: Header file for Huffman FGK tree and helper functions.

#pragma once

#include <vector>
#include <cstdint>
#include <queue>
#include <ostream>

using std::vector;
using std::queue;
using std::ostream;

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

    // print internal representation of tree to given stream (for debugging)
    void print(ostream &os);

private:
    HuffNode *root;
    HuffNode *nodeNYT;

    // pointers to symbol nodes, some may be unsed when diff model is not used
    // nevertheless, it has no effect on the final result
    HuffNode *symbolNodes[2 * MAX_SYMBOLS] = {}; // initialized with nullptrs
    
    int bitsInSymbol; // number of bits in one symbol
    
    // go through the tree up to the root to provide the code of the node symbol
    vector<bool> nodeToCode(HuffNode *const node);
    // recursively search given node for greatest node number with the given frequency
    HuffNode* findSuccNode(HuffNode *const node, uint16_t freq);
    // swap two given nodes (must not be called on the root node)
    void swapNodes(HuffNode *const node1, HuffNode *const node2);

    // clean-up resources of the given node
    void deleteNode(const HuffNode *node);
    // print recursively given node to given stream (for debugging)
    void printNode(const HuffNode *node, ostream &os);
};
