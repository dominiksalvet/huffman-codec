// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-20
// filename: huff.hpp
// summary: Header file for Huffman tree and helper functions.

#pragma once

#include <vector>
#include <cstdint>

using std::vector;

struct HuffNode
{
    uint8_t data; // for leaf nodes
    // NYT exclusively has freq=0
    uint32_t freq; // max 4 GB of data per one tree
    HuffNode *parent; // for sum propagation
    HuffNode *left, *right;
};

bool isLeaf(const HuffNode &node);

class HuffTree
{
public:
    // initialize the Huffman tree
    HuffTree(uint8_t firstByteData);
    // clean-up the tree
    ~HuffTree();

    // encode given byte based on current tree
    vector<bool> encode(uint8_t byteData);
    // decode given binary vector to byte
    uint8_t decode(vector<bool> huffData);
    // update internal Huffman tree based on given byte
    void update(uint8_t byteData);

private:
    HuffNode *root;

    // clean-up resources of the given node
    void deleteNode(const HuffNode *node);
};
