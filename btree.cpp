#pragma warning(disable:4996)
#include <iostream>
#include <vector>
#include <cstdio>
using namespace std;

// ����
// fwrite(�����ּ�, ���Ͽ� �� byte ��, ���Ͽ� �� ������ ����, ���� ������)

// Leaf node�� ���� ������
class DataEntry {
public:
	int key;
	int value;

	// ������
	DataEntry(int key, int value) {
		this->key = key;
		this->value = value;
	}
};

// Non leaf node�� ���� ������
class IndexEntry {
public:
	int key;
	int BID;

	// ������
	IndexEntry(int key, int BID) {
		this->key = key;
		this->BID = BID;
	}
};

class Header {
public:
	int blockSize;	// ��� �ϳ��� ������
	int rootBID;	// ��Ʈ�� BID
	int depth;		// Ʈ���� depth

	// ������
	Header(int blockSize, int rootBID, int depth) {
		this->blockSize = blockSize;
		this->rootBID = rootBID;
		this->depth = depth;
	}
};

class LeafNode {
public:
	vector<DataEntry> dataEntries;	// leaf node�� ������ �ִ� data entry
	int nextLeafNode;				// ���� leaf node�� ����Ŵ

	// ������
	LeafNode() {
		nextLeafNode = 0;
	}
};

class NonLeafNode {
	int NextLevelBID;				 // ���� ������ ����Ŵ
	vector<IndexEntry> indexEntries; // non leaf node�� ������ �ִ� index entry

	NonLeafNode() {
		NextLevelBID = 0;
	}
};

class BTree {
public:
	// ��� ����
	const char* fileName;	// ���� �̸�
	int blockSize;			// ��� ������
	FILE* filePointer;

	// ������
	BTree() {}

	// file btree ����
	void creation(const char* fileName, int blockSize) {
		// btree�� ���� �̸��� �ʱ�ȭ�Ǿ� ���� ������ ���� �߻� -> ����
		if (fileName == NULL) {
			cout << "������ ��ã�ҽ��ϴ�." << '\n';
			return;
		}

		this->filePointer = fopen(this->fileName, "wb");

		// ó�� creation�� RootBID�� Depth�� 0 ����
		int RootBID = 0;
		int Depth = 0;

		// ���Ͽ� blockSize, RootBID, Depth ������ write
		fwrite(&blockSize, sizeof(int), 1, filePointer);
		fwrite(&RootBID, sizeof(int), 1, filePointer);
		fwrite(&Depth, sizeof(int), 1, filePointer);

		fclose(filePointer);
	}

	// ����
	bool insert(int key, int rid) {

	}

	// ���
	void print() {

	}

	// point search
	int* search(int key) {

	}

	// range search
	int* search(int startRange, int endRange) {

	}
};

// input example
// 1. btree.exe		c	btree.bin		36
// 2. btree.exe		i	btree.bin		insert.txt
// 3. btree.exe		s	btree.bin		search.txt		output.txt
// 4. btree.exe		r	btree.bin		rangesearch.txt output.txt
// 5. btree.exe		p	btree.bin		input.txt		output.txt
//		[0]			[1]		[2]			[3]					[4] 
int main(int argc, char* argv[]) {
	char command = argv[1][0];
	BTree* myBTree = new BTree();

	switch (command) {
	case 'c':
		// create index file
		int blockSize = atoi(argv[3]);
		const char* fileName = argv[2];

		myBTree->creation(fileName, blockSize);
		break;

	case 'i':
		// insert records from [records data file], ex) records.txt
		break;

	case 's':
		// search keys in [input file] and print results to [output file]
		break;

	case 'r':
		// search keys in [input file] and print results to [output file]
		break;

	case 'p':
		// print B+-tree structure to [output file]
		break;
	}
}