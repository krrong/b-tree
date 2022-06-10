#pragma warning(disable:4996)
#include <iostream>
#include <vector>
#include <cstdio>
#include <string>
using namespace std;

// 참고
// fwrite(시작주소, 파일에 쓸 byte 수, 파일에 쓸 데이터 개수, 파일 포인터)

// Leaf node에 들어가는 데이터 --> 8byte
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

// Non leaf node에 들어가는 데이터 --> 8byte
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

// 12byte
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

	// 기본 생성자
	Header() {
		blockSize = 0;
		rootBID = 0;
		depth = 0;
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
	int NextLevelBID;				 // 다음 레벨을 가리킴
	vector<IndexEntry> indexEntries; // non leaf node가 가지고 있는 index entry

	NonLeafNode() {
		NextLevelBID = 0;
	}
};

class BTree {
public:
	// 멤버 변수
	Header header;			// Header 내용 저장
	const char* fileName;	// 파일 이름
	int blockSize;			// 블록 사이즈

	// 생성자
	BTree(const char* fileName) {
		this->fileName = fileName;
		setHeader();
	}

	// creation을 제외한 command는 btree.bin을 이용하여 btree를 초기화 함
	// --> btree.bin으로부터 btree를 만드는 과정도 필요
	void setHeader() {
		FILE* fp = fopen(this->fileName, "rb");

		// blockSize, rootBID, depth
		int buffer[3];

		fread(buffer, sizeof(int), 3, fp);

		this->header.blockSize = buffer[0];
		this->header.rootBID = buffer[1];
		this->header.depth = buffer[2];

		cout << this->header.blockSize << endl;
		cout << this->header.rootBID << endl;
		cout << this->header.depth<< endl;

		fclose(fp);
	}

	// file btree 생성
	void creation(const char* fileName, int blockSize) {
		// btree에 파일 이름이 초기화되어 있지 않으면 오류 발생 -> 종료
		if (fileName == NULL) {
			cout << "파일을 못찾았습니다." << '\n';
			return;
		}

		FILE* filePointer = fopen(this->fileName, "wb");
		if (filePointer == NULL) {
			cout << "File Error from creation" << '\n';
			exit(1);
		}

		// 처음 creation시 RootBID와 Depth는 0 고정
		int RootBID = 0;
		int Depth = 0;

		// 파일에 blockSize, RootBID, Depth 순서로 write
		fwrite(&blockSize, sizeof(int), 1, filePointer);
		fwrite(&RootBID, sizeof(int), 1, filePointer);
		fwrite(&Depth, sizeof(int), 1, filePointer);

		//// btree header에 읽어온 정보 저장
		//this->header.blockSize = blockSize;
		//this->header.rootBID = RootBID;
		//this->header.depth = Depth;

		fclose(filePointer);
	}

	// 삽입
	bool insert(int key, int rid) {
		// 트리가 비어있으면 루트노드 생성

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

	// BID를 가진 Block의 Offset 리턴 (BID block의 시작 위치)
	int getBlockOffset(int BID) {
		return 12 + ((BID - 1) * this->blockSize);
	}

	// 파일에 데이터 write
	void writeData() {

	}
};

vector<int> readFile(string readFileName, char command) {
	FILE* filePointer = fopen(readFileName.c_str(), "r");
	if (filePointer == NULL) {
		cout << "File Error from readFile" << '\n';
		exit(1);
	}

	char str[100];
	string preStr;	// 마지막 행이 반복되는 것 방지위함
	vector<int> result;	

	// point search는 한 라인에 하나의 숫자가 있어 분할 필요 X
	if (command == 's') {
		while (feof(filePointer) == 0) {
			fgets(str, 100, filePointer);

			// 개행문자 제거
			if (str[strlen(str) - 1] == '\n') {
				str[strlen(str) - 1] = '\0';
			}

			// 입력이 없으면 종료
			if (str[0] == '\0') {
				break;
			}

			// 읽은 라인을 벡터에 저장
			result.push_back(stoi(string(str)));	// string -> int
		}
	}
	// range search와 insertion은 한 라인에 두 개의 숫자가 있어 분할 필요
	else {
		// 파일 포인터가 파일의 끝이 아닐 때 계속 반복
		while (feof(filePointer) == 0) {
			fgets(str, 100, filePointer);

			// 개행문자 제거
			if (str[strlen(str) - 1] == '\n') {
				str[strlen(str) - 1] = '\0';
			}

			char* context = NULL;	// 분리된 후 남은 문자열이 들어감
			char* token = strtok_s(str, ", ", &context);

			// 마지막 행이 반복되는 것 방지위함
			if (preStr == token) {
				break;
			}
			preStr = token;

			// 읽은 라인을 토큰으로 나누어 벡터에 저장
			while (token != NULL) {
				result.push_back(stoi(string(token)));	// string -> int
				token = strtok_s(NULL, ", ", &context);
			}
		}
	}

	fclose(filePointer);
	return result;
}

// input example
// 1. btree.exe		c	btree.bin		36
// 2. btree.exe		i	btree.bin		insert.txt
// 3. btree.exe		s	btree.bin		search.txt		output.txt
// 4. btree.exe		r	btree.bin		rangesearch.txt output.txt
// 5. btree.exe		p	btree.bin		input.txt		output.txt
//	[0]		[1]		[2]		[3]		[4] 
int main(int argc, char* argv[]) {
	char command = argv[1][0];
	const char* fileName = argv[2];
	vector<int> data;
	string readFileName = "";

	int blockSize = 0;
	BTree* myBTree = new BTree(fileName);

	switch (command) {
	case 'c':
		// create index file
		blockSize = stoi(string(argv[3]));	// argv로부터 blockSize 가져오기
		myBTree->creation(fileName, blockSize);
		break;
	case 'i':
		// insert records from [records data file], ex) records.txt
		readFileName = argv[3];
		data = readFile(readFileName, command);

		// 확인
		for (int i = 0; i < data.size(); i = i + 2) {
			cout << data[i] << ", " << data[i + 1] << endl;
			// myBTree->insert(data[i], data[i+1]);
		}
		break;

	case 's':
		// search keys in [input file] and print results to [output file]
		readFileName = argv[3];
		data = readFile(readFileName, command);

		// 확인
		for (int i = 0; i < data.size(); i = i++) {
			cout << data[i] << endl;
		}
		break;

	case 'r':
		// search keys in [input file] and print results to [output file]
		readFileName = argv[3];
		data = readFile(readFileName, command);

		// 확인
		for (int i = 0; i < data.size(); i = i + 2) {
			cout << data[i] << ", " << data[i + 1] << endl;
		}
		break;

	case 'p':
		// print B+-tree structure to [output file]
		break;
	}
}