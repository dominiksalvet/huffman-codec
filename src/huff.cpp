// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-20
// filename: huff.cpp
// summary: The implementation of Huffman FGK tree and helper functions.

#include "huff.hpp"

bool isLeaf(const HuffNode &node)
{
    return !node.left && !node.right;
}

bool isRoot(const HuffNode &node)
{
    return !node.parent;
}

// -------------------------- PUBLIC -------------------------------------------

HuffTree::HuffTree()
{
    this->root = new HuffNode{0, 0, NULL, NULL, NULL}; // NYT node
}

HuffTree::~HuffTree()
{
    deleteNode(this->root);
}

vector<bool> HuffTree::encode(uint8_t byteData)
{

}

uint8_t HuffTree::decode(vector<bool> huffData)
{

}

void HuffTree::update(uint8_t byteData)
{
    
}

// -------------------------- PRIVATE ------------------------------------------

void HuffTree::deleteNode(const HuffNode *node)
{
    if (node != NULL)
    {
        deleteNode(node->left);
        deleteNode(node->right);
        delete node;
    }
}
