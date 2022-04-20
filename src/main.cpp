// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-20
// filename: main.cpp
// summary: Simple Huffman codec with multiple options.
//          Assumes 8-bit grayscale format of input data.

#include <iostream>
#include <unistd.h>
#include <fstream>
#include <sstream>

using namespace std;

const string HELP_MESSAGE =
"USAGE:\n"
"  huff_codec [-c] [-m] [-a] -i IFILE -o OFILE -w WIDTH\n"
"  huff_codec -d -i IFILE -o OFILE -w WIDTH | -h\n"
"\n"
"OPTION:\n"
"  -c/-d  perform compression/decompression\n"
"  -m     use model for data preprocessing\n"
"  -a     use adaptive scanning\n"
"  -i/-o  input/output file path\n"
"  -w     used image width\n"
"  -h     show this help\n";

// redirect input to stderr, but also print a help hint
void cerrh(const char *s)
{
    cerr << s << "try 'huff_codec -h' for more information\n";
}

// entry point of program
int main(int argc, char *argv[])
{
    // default setup
    bool compress = true;
    bool use_model = false;
    bool adapt_scan = false;

    string ifp; // input file path
    string ofp; // empty by default constructor
    int32_t img_width = 0;

    // argument processing
    int opt;
    while ((opt = getopt(argc, argv, ":cdmai:o:w:h")) != -1)
    {
        switch (opt)
        {
        case 'c': compress = true; break;
        case 'd': compress = false; break;
        case 'm': use_model = true; break;
        case 'a': adapt_scan = true; break;
        case 'i': ifp = optarg; break;
        case 'o': ofp = optarg; break;
        case 'w': img_width = stoi(optarg); break;
        case 'h':
            cout << HELP_MESSAGE;
            return 0;
            break;
        case ':':
            cerrh("ERROR: missing additional argument\n");
            return 1;
            break;
        case '?':
            cerrh("ERROR: unrecognized option used\n");
            return 2;
            break;
        }
    }

    // mandatory arguments check
    if (ifp.empty())
    {
        cerrh("ERROR: no input file path provided\n");
        return 3;
    }
    if (ofp.empty())
    {
        cerrh("ERROR: no output file path provided\n");
        return 4;
    }
    if (img_width <= 0)
    {
        cerrh("ERROR: invalid or missing image width\n");
        return 5;
    }

    // reading input file
    ifstream ifs(ifp, ios::binary); // input file stream
    if (ifs.fail())
    {
        cerr << "ERROR: given input file does not exist\n";
        return 6;
    }
    stringstream buf; buf << ifs.rdbuf();
    string ifc = buf.str(); // input file contents

    // derive image height
    if ((ifc.length() % img_width) != 0)
    {
        cerr << "ERROR: unable to determine integer image height\n";
        return 7;
    }
    int32_t img_height = ifc.length() / img_width;
}
