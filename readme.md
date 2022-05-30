# Huffman Codec (coder/decoder)

This program called `huffman-codec` performs adaptive Huffman encoding and decoding of given files. It is a lossless compression method, and its main target within this assignment are `RAW` grayscale images, yet it may be used with any type of file. This implementation contains some features, which may improve Huffman coding efficiency in general. It also has features to boost compression factors for data with matrix characteristics (which images have). When compressing, these additional features, together with the Huffman encoding itself, are executed one by one in pipeline. When uncompressing, we proceed in the reversed order.

> This program was created within a course of my master's studies.

## Implementation Details

First, the program parses command line arguments. To be as user-frienly as possible, many are optional. See the `huffman-codec -h` output below.

```
USAGE:
  huffman-codec [-cm] -i IFILE [-o OFILE]
  huffman-codec [-cm] -a [-w WIDTH] -i IFILE [-o OFILE]
  huffman-codec -d -i IFILE [-o OFILE] | -h

OPTION:
  -c/-d  perform compression/decompression
  -m     use differential model for preprocessing
  -a     use adaptive block RLE (default: RLE)
  -w     width of 2D data (default: 512)
  -i     input file path
  -o     output file path (default: b.out)
  -h     show this help
```

The program uses `getopt()` function for parsing those. There are default values for some options, so the program will not end up with an error if the user does not set them explicitly. For example, the program performs compression of a file in default.

Internally, the compression as well as decompression is broken down to individual steps, which are described below. Some are optional, some are always used. Basically, the following graph summarizes it.

`input -> [differential model] -> RLE | adaptive block RLE -> Huffman coding -> output`

### Differential Model

* `transform.cpp`

The differential model is a very simple model for transforming adjacent pixels into their differences. It is very useful for smooth transitions in the input data, which are transformed into several identical bytes. This implementation utilizes the properties of two's complement to simplify the implementation. The transformation is performed in situ.

### Run-Length Encoding (RLE)

* `transform.cpp`

After the differential model, some form of RLE is always applied. This RLE works basically on the data stream basis. It is very useful when input data or the result of differential model have repeating identical bytes. Due to following processing in Huffman coding, it was required to choose the RLE format, which works on byte resolution not to shit byte patterns. Hence, MNP-5 Microcom format has been deployed.

### Adaptive Block RLE

* `transform.cpp, headers.cpp`

This method should be used when input data have matrix properties. It will break the matrix into several blocks, and performs either horizontal RLE, or vertical RLE, based on better compression factor. Hence it must also store a bit of direction for each block in its output. For these purpose, there is an adaptive block header, where is stored following: `<64b-matrix-width><64b-matrix-height><64b-block-size><block-scan-dirs>`. This header is present in the data only when this method is used and it is also a subject to Huffman encoding.

This implementation finds optimal block size with the best compression factor automatically, hence it is also present in the header (see above). Also, it supports arbitrary matrix sizes (they do not have to be divisible by block size).

### Huffman Coding

* `huffman.cpp, headers.cpp`

This is the main part of the program. All previous methods can be consider preprocessing to this, so that the data has better properties to compress it effectively using the Huffman coding. It is implemented as a Huffman tree (see `HuffTree` class in the code) and it uses Huffman nodes (see `HuffNode` struct in the code).

As this method is adaptive, the Huffman tree is built during compression as well as during decompression (they build identical tree). For this approach, the FGK algorithm is used.

When decompressing, we also need to know total bytes to decode. So, there is also a Huffman header added into the stream. It has the following format" `<64b-byte-count><8b-flags>`. Flags include information whether differential mode and adaptive RLE were used, so that the program knows that when decompressing a file.

## Compilation

A `Makefile` is provided for easier compilation of the program. Use `make` in the root directory to compile it. The final binary will be created as `huffman-codec` and it is prepared to be used (see help above). Also, `make clean` is supported for cleaning temporary files.

## Measured Performance

The performance analysis of given samples (see the `data` directory) was performed on our faculty server. For simplicity, compression algorithm was applied only once for each file (performing it twice or more, we can get better compression factor). The measurement is presented in the table below.

| File name      | Static without model | Static with model | Adaptive without model | Adaptive with model |
|----------------|----------------------|-------------------|------------------------|---------------------|
| df1h.raw       | 8,01bpc 3,22s        | 0,01bpc 0,03s     | 0,11bpc 0,38s          | 0,02bpc 0,39s       |
| df1hvx.raw     | 2,44bpc 0,50s        | 1,02bpc 0,22s     | 1,66bpc 0,59s          | 0,51bpc 0,47s       |
| df1v.raw       | 0,11bpc 0,08s        | 0,02bpc 0,04s     | 0,12bpc 0,44s          | 0,02bpc 0,40s       |
| hd01.raw       | 3,06bpc 1,32s        | 2,68bpc 0,86s     | 3,03bpc 1,56s          | 2,68bpc 1,06s       |
| hd02.raw       | 2,91bpc 1,08s        | 2,64bpc 0,88s     | 2,89bpc 1,61s          | 2,64bpc 1,02s       |
| hd07.raw       | 4,81bpc 1,55s        | 3,34bpc 0,62s     | 4,78bpc 1,71s          | 3,32bpc 0,85s       |
| hd08.raw       | 3,47bpc 0,76s        | 3,01bpc 0,70s     | 3,42bpc 1,05s          | 3,01bpc 0,95s       |
| hd09.raw       | 6,65bpc 2,28s        | 4,65bpc 0,94s     | 6,58bpc 2,71s          | 4,63bpc 1,14s       |
| hd12.raw       | 5,43bpc 1,98s        | 3,68bpc 0,94s     | 5,38bpc 2,33s          | 3,83bpc 1,45s       |
| nk01.raw       | 6,48bpc 1,86s        | 6,05bpc 1,28s     | 6,48bpc 2,03s          | 6,05bpc 1,65s       |
| *Average*      | 4,34bpc 1,46s        | 2,71bpc 0,65s     | 3,45bpc 1,44s          | 2,67bpc 0,94s       |

> The shorthand `bpc` means bits per character. Character is equivalent to byte here.

Also, the base `RAW` file sample set was extended with custom `RAW` files to test particular features. The `hd01double.raw` contains two `hd01.raw` in vertical (to test different width and height values). The `hd01extra.raw` contains `hd01.raw` and five pixel lines below it (to test image size, which is not divisible with RLE block size). These files are also included in the `data` directory. Their results are in the following table.

| File name      | Static without model | Static with model | Adaptive without model | Adaptive with model |
|----------------|----------------------|-------------------|------------------------|---------------------|
| hd01double.raw | 3,05bpc 2,19s        | 2,68bpc 1,41s     | 3,03bpc 3,05s          | 2,67bpc 2,19s       |
| hd01extra.raw  | 3,03bpc 1,30s        | 2,66bpc 0,72s     | 3,00bpc 1,41s          | 2,65bpc 1,13s       |

## Sources

Most of my sources came from the Internet and they were too many that it is impossible to list some particular. Some highligted websites include Stack Overflow, online CPP reference, and GitHub. I have also used some knowledge from my old personal projects.
