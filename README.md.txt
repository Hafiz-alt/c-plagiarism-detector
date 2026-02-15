# 🔍 Advanced Code Plagiarism Detector in C

A robust, multi-algorithm code plagiarism detection system built entirely in C. This tool analyzes source code files and calculates their similarity to detect potential copying, even when attempts are made to hide it by renaming variables or changing loop structures (e.g., swapping `for` for `while`).

## 🚀 Overview

Basic text-comparison tools fail when a plagiarist modifies identifiers or slightly alters the code flow. This project solves that by implementing a custom **Lexical Tokenizer** that strips away superficial differences and compares the underlying structural and algorithmic logic using Dynamic Programming.

## 🧠 Algorithms & Technical Concepts Applied

This project serves as a practical application of Design and Analysis of Algorithms (DAA) concepts:

* **Longest Common Subsequence (LCS):** Implemented using Dynamic Programming to find the longest continuous sequence of matching tokens between two files.
* **Edit Distance (Levenshtein Distance):** Calculates the minimum number of operations required to transform one token sequence into another.
* **N-gram Similarity (3-grams):** Breaks code into smaller sliding windows to catch localized copying.
* **Vector Dot Product (Frequency Similarity):** Compares the mathematical frequency of keywords and operators used across both files.
* **Manual Memory Management:** Extensive use of `malloc`, `calloc`, and `free` to safely handle dynamic multi-dimensional arrays for the DP matrices.

## ⚙️ How to Compile and Run

1. Clone the repository:
   ```bash
   git clone [https://github.com/YOUR-USERNAME/c-plagiarism-detector.git](https://github.com/YOUR-USERNAME/c-plagiarism-detector.git)
   cd c-plagiarism-detector