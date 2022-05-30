#pragma warning(disable:4996)
#include <iostream>
#include <vector>
#include <cstdio>
using namespace std;

// 참고
// fwrite(시작주소, 파일에 쓸 byte 수, 파일에 쓸 데이터 개수, 파일 포인터)

class DataEntry {
public:
	int key;
	int value;

	// 생성자
	DataEntry(int key, int value) {
		this->key = key;
		this->value = value;
	}
};

class IndexEntry {
public:
	int key;
	int BID;

	// 생성자
	IndexEntry(int key, int BID) {
		this->key = key;
		this->BID = BID;
	}
};

class Header {
public:
	int blockSize;	// 블록 하나의 사이즈
	int rootBID;	// 루트의 BID
	int depth;		// 트리의 depth

	// 생성자
	Header(int blockSize, int rootBID, int depth) {
		this->blockSize = blockSize;
		this->rootBID = rootBID;
		this->depth = depth;
	}
};

class LeafNode {
public:
	vector<DataEntry> dataEntries;	// leaf node가 가지고 있는 data entry
	int nextLeafNode;				// 다음 leaf node를 가리킴

	// 생성자
	LeafNode() {
		nextLeafNode = 0;
	}
};

class NonLeafNode {
	int BID;						 // 다음 레벨을 가리킴
	vector<IndexEntry> indexEntries; // non leaf node가 가지고 있는 index entry

	NonLeafNode() {
		BID = 0;
	}
};

class BTree {
public:
	// 멤버 변수
	const char* fileName;	// 파일 이름
	int blockSize;			// 블록 사이즈
	//int blockCount;			// 블록 개수
	FILE* filePointer;

	// 생성자
	BTree(const char* fileName, int blockSize) {
		this->fileName = fileName;
		this->blockSize = blockSize;
	}

	// file btree 생성
	void creation() {
		// btree에 파일 이름이 초기화되어 있지 않으면 오류 발생 -> 종료
		if (this->fileName == NULL) {
			cout << "파일 이름이 잘못되었습니다." << '\n';
			return;
		}
		filePointer = fopen(this->fileName, "wb");

		// 처음 creation시 RootBID와 Depth는 0 고정
		int RootBID = 0;
		int Depth = 0;
		const char* fileName = this->fileName;

		// 파일에 blockSize, RootBIT, Depth 순서로 write
		fwrite(&blockSize, sizeof(int), 1, filePointer);
		fwrite(&RootBID, sizeof(int), 1, filePointer);
		fwrite(&Depth, sizeof(int), 1, filePointer);

		fclose(filePointer);
	}

	// 삽입
	bool insert(int key, int rid) {

	}

	// 출력
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
	const char* fileName = argv[2];
	int blockSize = atoi(argv[3]);

	BTree* myBTree = new BTree(fileName, blockSize);

	switch (command) {
	case 'c':
		// create index file
		myBTree->creation();
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