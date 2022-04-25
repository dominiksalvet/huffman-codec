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

HuffTree::~HuffTree() {
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
    HuffNode *node = symbolNodes[symbol];

    if (node == nullptr) // NYT node splitting (add new symbol)
    {
        HuffNode *leftChild = new HuffNode{
            uint16_t(nodeNYT->nodeNum - 2), 0, 0, nodeNYT, nullptr, nullptr};
        node = new HuffNode{
            uint16_t(nodeNYT->nodeNum - 1), 0, symbol, nodeNYT, nullptr, nullptr};
        
        nodeNYT->left = leftChild; // new NYT node
        nodeNYT->right = node; // new node for symbol

        nodeNYT = leftChild;
        symbolNodes[symbol] = node; // register new symbol
    }

    while (node != root)
    {
        HuffNode *succNode = findSuccNode(root, node->freq);

        // check if any valid successor found (also useless to switch same nodes)
        if (succNode != nullptr &&
            succNode != node->parent &&
            succNode != node) {
            swapNodes(node, succNode);
        }
        node->freq++;

        node = node->parent; // next node
    }
    node->freq++; // also increase root freq afterwards
}

void HuffTree::print(ostream &os) {
    printNode(root, os);
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

HuffNode* HuffTree::findSuccNode(HuffNode *const node, uint16_t freq)
{
    HuffNode *succNode = nullptr;

    if (!isLeaf(node) && node->freq > freq) // still higher value
    {
        HuffNode *leftSuccNode = findSuccNode(node->left, freq);
        HuffNode *rightSuccNode = findSuccNode(node->right, freq);

        if (leftSuccNode != nullptr && rightSuccNode != nullptr)
        {
            // prefer higher node number if both subtrees have candidate
            if (leftSuccNode->nodeNum > rightSuccNode->nodeNum) {
                succNode = leftSuccNode;
            } else {
                succNode = rightSuccNode;
            }
        }
        else { // otherwise set to valid pointer of those two if exists
            succNode = leftSuccNode != nullptr ? leftSuccNode : rightSuccNode;
        }
    }
    else if (node->freq == freq) {
        succNode = node;
    }

    return succNode;
}

void HuffTree::swapNodes(HuffNode *const node1, HuffNode *const node2)
{
    // swap nodes number (since that does not change when swapping nodes)
    uint16_t node1Num = node1->nodeNum;
    node1->nodeNum = node2->nodeNum;
    node2->nodeNum = node1Num;

    // first scan, then modify (to prevent bugs)
    bool node1IsLeftChild = false;
    if (node1->parent->left == node1) {
        node1IsLeftChild = true;
    }
    bool node2IsLeftChild = false;
    if (node2->parent->left == node2) {
        node2IsLeftChild = true;
    }

    if (node1IsLeftChild) {
        node1->parent->left = node2;
    } else {
        node1->parent->right = node2;
    }
    if (node2IsLeftChild) {
        node2->parent->left = node1;
    } else {
        node2->parent->right = node1;
    }

    HuffNode *node1Parent = node1->parent;
    node1->parent = node2->parent;
    node2->parent = node1Parent;
}

// -------------------------- HELPER FUNCTIONS ---------------------------------

void HuffTree::deleteNode(const HuffNode *node)
{
    if (node != nullptr)
    {
        deleteNode(node->left);
        deleteNode(node->right);
        delete node;
    }
}

void HuffTree::printNode(const HuffNode *node, ostream &os)
{
    os << "nodeNum: " << node->nodeNum << 
          ", freq: " << node->freq <<
          ", symbol: " << node->symbol;
    
    os << ", parent: ";
    if (node->parent != nullptr) {
        os << node->parent->nodeNum;
    } else {
        os << "NULL";
    }

    os << ", left: ";
    if (node->left != nullptr) {
        os << node->left->nodeNum;
    } else {
        os << "NULL";
    }

    os << ", right: ";
    if (node->right != nullptr) {
        os << node->right->nodeNum;
    } else {
        os << "NULL";
    }
    os << "\n";

    if (node->left != nullptr) {
        printNode(node->left, os);
    }

    if (node->right != nullptr) {
        printNode(node->right, os);
    }
}
