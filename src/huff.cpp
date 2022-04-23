// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-20
// filename: huff.cpp
// summary: The implementation of Huffman FGK tree and helper functions.

#include "huff.hpp"

#include <climits>


bool isLeaf(const HuffNode *node)
{
    // no need to check the other child for Huffman FGK tree
    return node->left == nullptr;
}

bool isRoot(const HuffNode *node)
{
    return node->parent == nullptr;
}

bool isNYTNode(const HuffNode *node)
{
    return node->freq == 0;
}

// -------------------------- PUBLIC -------------------------------------------

HuffTree::HuffTree(bool useDiffSymbols)
{
    // there are twice as much symbols when diff model is used
    int realMaxSymbols = useDiffSymbols ? 2 * MAX_SYMBOLS : MAX_SYMBOLS;

    // NYT is not included in the symbols alphabet (hence this formula)
    uint16_t firstNodeNum = 2 * realMaxSymbols + 1;
    // create tree with NYT node only
    this->root = new HuffNode{firstNodeNum, 0, 0, nullptr, nullptr, nullptr};

    // we consider only platforms where memory is addressable by bytes (8 bits)
    bitsInSymbol = useDiffSymbols ? CHAR_BIT + 1 : CHAR_BIT;
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
        code = codeNYT; // we must start with NYT code

        // symbol to boolean vector conversion
        for (int i = 0; i < bitsInSymbol; i++)
        {
            bool curBit = (symbol >> (bitsInSymbol - i - 1)) & 0x01;
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
        if (code.empty()) {
            return -1;
        }

        // decision bit to choose the next node
        bool decBit = code.front(); code.pop();
        curNode = decBit ? curNode->right : curNode->left;
    }

    uint16_t finalSymbol = 0;
    if (isNYTNode(curNode))
    {
        for (int i = 0; i < bitsInSymbol; i++)
        {
            if (code.empty()) {
                return -1;
            }

            bool curBit = code.front(); code.pop();
            finalSymbol = (finalSymbol << 1) | curBit;
        }
    }
    else { finalSymbol = curNode->symbol; }

    return finalSymbol;
}

void HuffTree::update(uint16_t symbol)
{
    
}

// -------------------------- PRIVATE ------------------------------------------

void HuffTree::deleteNode(const HuffNode *node)
{
    if (node != nullptr)
    {
        deleteNode(node->left);
        deleteNode(node->right);
        delete node;
    }
}
