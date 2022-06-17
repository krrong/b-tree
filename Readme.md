## Environment
1. C++
2. Input data type : integers(4 bytes each)

## Specifications
- Your B+-tree should be stored in a single binary file, e.g., “btree.bin”
- Do not load a whole B+-tree index in the main memory.
- A node is corresponding to a block. Thus, the size of each node exactly one block.
- Each block is identified by its BID (Block ID), which represented as also a 4-byte integer.
- BID starts from 1, and 0 indicates a NULL block.
- Physical offset of a block in the B+-Tree binary file is calculated in the following way: 12 + ((BID-1) * BlockSize)
- BlockSize represents the physical size of a B+-Tree node, e.g., 1024 bytes.
- The number of entries per node is calculated as follows: (BlockSize – 4) / 8

## Entry Structure
1. Index Entry (Non-leaf)  
<Key, NextLevelBID> (8bytes)  
Key : an integer  
NextLevelBID : a right child node's BID in the B+-Tree binary file.


2. Data entry (Leaf)  
<Key, Value> (8 bytes)  
Key: an integer  
Value: an integer

## Node Structure
1. Non-leaf node  
<NextLevelBID, Index entry, Index entry, ...>  
NextLevelBID : a left child node's in the B+-Tree binary file.  
Index entry : as described above (II.2.A)

2. Leaf node  
<Data entry, Data entry,…,Data entry, NextBID>  
Date entry: as described above (II.2.B)  
NextBID: the BID of the next leaf node in the B+-Tree binary file.

##  B+-Tree binary file structure
1. File header  
<BlockSize, RootBID, Depth> (12 bytes)  
BlockSize: the physical size of a B+-Tree node, which represented as an integer.  
RootBID: the root node’s BID in the B+-Tree binary file.  
Depth: the depth of the B+-Tree. 
By using this variable, we can check whether a node is leaf or not.

2. The rest part of the file stores all the nodes in the B+-Tree.

## Operation to be implement
### 1. Insertion
btree.exe c [btree binary file] [block_size]
e.g., btree.exe c Btree.bin 1028


### 2. Point search
btree.exe i [btree binary file] [records data text file]  
e.g., btree.exe i Btree.bin records.txt  
Included “record.txt” file has the following format:
![](2022-06-17-21-34-52.png)

### 3. Range search
btree.exe s [btree binary file] [input text file] [output text file]  
e.g., btree.exe s Btree.bin search.txt result.txt  
“search.txt” has the following format : 
![](2022-06-17-21-36-01.png)  


The result of search operation, “result.txt” should have “Keys” in the following format : 
<img src="2022-06-17-21-36-23.png">


### 4. Print B+-Tree (print the root node and its child nodes only, i.e., top-2 levels only)
btree.exe r [btree binary file] [input text file] [output text file]  
e.g., btree.exe r Btree.bin range_search.txt range_result.txt  
“range_search.txt” has the following format : 
![](2022-06-17-21-36-43.png)  


The result of range search operation, “range_result.txt” should have “Keys” in the following format:  
![](2022-06-17-21-36-53.png)  


The result of range search operation should include the beginning and the end of the range. 