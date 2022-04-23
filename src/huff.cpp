// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-20
// filename: huff.cpp
// summary: The implementation of Huffman FGK tree and helper functions.

#include "huff.hpp"

#include <climits>
#include <algorithm>

using std::reverse;

bool isLeaf(const HuffNode *node)
{
    // no need to check the other child for Huffman FGK tree
    return node->left == nullptr;
}

// -------------------------- PUBLIC -------------------------------------------

HuffTree::HuffTree(bool useDiffSymbols)
{
    // there are twice as much symbols when diff model is used
    int realMaxSymbols = useDiffSymbols ? 2 * MAX_SYMBOLS : MAX_SYMBOLS;

    // NYT is not included in the symbols alphabet (hence this formula)
    uint16_t firstNodeNum = 2 * realMaxSymbols; // include 0 as number
    // create tree with NYT node only
    root = new HuffNode{firstNodeNum, 0, 0, nullptr, nullptr, nullptr};
    nodeNYT = root;

    // we consider only platforms where memory is addressable by bytes (8 bits)
    bitsInSymbol = useDiffSymbols ? CHAR_BIT + 1 : CHAR_BIT;
}

HuffTree::~HuffTree()
{
    deleteNode(root);
}

vector<bool> HuffTree::encode(uint16_t symbol)
{
    vector<bool> code;
    HuffNode *symbolNode = symbolNodes[symbol];

    if (symbolNode == nullptr) // no symbol existing => not yet transmitted
    {
        code = nodeToCode(nodeNYT); // we must start with NYT code

        // current symbol to boolean vector conversion
        for (int i = 0; i < bitsInSymbol; i++)
        {
            bool curBit = (symbol >> (bitsInSymbol - i - 1)) & 0x01;
            code.push_back(curBit);
        }
    }
    else {
        code = nodeToCode(symbolNode);
    }

    return code;
}

int HuffTree::decode(queue<bool> &code)
{
    HuffNode *curNode = root;
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
    if (curNode == nodeNYT)
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

vector<bool> HuffTree::nodeToCode(HuffNode *const node)
{
    vector<bool> code;

    HuffNode *curNode = node;
    while(curNode != root) // up to the root
    {
        // add bits incrementally
        if (curNode->parent->left == curNode) {
            code.push_back(0);
        } else {
            code.push_back(1);
        }
        curNode = curNode->parent;
    }

    // the received code is in the reverse order
    reverse(code.begin(), code.end());
    return code;
}

void HuffTree::swapNodes(HuffNode *const node1, HuffNode *const node2)
{
    // swap nodes number (since that does not change when swapping nodes)
    uint16_t node1Num = node1->nodeNum;
    node1->nodeNum = node2->nodeNum;
    node2->nodeNum = node1Num;

    if (node1->parent->left == node1) {
        node1->parent->left = node2;
    } else {
        node1->parent->right = node2;
    }

    if (node2->parent->left == node2) {
        node2->parent->left = node1;
    } else {
        node2->parent->right = node1;
    }

    HuffNode *node1Parent = node1->parent;
    node1->parent = node2->parent;
    node2->parent = node1Parent;
}

void HuffTree::deleteNode(const HuffNode *node)
{
    if (node != nullptr)
    {
        deleteNode(node->left);
        deleteNode(node->right);
        delete node;
    }
}
