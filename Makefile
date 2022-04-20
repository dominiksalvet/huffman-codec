# author: Dominik Salvet
# login: xsalve03
# date: 2022-04-20
# filename: Makefile
# summary: Build system for Huffman codec.

SRC_DIR = src
SRC_FILES = $(SRC_DIR)/main.cpp

all: huff_codec

huff_codec: $(SRC_FILES)
	g++ -Wall -o $@ $^

clean:
	rm -f huff_codec
