#-------------------------------------------------------------------------------
# Copyright 2022 Dominik Salvet
# https://github.com/dominiksalvet/huffman-codec
#-------------------------------------------------------------------------------
# Build system for Huffman codec.
#-------------------------------------------------------------------------------

SRC_DIR = src
SRC_FILES = $(SRC_DIR)/main.cpp\
            $(SRC_DIR)/huffman.cpp\
            $(SRC_DIR)/transform.cpp\
            $(SRC_DIR)/headers.cpp
HEADER_FILES = $(SRC_DIR)/huffman.hpp\
               $(SRC_DIR)/transform.hpp\
               $(SRC_DIR)/headers.hpp

all: huffman-codec

huffman-codec: $(SRC_FILES) $(HEADER_FILES)
	g++ -Wall -o $@ $(SRC_FILES)

clean:
	rm -f huffman-codec b.out huff raw
