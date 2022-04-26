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
