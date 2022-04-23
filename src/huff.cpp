// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-20
// filename: huff.cpp
// summary: The implementation of Huffman FGK tree and helper functions.

#include "huff.hpp"


bool isLeaf(const HuffNode *node)
{
    return !node->left && !node->right;
}

bool isRoot(const HuffNode *node)
{
    return !node->parent;
}

bool isNYTNode(const HuffNode *node)
{
    return node->freq == 0;
}

// -------------------------- PUBLIC -------------------------------------------

HuffTree::HuffTree()
{
    // initial NYT node
    this->root = new HuffNode{2 * MAX_SYMBOLS - 1, 0, NYT_INDEX, NULL, NULL, NULL};
}

HuffTree::~HuffTree()
{
    deleteNode(this->root);
}

vector<bool> HuffTree::encode(uint16_t symbol)
{
    vector<bool> code = codes[symbol];

    if (code.empty()) // no code existing => not yet transmitted
    {
        code = codes[NYT_INDEX]; // we must start with NYT code

        // symbol to boolean vector conversion
        for (int i = 0; i < BITS_IN_SYMBOL; i++)
        {
            bool curBit = (symbol >> (BITS_IN_SYMBOL - i - 1)) & 0x01;
            code.push_back(curBit);
        }
    }

    return code;
}

int HuffTree::decode(queue<bool> &code)
{
    HuffNode *curNode = this->root;
    while (!isLeaf(curNode))
    {
        if (code.empty())
            return -1;

        // decision bit to choose the next node
        bool decBit = code.front(); code.pop();
        curNode = decBit ? curNode->right : curNode->left;
    }

    uint16_t finalSymbol = 0;
    if (isNYTNode(curNode))
    {
        for (int i = 0; i < BITS_IN_SYMBOL; i++)
        {
            if (code.empty())
                return -1;

            bool curBit = code.front(); code.pop();
            finalSymbol = (finalSymbol << 1) | curBit;
        }
    }
    else finalSymbol = curNode->symbol;

    return finalSymbol;
}

void HuffTree::update(uint16_t symbol)
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
