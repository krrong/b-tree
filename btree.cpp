#pragma warning(disable:4996)
#include <iostream>
#include <vector>
#include <stack>
#include <cstdio>
#include <string>
#include <algorithm>
using namespace std;

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

// key를 기준으로 dataEntry 오름차순 정렬 (LeafNode)
bool compLeafNode(DataEntry* a, DataEntry* b) {
	return a->key < b->key;
}

// key 기준으로 indexEntry 오름차순 정렬 (NonLeafNode)
bool compNonLeafNode(IndexEntry* a, IndexEntry* b) {
	return a->key < b->key;
}

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

class BTree {
public:
	// 멤버 변수
	Header header;			// Header 내용 저장
	const char* fileName;	// 파일 이름
	int totalBlockCount;	// Btree의 총 block 개수

	// 생성자
	BTree(const char* fileName) {
		this->fileName = fileName;
		readHeader();
		totalBlockCount = getTotalBlockCount();
	}

	int getTotalBlockCount() {
		FILE* fp = fopen(this->fileName, "r+b");

		if (fp == NULL) {
			return -1;
		}

		// size = 파일의 총 bytes
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);

		fclose(fp);

		// btree.bin 의 blockSize가 0이면 -1 리턴
		if (this->header.blockSize == 0) {
			return -1;
		}

