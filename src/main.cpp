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
#include <vector>

using namespace std;

const string HELP_MESSAGE =
"USAGE:\n"
"  huff_codec [-cma] -w WIDTH -i IFILE [-o OFILE]\n"
"  huff_codec -d -i IFILE [-o OFILE] | -h\n"
"\n"
"OPTION:\n"
"  -c/-d  perform compression/decompression\n"
"  -m     use model for image preprocessing\n"
"  -a     use adaptive scanning\n"
"  -i/-o  input/output file path\n"
"  -w     used image width\n"
"  -h     show this help\n";

// redirect input to stderr, but also print a help hint
void cerrh(const char *s)
{
    cerr << s << "try 'huff_codec -h' for more information\n";
}

// transform pixel values to their differences (in situ)
void diff_model(vector<int16_t> &vec)
{
    int16_t prev_val = 0;
    for (size_t i = 0; i < vec.size(); i++)
    {
        int16_t cur_val = vec[i];
        vec[i] = cur_val - prev_val;
        prev_val = cur_val;
    }
}

// compress image based on several given options
vector<uint8_t> compress(
    ifstream& ifs,
    bool use_model,
    bool adapt_scan,
    int32_t img_width)
{
    // load input file to internal representation vector
    vector<int16_t> idata; // 16 bits for sure
    int c;
    while ((c = ifs.get()) != EOF)
        idata.push_back(c); // implicit conversion
    ifs.close();

    // derive image height
    if ((idata.size() % img_width) != 0)
    {
        cerr << "ERROR: unable to determine integer image height\n";
        exit(6);
    }
    int32_t img_height = idata.size() / img_width;

    if (use_model)
        diff_model(idata);

    // TODO: return real data
    vector<uint8_t> a;
    for (int16_t item : idata)
        a.push_back(abs(item));
    return a;
}

// decompress image of the given input stream
vector<uint8_t> decompress(ifstream &ifs)
{
    // load input file to internal representation vector
    vector<uint8_t> idata; // byte by byte
    int c;
    while ((c = ifs.get()) != EOF)
        idata.push_back(c);
    ifs.close();

    return idata; // TODO: return real data
}

// write final data from vector to given output file path
void write_odata(vector<uint8_t> &vec, const string &ofp)
{
    ofstream ofs(ofp, ios::out | ios::binary); // output file stream
    if (ofs.fail())
    {
        cerr << "ERROR: cannot write to " << ofp << " output file\n";
        exit(7);
    }

    ofs.write((char *) vec.data(), vec.size());
    ofs.close();
}

// entry point of program
int main(int argc, char *argv[])
{
    // default setup
    bool use_compr = true;
    bool use_model = false;
    bool adapt_scan = false;

    string ifp; // input file path (empty by default constructor)
    string ofp = "a.out"; // some default path
    int32_t img_width = 0;

    // argument processing
    int opt;
    while ((opt = getopt(argc, argv, ":cdmai:o:w:h")) != -1)
    {
        switch (opt)
        {
        case 'c': use_compr = true; break;
        case 'd': use_compr = false; break;
        case 'm': use_model = true; break;
        case 'a': adapt_scan = true; break;
        case 'i': ifp = optarg; break;
        case 'o': ofp = optarg; break;
        case 'w': img_width = stoi(optarg); break;
        case 'h':
            cout << HELP_MESSAGE;
            return 0; break;
        case ':':
            cerrh("ERROR: missing additional argument\n");
            return 1; break;
        case '?':
            cerrh("ERROR: unrecognized option used\n");
            return 2; break;
        }
    }

    // mandatory arguments check
    if (ifp.empty())
    {
        cerrh("ERROR: no input file path provided\n");
        return 3;
    }
    if (use_compr && img_width <= 0)
    {
        cerrh("ERROR: invalid or missing image width\n");
        return 4;
    }

    // reading input file
    ifstream ifs(ifp, ios::in | ios::binary); // input file stream
    if (ifs.fail())
    {
        cerr << "ERROR: given input file does not exist\n";
        return 5;
    }

    // perform required operation
    vector<uint8_t> odata; // alway array of bytes
    if (use_compr)
        odata = compress(ifs, use_model, adapt_scan, img_width);
    else
        odata = decompress(ifs);
    
    write_odata(odata, ofp);
}
