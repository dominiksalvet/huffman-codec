// author: Dominik Salvet
// login: xsalve03
// date: 2022-04-26
// filename: transform.hpp
// summary: Header file of byte transforming helper functions.

#pragma once

#include <vector>
#include <cstdint>

using std::vector;


// transform pixel values to their differences (in situ)
// this algorithm utilizes the properties of two's complement (underflow)
void applyDiffModel(vector<uint8_t> &vec);

// revert the differential model (in situ)
// also uses the two's complement properties (overflow)
void revertDiffModel(vector<uint8_t> &vec);

// apply run-length encoding without explicit tag (MNP-5 Microcom format)
vector<uint8_t> applyRLE(const vector<uint8_t> &vec);

// recover the given RLE-encoded data
vector<uint8_t> revertRLE(const vector<uint8_t> &vec);
