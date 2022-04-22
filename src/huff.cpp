// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-20
// filename: huff.cpp
// summary: The implementation of Huffman tree and helper functions.

#include "huff.hpp"

bool isLeaf(const HuffNode &node)
{
    return !(node.left) && !(node.right);
}

HuffTree::HuffTree(uint8_t firstByteData)
{
    this->root = new HuffNode{0, 1, NULL, NULL, NULL};
    HuffNode *NYT = new HuffNode{0, 0, this->root, NULL, NULL};
    HuffNode *node = new HuffNode{firstByteData, 1, this->root, NULL, NULL};
    this->root->left = NYT;
    this->root->right = node;
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

void HuffTree::deleteNode(const HuffNode *node)
{
    if (node != NULL)
    {
        deleteNode(node->left);
        deleteNode(node->right);
        delete node;
    }
}
