# author: Dominik Salvet
# login: xsalve03
# date: 2022-04-20
# filename: Makefile
# summary: Build system for Huffman codec.

SRC_DIR = src
SRC_FILES = $(SRC_DIR)/main.cpp\
            $(SRC_DIR)/huff.cpp
HEADER_FILES = $(SRC_DIR)/huff.hpp

all: huff_codec

huff_codec: $(SRC_FILES) $(HEADER_FILES)
	g++ -Wall -o $@ $(SRC_FILES)

clean:
	rm -f huff_codec a.out
