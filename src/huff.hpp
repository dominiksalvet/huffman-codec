// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-20
// filename: huff.hpp
// summary: Header file for Huffman tree and helper functions.

#pragma once

#include <vector>
#include <cstdint>

using std::vector;

struct Node
{
    Node *parent;
    Node *left, *right;
};

class HuffTree
{
public:
    // initialize the Huffman tree
    HuffTree();
    // clean-up the tree
    ~HuffTree();

    // encode given byte based on current tree
    vector<bool> encode(uint8_t b);
    // decode given binary string to byte
    uint8_t decode(vector<bool> b);
    // update internal Huffman tree based on given byte
    void update(uint8_t b);
};
