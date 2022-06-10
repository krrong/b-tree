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
	vector<DataEntry*> dataEntries;	// leaf node가 가지고 있는 data entry
	int nextLeafNode;				// 다음 leaf node를 가리킴

	// 생성자
	LeafNode() {
		nextLeafNode = 0;
	}
};

class NonLeafNode {
public:
	int NextLevelBID;				 // 다음 레벨을 가리킴
	vector<IndexEntry*> indexEntries; // non leaf node가 가지고 있는 index entry

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
		readHeader();
	}

	// creation을 제외한 command는 btree.bin을 이용하여 btree를 초기화 함
	// --> btree.bin으로부터 btree를 만드는 과정도 필요
	void readHeader() {
		FILE* fp = fopen(this->fileName, "r+b");	// 파일을 binary 형태로 읽고 쓸 수 있도록 open

		// blockSize, rootBID, depth
		int buffer[3];

		fread(buffer, sizeof(int), 3, fp);

		this->header.blockSize = buffer[0];
		this->header.rootBID = buffer[1];
		this->header.depth = buffer[2];

		fclose(fp);
	}

	// 인자로 받은 rootBID와 depth를 파일에 다시 작성
	void setHeader(int rootBID, int depth) {
		FILE* fp = fopen(this->fileName, "r+b");	// 파일을 binary 형태로 읽고 쓸 수 있도록 open

		// rootBID가 있는 곳으로 커서 이동
		fseek(fp, sizeof(int), SEEK_SET);

		// 
		fwrite(&rootBID, sizeof(int), 1, fp);
		fwrite(&depth, sizeof(int), 1, fp);

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

		fclose(filePointer);
	}

	// 삽입
	bool insert(int key, int rid) {
		// 트리가 비어있으면 루트노드 생성
		if (this->header.rootBID == 0) {
			this->header.rootBID = 1;	// 새로 생성 시 루트 BID = 1
			makeNewNode(this->header.rootBID);
			setHeader(this->header.rootBID, this->header.depth);
		}
		
		writeData(this->header.rootBID, key, rid);

		return true;
	}

	// Key를 가지고 있는 block search
	int searchBlock(int searchKey) {
		int curBID = this->header.rootBID;
		int totalDepth = this->header.depth;
		int curDepth = 0;

		// 루트부터 key를 가지고 있는 노드 탐색
		while (totalDepth != curDepth) {
			NonLeafNode* curNonLeafNode = getNonLeafNode(curBID);	// BID를 가지고 NonLeafNode 읽어옴
			bool searchFlag = false;

			// NonLeafNode : [nextBID, (key, value), (key, value)...]
			for (int i = 0; i < curNonLeafNode->indexEntries.size(); i++) {
				int nextBID = 0;
				int nextKey = 0;

				// i == 0인 경우 nextBID는 NonLeafNode의 NextLevelBID
				if (i == 0) {
					nextBID = curNonLeafNode->NextLevelBID;
					nextKey = curNonLeafNode->indexEntries.front()->key;
				}
				// i != 0인 경우 nextBID는 NonLeafNode->indexEntries[i-1]->BID
				else {
					nextBID = curNonLeafNode->indexEntries[i - 1]->BID;
					nextKey = curNonLeafNode->indexEntries[i]->key;
				}

				// searchKey가 더 작으면 왼쪽 BID 사용 & 현재 노드 탐색 종료
				if (searchKey < nextKey) {
					curBID = nextBID;
					curDepth++;
					searchFlag = true;
					break;
				}
				// searchKey가 더 크면 오른쪽으로 이동
			}
			// 현재 노드에서 못찾았으면 가장 우측의 BID로 이동
			if (searchFlag == false) {
				curBID = curNonLeafNode->indexEntries.back()->BID;
				curDepth++;
			}
		}

		return curBID;
	}

	// 파일에 데이터 write
	void writeData(int BID, int key, int value) {
		FILE* fp = fopen(this->fileName, "r+b");

		// 처음부터 block의 시작 위치까지 커서 이동
		int blockOffset = getBlockOffset(BID);
		fseek(fp, blockOffset, SEEK_SET);

		int k = key;
		int v = value;

		fwrite(&k, sizeof(int), 1, fp);
		fwrite(&v, sizeof(int), 1, fp);

		fclose(fp);
	}

	// Node에 Entry는 (blockSize - 4) / 8 개 만큼 들어갈 수 있음
	// true면 split 필요
	bool isLeafNodeFull(LeafNode* leafNode) {
		return leafNode->dataEntries.size() > (this->blockSize - 4) / 8;
	}

	// Node에 Entry는 (blockSize - 4) / 8 개 만큼 들어갈 수 있음
	// true면 split 필요
	bool isNonLeafNodeFull(NonLeafNode* nonLeafNode) {
		return nonLeafNode->indexEntries.size() > (this->blockSize - 4) / 8;
	}

	// [(key, value), (key, value), ..., nextBID]
	// BID의 blockOffset에서부터 LeafNode Size만큼 읽어옴
	LeafNode* getLeafNode(int BID) {
		int bufferSize = getNodeSize();		
		int* buffer = new int[bufferSize]();	// 0으로 초기화
		int blockOffset = getBlockOffset(BID);

		FILE* fp = fopen(this->fileName, "rb");

		// 처음부터 blockOffset만큼 커서 이동
		fseek(fp, blockOffset, SEEK_SET);

		// bufferSize만큼 읽어옴
		fread(buffer, sizeof(int), bufferSize, fp);

		LeafNode* leafNode = new LeafNode();
		
		for (int i = 0; i < bufferSize - 1; i=i+2) {
			// buffer에 0이 있으면 데이터가 없는 것

			if (buffer[i] != 0) {
				DataEntry* dataEntry = new DataEntry(buffer[i], buffer[i + 1]);
				leafNode->dataEntries.push_back(dataEntry);
			}
		}
		leafNode->nextLeafNode = buffer[bufferSize - 1];

		fclose(fp);

		delete[] buffer;
		return leafNode;
	}

	// [nextBID, (key, value), (key, value), ...]
	// BID의 blockOffset에서부터 LeafNode Size만큼 읽어옴
	NonLeafNode* getNonLeafNode(int BID) {
		int bufferSize = getNodeSize();
		int* buffer = new int[bufferSize]();	// 0으로 초기화
		int blockOffset = getBlockOffset(BID);

		FILE* fp = fopen(this->fileName, "rb");

		// 처음부터 blockOffset만큼 커서 이동
		fseek(fp, blockOffset, SEEK_SET);

		// bufferSize만큼 읽어옴
		fread(buffer, sizeof(int), bufferSize, fp);

		NonLeafNode* nonLeafNode = new NonLeafNode();
		for (int i = 1; i < bufferSize; i = i + 2) {
			// buffer에 0이 있으면 데이터가 없는 것

			if (buffer[i] != 0) {
				IndexEntry* indexEntry = new IndexEntry(buffer[i], buffer[i + 1]);
				nonLeafNode->indexEntries.push_back(indexEntry);
			}
		}
		nonLeafNode->NextLevelBID = buffer[0];

		fclose(fp);

		delete[] buffer;
		return nonLeafNode;
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

	// blockID 가 BID인 Block의 시작 위치 리턴
	int getBlockOffset(int BID) {
		return 12 + ((BID - 1) * this->blockSize);
	}

	// 새로운 노드 생성 -> 파일의 지정된 위치에 nodeSize만큼을 0으로 write
	void makeNewNode(int blockBID) {
		FILE* fp = fopen(this->fileName, "ab");

		int blockOffset = getBlockOffset(blockBID);
		
		// 파일의 처음부터 blockOffset만큼 커서 이동
		fseek(fp, blockOffset, SEEK_SET);
		
		int bufferByte = getNodeSize();
		int* buffer = new int[bufferByte] ();		// nodeSize만큼 동적할당 -> 파일에 0으로 쓰기 위함

		// blockOffset위치부터 nodeSize만큼 0으로 write
		fwrite(buffer, sizeof(int), bufferByte, fp);

		delete[] buffer;
		fclose(fp);
	}

	// nodeSize 리턴 : blockSize를 4로 나누어 리턴
	int getNodeSize() {
		return this->header.blockSize / 4;
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
		//readFileName = argv[3];
		//data = readFile(readFileName, command);

		//// 확인
		//for (int i = 0; i < data.size(); i = i + 2) {
		//	cout << data[i] << ", " << data[i + 1] << endl;
		//	// myBTree->insert(data[i], data[i+1]);
		//}

		myBTree->insert(1,2);
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