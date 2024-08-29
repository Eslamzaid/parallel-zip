# Parallel Zip

## Description
This project implements a simple parallel zip utility in C, designed to efficiently compress large files using multi-threading techniques.

## Features
- Uses `open` and `mmap` for handling large files efficiently.
- Divides the file into 4 main chunks, each further divided into 4 inner chunks for parallel processing.
- Utilizes multiple threads (`NUM_OF_CORS`) to process each inner chunk independently.
- Implements a compression algorithm on buffers corresponding to inner chunks.
- Ensures thread synchronization to maintain the correct sequence of compressed data in the output.
- Handles smaller files by directly reading and compressing them.

## File Structure Example (10MB)

                👇 inner chunks
    +-----------------------------------------------------------------------------------------------------+
    | Chunk 1: [0 - 631244]  | [2524980 - 3156224]  | [5049960 - 5681204]  | [7574940 - 8206184]          |  
    | Chunk 2: [631246 - 1262489]  | [3156226 - 3787469]  | [5681206 - 6312449]  | [8206186 - 8837429]    |
    | Chunk 3: [1262491 - 1893734]  | [3787471 - 4418714]  | [6312451 - 6943694]  | [8837431 - 9468674]   |
    | Chunk 4: [1893736 - 2524980]  | [4418716 - 5049960]  | [6943696 - 7574940]  | [9468676 - 10099919]  |
    +-----------------------------------------------------------------------------------------------------+


## Usage
- For large files: Utilizes `open`, `mmap`, and multi-threading to efficiently compress and output data.
- For small files: Directly reads and applies compression algorithms.