		// 파일의 총 bytes - (12(header) / blockSize) = 총 block의 수
		return (size - 12) / this->header.blockSize;
	}

	// creation을 제외한 command는 btree.bin을 이용하여 btree를 초기화 함
	//  --> btree.bin으로부터 btree를 만드는 과정도 필요
	// btree.bin파일로부터 btree의 header구성
	void readHeader() {
		FILE* fp = fopen(this->fileName, "r+b");	// 파일을 binary 형태로 읽고 쓸 수 있도록 open

		if (fp == NULL) {
			return;
		}

		// blockSize, rootBID, depth
		int buffer[3];

		// 파일을 읽어 header수정
		fread(buffer, sizeof(int), 3, fp);
		this->header.blockSize = buffer[0];
		this->header.rootBID = buffer[1];
		this->header.depth = buffer[2];

		fclose(fp);
	}

	// 인자로 받은 rootBID와 depth를 btree.bin 파일 header 부분에 다시 작성
	void writeHeader(int rootBID, int depth) {
		FILE* fp = fopen(this->fileName, "r+b");	// 파일을 binary 형태로 읽고 쓸 수 있도록 open

		// rootBID가 있는 곳으로 커서 이동
		fseek(fp, sizeof(int), SEEK_SET);

		// rootBID, depth write
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
		// leafNode에 추가 할 dataEntry 생성
		DataEntry* newDataEntry = new DataEntry(key, rid);

		// 트리가 비어있으면 루트노드 생성
  		if (this->header.rootBID == 0) {
			this->header.rootBID = 1;	// 새로 생성 시 루트 BID = 1
			makeNewNode(this->header.rootBID);	// 새로운 노드(루트) 생성
			writeHeader(this->header.rootBID, this->header.depth);	// btree 헤더 내용 수정
		}

		// searchKey를 포함하는 leafNode를 탐색
		stack<int> routeBID = getRoute(key);
		int updateBID = routeBID.top();	// leafNode의 Key는 stack에 top에 존재
		routeBID.pop();

		// leafNode에 새로 생성한 dataEntry추가, key기준 오름차순 정렬->split 필요 확인
   		LeafNode* leafNode = getLeaf(updateBID);
		leafNode->dataEntries.push_back(newDataEntry);
		sort(leafNode->dataEntries.begin(), leafNode->dataEntries.end(), compLeafNode);

		// split필요하지 않은 경우
		if (!isLeafNodeFull(leafNode)) {
			// split이 필요하지 않으면 이미 정렬된 상태이기 때문에 바로 rewrite하고 종료
			rewriteLeafNode(updateBID, leafNode);
			return true;
		}
		else {
			// leafNode가 꽉 차면 leafNode split진행 --> nonLeafNode에 새로운 indexDentry insert 필요
			IndexEntry* newIndexEntry = leafNodeSplit(leafNode, updateBID);

			// 부모 노드도 split이 필요한지 root까지 올라가면서 확인
			while (true) {
				// 부모 노드가 없을경우 부모 생성 후 종료
				if (routeBID.empty()) {
					// nonLeafNode 생성 (루트 노드)
					NonLeafNode* parentNode = new NonLeafNode();

					int parentBID = getTotalBlockCount() + 1;
					makeNewNode(parentBID);
					parentNode->NextLevelBID = updateBID;	// NextLevelBID는 split이후 왼쪽 노드를 가리킴
					parentNode->indexEntries.push_back(newIndexEntry);	// split이후 오른쪽 노드로부터 만들어진 IndexEntry를 추가

					this->header.depth = this->header.depth + 1;	// 부모 노드 추가 시 depth + 1
					this->header.rootBID = parentBID;	// 부모 노드 추가 시 depth + 1
					writeHeader(parentBID, this->header.depth);	// 부모 노드 추가 시 rootNodeBID변경

					// NonLeafNode write후 종료
					rewriteNonLeafNode(parentBID, parentNode);
					break;
				}
				// 부모가 있는 경우 2가지
				// 1. 부모에 자리가 있어서 삽입 가능한 경우
				// 2. 부모에 자리가 없어서 다시 split이 일어나는 경우
				else {
					// 부모노드의 BID
					int parentBID = routeBID.top();
					routeBID.pop();

					// 부모 노드 NonLeafNode에 split하며 생긴 IndexEntry추가 후 정렬
					NonLeafNode* parentNonLeafNode = getNonLeaf(parentBID);
					parentNonLeafNode->indexEntries.push_back(newIndexEntry);
					sort(parentNonLeafNode->indexEntries.begin(), parentNonLeafNode->indexEntries.end(), compNonLeafNode);

					// 1. 부모에 자리가 없어서 다시 split이 일어나는 경우
					if (!isNonLeafNodeFull(parentNonLeafNode)) {
						// 부모 노드를 다시 파일에 write -> 종료(split 전파 X)
						rewriteNonLeafNode(parentBID, parentNonLeafNode);
						break;
					}
					// 2. 부모에 자리가 있어서 삽입 가능한 경우
					else {
						// split 이후 새로 생기는 IndexEntry
						newIndexEntry = nonLeafNodeSplit(parentNonLeafNode, parentBID);

						// 수정할 노드는 부모노드임으로 updateBID 변경
						updateBID = parentBID;
					}
				}
			}
		}
		return true;
	}

	// split이후 부모노드에 추가할 IndexEnrty 반환
	IndexEntry* leafNodeSplit(LeafNode* originLeafNode, int originLeafNodeBID) {
		// 스플릿이 되고나면 새로운 블록 생성 entry가 나누어지고 기존 leafNode가 새로 생긴 leafNode를 가리키는 형태
		// 파일의 가장 끝에 노드가 생성되겠지? --> Btree에서 12 + 블록사이즈 * 블록의 개수 하면 가장 끝의 offset이 나오니까 btree에 total block의 개수를 저장하자
		
		int newBlockID = this->totalBlockCount + 1;	// 총 block 수 증가
		makeNewNode(newBlockID);	// 파일에 새로운 블록 0으로 write
		LeafNode* newLeafNode = new LeafNode();	// split이후 생기는 새로운 leafNode

		newLeafNode->nextLeafNode = originLeafNode->nextLeafNode;	// 원래 node가 가리키던 nextBID를 split 이후 새로 생기는 노드의 nextBID에 넣어줌
		originLeafNode->nextLeafNode = newBlockID;	// split전 노드에서 split 이후 새로 생기는 노드로 연결

		// 왼쪽 : [0 ~ dataEntries.size() / 2)
		// 오른쪽 : [dataEntries.size() / 2 ~ dataEntries.size())
		// 5개라면
		// 왼쪽 0, 1, 2
		// 오른쪽 3, 4
		int left = originLeafNode->dataEntries.size() / 2;

		// 원래 leaf에서 새로 생긴 leaf로 우측 데이터 복사
		for (int i = left; i < originLeafNode->dataEntries.size(); i++) {

			DataEntry* newDataEntry = new DataEntry(originLeafNode->dataEntries[i]->key, originLeafNode->dataEntries[i]->value);
			newLeafNode->dataEntries.push_back(newDataEntry);
		}

		// 복사한 만큼의 데이터 삭제
		originLeafNode->dataEntries.erase(originLeafNode->dataEntries.begin() + left, originLeafNode->dataEntries.end());

		rewriteLeafNode(originLeafNodeBID, originLeafNode);	// 기존 노드 rewrite
		rewriteLeafNode(newBlockID, newLeafNode);	// split 해서 생긴 노드 write

		// split이후 부모노드에 추가할 IndexEnrty 리턴 (우측 노드의 첫 번째 key값, 우측 노드의 BID)
		IndexEntry* newIndexEntry = new IndexEntry(newLeafNode->dataEntries[0]->key, newBlockID);

		return newIndexEntry;
	}

	// split이후 부모노드에 추가할 IndexEntry 반환
	IndexEntry* nonLeafNodeSplit(NonLeafNode* originNonLeafNode, int originNonLeafNodeBID) {
		// 스플릿이 되고나면 새로운 블록 생성 entry가 나누어지고 상위 노드로 IndexEntry 삽입
		// 파일의 가장 끝에 노드가 생성될 것
		
		int newBlockBID = this->totalBlockCount + 1;	// 총 block 수 증가
		makeNewNode(newBlockBID);	// 파일에 새로운 블록 0으로 write
		NonLeafNode* newNonLeafNode = new NonLeafNode();	// split이후 생기는 새로운 nonLeafNode

		int left = originNonLeafNode->indexEntries.size() / 2;
		newNonLeafNode->NextLevelBID = originNonLeafNode->indexEntries[left]->BID;	// split하여 생기는 nonleafnode에 nextLevelBID를 나누어지는 첫 번째 BID로 삽입

		// 원래 nonLeafNode에서 새로 생긴 nonLeafNode로 데이터 복사 (우측 절반 - 1)
		for (int i = left + 1; i < originNonLeafNode->indexEntries.size(); i++) {

			IndexEntry* newIndexEntry = new IndexEntry(originNonLeafNode->indexEntries[i]->key, originNonLeafNode->indexEntries[i]->BID);
			newNonLeafNode->indexEntries.push_back(newIndexEntry);
		}

		// 우측에서 첫 번째 key값과 우측 BID는 부모로 올라가는 IndexEntry
		IndexEntry* newIndexEntry = new IndexEntry(originNonLeafNode->indexEntries[left]->key, newBlockBID);

		// 복사한만큼의 데이터 삭제
		originNonLeafNode->indexEntries.erase(originNonLeafNode->indexEntries.begin() + left, originNonLeafNode->indexEntries.end());

		rewriteNonLeafNode(originNonLeafNodeBID, originNonLeafNode);	// 기존 노드 rewrite
		rewriteNonLeafNode(newBlockBID, newNonLeafNode);	// split 해서 생긴 노드 rewrite

		return newIndexEntry;
	}

	// BID와 LeafNode를 인자로 받아 파일에 rewrite
	void rewriteLeafNode(int BID, LeafNode* leafNode) {
		// BID의 시작 위치 반환
		int blockOffset = getBlockOffset(BID);

		FILE* fp = fopen(this->fileName, "r+b");

		// 처음부터 block의 시작 위치까지 커서 이동
		fseek(fp, blockOffset, SEEK_SET);

		// 파일에 0으로 쓰기 위함
		int bufferByte = getNodeSize();
		int* buffer = new int[bufferByte]();

		// blockOffset위치부터 nodeSize만큼 0으로 write --> rewrite 하기 위한 사전 준비
		fwrite(buffer, sizeof(int), bufferByte, fp);

		// fwrite을 하면 커서가 이동되기 때문에 다시 제위치로 이동
		fseek(fp, blockOffset, SEEK_SET);

		// leafNode의 dataEntry write
		for (int i = 0; i < leafNode->dataEntries.size(); i++) {
			// file에 write
			fwrite(&leafNode->dataEntries[i]->key, sizeof(int), 1, fp);
			fwrite(&leafNode->dataEntries[i]->value, sizeof(int), 1, fp);
		}

		// leafNode의 nextLeafNode 위치로 이동
		fseek(fp, blockOffset + this->header.blockSize - 4, SEEK_SET);

		// leafNode의 nextLeafNode write
		int nextLeafNode = leafNode->nextLeafNode;
		fwrite(&nextLeafNode, sizeof(int), 1, fp);

		fclose(fp);
	}

	// BID와 NonLeafNode를 인자로 받아 파일에 rewrite
	void rewriteNonLeafNode(int BID, NonLeafNode* nonLeafNode) {
		// BID의 시작 위치 반환
		int blockOffset = getBlockOffset(BID);

		FILE* fp = fopen(this->fileName, "r+b");

		// 처음부터 block의 시작 위치까지 커서 이동
		fseek(fp, blockOffset, SEEK_SET);

		// 파일에 0으로 쓰기 위함
		int bufferByte = getNodeSize();
		int* buffer = new int[bufferByte]();

		// blockOffset위치부터 nodeSize만큼 0으로 write --> rewrite 하기 위한 사전 준비
		fwrite(buffer, sizeof(int), bufferByte, fp);
		
		// fwrite을 하면 커서가 이동되기 때문에 다시 제위치로 이동
		fseek(fp, blockOffset, SEEK_SET);

		// nextLevelBID write
		int NextLevelBID = nonLeafNode->NextLevelBID;
		fwrite(&NextLevelBID, sizeof(int), 1, fp);

		// nonLeafNode의 indexEntry write
		for (int i = 0; i < nonLeafNode->indexEntries.size(); i++) {
			// file에 write
			fwrite(&nonLeafNode->indexEntries[i]->key, sizeof(int), 1, fp);
			fwrite(&nonLeafNode->indexEntries[i]->BID, sizeof(int), 1, fp);
		}

		// leafNode의 nextLeafNode 위치로 이동
		fseek(fp, blockOffset + this->header.blockSize - 4, SEEK_SET);

		fclose(fp);
	}

	// searchKey를 통해 block search
	//  --> searchKey 범위를 가지고 있는 block을 찾아가는 route의 BID리턴
	stack<int> getRoute(int searchKey) {
		stack<int> result;
		int curBID = this->header.rootBID;
		int totalDepth = this->header.depth;
		int curDepth = 0;
		result.push(curBID);	// root 노드만 존재할 때 while문을 타지 않음

		// 루트부터 key를 가지고 있는 노드 탐색
		while (totalDepth != curDepth) {
			NonLeafNode* curNonLeafNode = getNonLeaf(curBID);	// BID를 가지고 NonLeafNode 읽어옴
			bool searchFlag = false;

			int size = curNonLeafNode->indexEntries.size();
			// NonLeafNode : [nextBID, (key, value), (key, value)...]
			for (int i = 0; i < size; i++) {
				int nextKey = 0;
				int nextBID = 0;

				// i == 0인 경우 nextBID는 NonLeafNode의 NextLevelBID
				if (i == 0) {
					nextKey = curNonLeafNode->indexEntries[0]->key;
					nextBID = curNonLeafNode->NextLevelBID;
				}
				// i != 0인 경우 nextBID는 NonLeafNode->indexEntries[i-1]->BID
				// i != 0인 경우 nextKey는 NonLeafNode->indexEntries[i]->key
				else {
					nextKey = curNonLeafNode->indexEntries[i]->key;
					nextBID = curNonLeafNode->indexEntries[i - 1]->BID;
				}

				// searchKey가 더 작으면 왼쪽 BID 사용 & 현재 노드 탐색 종료
				if (searchKey < nextKey) {
					curDepth++;
					searchFlag = true;
					curBID = nextBID;
					result.push(curBID);
					break;
				}
				// searchKey가 더 크면 노드에서 오른쪽으로 이동
			}
			// 현재 노드에서 못찾았으면 가장 우측의 BID로 이동
			if (searchFlag == false) {
				curBID = curNonLeafNode->indexEntries.back()->BID;
				curDepth++;
				result.push(curBID);
			}
		}

		return result;
	}


	// Node에 Entry는 (blockSize - 4) / 8 개 만큼 들어갈 수 있음
	// true면 split 필요
	bool isLeafNodeFull(LeafNode* leafNode) {
		return leafNode->dataEntries.size() > ((this->header.blockSize - 4) / 8);
	}

	// Node에 Entry는 (blockSize - 4) / 8 개 만큼 들어갈 수 있음
	// true면 split 필요
	bool isNonLeafNodeFull(NonLeafNode* nonLeafNode) {
		return nonLeafNode->indexEntries.size() > (this->header.blockSize - 4) / 8;
	}

	// [(key, value), (key, value), ..., nextBID]
	// BID의 blockOffset에서부터 LeafNode Size만큼 읽어옴
	LeafNode* getLeaf(int BID) {
		int bufferSize = getNodeSize();		
		int* buffer = new int[bufferSize]();	// 0으로 초기화
		int blockOffset = getBlockOffset(BID);

		FILE* fp = fopen(this->fileName, "rb");

		// 처음부터 blockOffset만큼 커서 이동
		fseek(fp, blockOffset, SEEK_SET);

		// bufferSize만큼 읽어옴
		fread(buffer, sizeof(int), bufferSize, fp);

		LeafNode* leafNode = new LeafNode();
		// 다음 leafNode를 가리키는 BID는 buffer 맨 마지막에 존재함
		leafNode->nextLeafNode = buffer[bufferSize - 1];

		for (int i = 0; i < bufferSize - 1; i=i+2) {
			// buffer에 0이 있으면 데이터가 없는 것

			// 있는 데이터만 담도록 함
			if (buffer[i] != 0) {
				DataEntry* dataEntry = new DataEntry(buffer[i], buffer[i + 1]);
				leafNode->dataEntries.push_back(dataEntry);
			}
		}

		fclose(fp);

		delete[] buffer;
		return leafNode;
	}

	// [nextBID, (key, value), (key, value), ...]
	// BID의 blockOffset에서부터 LeafNode Size만큼 읽어옴
	NonLeafNode* getNonLeaf(int BID) {
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

	// 레벨 1, 2출력
	void print(string writeFileName) {
		FILE* fp = fopen(writeFileName.c_str(), "w+b");

		int rootBID = this->header.rootBID;
		NonLeafNode* rootNode = getNonLeaf(rootBID);

		string level0 = "[level 0]\n";
		string level1 = "[level 1]\n";
		string enter = "\n";
		string comma = ",";

		// level0 write
		fputs(level0.c_str(), fp);
		for (int i = 0; i < rootNode->indexEntries.size(); i++) {
			// 마지막에는 "," 제외
			if (i + 1 == rootNode->indexEntries.size()) {
				int key = rootNode->indexEntries[i]->key;
				fputs(to_string(key).c_str(), fp);
			}
			else {
				int key = rootNode->indexEntries[i]->key;
				fputs(to_string(key).c_str(), fp);
				fputs(comma.c_str(), fp);
			}
		}
		fputs(enter.c_str(), fp);
		fputs(enter.c_str(), fp);

		// level1 write
		fputs(level1.c_str(), fp);

		int nextBID = 0;
		LeafNode* leafNode;
		NonLeafNode* nonLeafNode;
		
		// depth가 1인 경우 level1노드는 leafNode
		if (this->header.depth == 1) {
			for (int i = 0; i < rootNode->indexEntries.size(); i++) {
				// i가 첫 번째면 nextBID는 nonLeafNode가 가지고 있는 BID
				if (i == 0) {
					nextBID = rootNode->NextLevelBID;
				}
				else {
					nextBID = rootNode->indexEntries[i]->BID;
				}

				leafNode = getLeaf(nextBID);
				for (int i = 0; i < leafNode->dataEntries.size(); i++) {
					int key = leafNode->dataEntries[i]->key;
					fputs(to_string(key).c_str(), fp);
					fputs(comma.c_str(), fp);
				}
			}
		}
		// 1보다 깊은 경우 level1노드는 nonLeafNode
		else if (this->header.depth > 1) {
			for (int i = 0; i < rootNode->indexEntries.size() + 1; i++) {
				// i가 첫 번째면 nextBID는 nonLeafNode가 가지고 있는 BID
				if (i == 0) {
					nextBID = rootNode->NextLevelBID;
				}
				else {
					nextBID = rootNode->indexEntries[i-1]->BID;
				}

				nonLeafNode = getNonLeaf(nextBID);
				for (int j = 0; j < nonLeafNode->indexEntries.size(); j++) {
					fputs(to_string(nonLeafNode->indexEntries[j]->key).c_str(), fp);

					// 마지막 "," 출력 안되도록 분기
					if (i == rootNode->indexEntries.size() && j == nonLeafNode->indexEntries.size() - 1) {
						continue;
					}
					fputs(comma.c_str(), fp);
				}
			}
		}
		else {

		}
		fclose(fp);
	}

	// point search
	void pointSearch(int searchKey, string writeFileName) {
		FILE* fp;
		fp = fopen(writeFileName.c_str(), "a+b");

		LeafNode* leafNode = new LeafNode();
		stack<int> routeBID = getRoute(searchKey);	// key를 이용하여 root부터 leaf까지 BID를 가진 스택 리턴

		// 가장 마지막에 있는 원소가 leafNode의 BID
		int leafNodeKey = routeBID.top();
		routeBID.pop();

		// BID를 이용하여 leafNode 리턴
		leafNode = getLeaf(leafNodeKey);

		int key = -1;
		int value = -1;
		string comma = ",";
		string enter = "\n";

		// leafNode의 DataEntry의 key와 searchKey를 비교해가면서 같은 값 나오면 파일에 바로 write (중복된 값이 없다고 가정)
		for (int i = 0; i < leafNode->dataEntries.size(); i++) {
			if (searchKey == leafNode->dataEntries[i]->key) {
				key = leafNode->dataEntries[i]->key;
				value = leafNode->dataEntries[i]->value;

				// (key, value\n) 형태로 파일에 write
				fputs(to_string(key).c_str(), fp);
				fputs(comma.c_str(), fp);
				fputs(to_string(value).c_str(), fp);
				fputs(enter.c_str(), fp);
			}
		}

		fclose(fp);
	}

	// range search
	void rangeSearch(int startRange, int endRange, string writeFileName) {
		FILE* fp;
		fp = fopen(writeFileName.c_str(), "a+b");

		LeafNode* leafNode = new LeafNode();
		stack<int> routeBID = getRoute(startRange);	// key를 이용하여 root부터 leaf까지 BID를 가진 스택 리턴

		// 가장 마지막에 있는 원소가 leafNode의 BID
		int leafNodeKey = routeBID.top();
		routeBID.pop();

		// BID를 이용하여 leafNode 리턴
		leafNode = getLeaf(leafNodeKey);

		int key = -1;
		int value = -1;
		string comma = ",";
		string enter = "\n";
		string slash = " / ";
		
		while (true) {
			// 현재 리프노드에서 탐색
			// leafNode의 DataEntry의 key와 startRange를 비교해가면서 startRange보다 큰 값이 나오면 파일에 바로 write (없는 키 값이 있을 수 있음)
			for (int i = 0; i < leafNode->dataEntries.size(); i++) {
				// endPoint보다 크면 "\n" 입력 후 탐색 종료
				if (endRange < leafNode->dataEntries[i]->key) {
					fputs(enter.c_str(), fp);
					return;
				}
				
				// endPoint보다 작지만 startPoint보다 크면 파일에 write
				if (startRange <= leafNode->dataEntries[i]->key) {
					key = leafNode->dataEntries[i]->key;
					value = leafNode->dataEntries[i]->value;

					fputs(to_string(key).c_str(), fp);
					fputs(comma.c_str(), fp);
					fputs(to_string(value).c_str(), fp);
					fputs(slash.c_str(), fp);
				}
			}
			// 다음 리프노드에서 탐색
			int nextBID = leafNode->nextLeafNode;	
			leafNode = getLeaf(nextBID);
		}
		fclose(fp);
	}

	// blockID 가 BID인 Block의 시작 위치 리턴
	int getBlockOffset(int BID) {
		return 12 + ((BID - 1) * this->header.blockSize);
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

		// 블록 개수 증가
		this->totalBlockCount++;

		delete[] buffer;
		fclose(fp);
	}

	// nodeSize 리턴 : blockSize를 4로 나누어 리턴
	int getNodeSize() {
		return this->header.blockSize / 4;
	}

	
};

int main(int argc, char* argv[]) {
	char command = argv[1][0];
	string fileName = argv[2];	// btree.bin
	string readFileName = "";		// read할 파일명
	string writeFileName = "";		// search 결과를 적을 파일명
	
	vector<int> data;	// 읽어온 데이터를 저장할 벡터

	int blockSize = 0;
	BTree* myBTree = new BTree(fileName.c_str());

	switch (command) {
	case 'c':
		// create index file
		blockSize = stoi(string(argv[3]));	// argv로부터 blockSize 가져오기
		myBTree->creation(fileName.c_str(), blockSize);
		break;

	case 'i':
		// insert records from [records data file], ex) records.txt
		readFileName = argv[3];
		data = readFile(readFileName, command);

		// 삽입
		for (int i = 0; i < data.size(); i = i + 2) {
			//cout << data[i] << ", " << data[i + 1] << endl;
			 myBTree->insert(data[i], data[i+1]);
		}
		break;

	case 's':
		// search keys in [input file] and print results to [output file]
		readFileName = argv[3];
		writeFileName = argv[4];		// search 결과를 적을 파일명

		data = readFile(readFileName, command);

		// point search
		for (int i = 0; i < data.size(); i = i++) {
			myBTree->pointSearch(data[i], writeFileName);
		}
		break;

	case 'r':
		// search keys in [input file] and print results to [output file]
		readFileName = argv[3];
		writeFileName = argv[4];		// search 결과를 적을 파일명

		data = readFile(readFileName, command);

		// range search
		for (int i = 0; i < data.size(); i = i + 2) {
			myBTree->rangeSearch(data[i], data[i + 1], writeFileName);
		}
		break;

	case 'p':
		// print B+-tree structure to [output file]
		writeFileName = argv[3];		// print할 파일명

		myBTree->print(writeFileName);
		break;
	}
 }