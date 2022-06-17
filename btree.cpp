#pragma warning(disable:4996)
#include <iostream>
#include <vector>
#include <stack>
#include <cstdio>
#include <string>
#include <algorithm>
using namespace std;

// Leaf node�� ���� ������ --> 8byte
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

// Non leaf node�� ���� ������ --> 8byte
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

// 12byte
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

	// �⺻ ������
	Header() {
		blockSize = 0;
		rootBID = 0;
		depth = 0;
	}
};

class LeafNode {
public:
	vector<DataEntry*> dataEntries;	// leaf node�� ������ �ִ� data entry
	int nextLeafNode;				// ���� leaf node�� ����Ŵ

	// ������
	LeafNode() {
		nextLeafNode = 0;
	}
};

class NonLeafNode {
public:
	int NextLevelBID;				 // ���� ������ ����Ŵ
	vector<IndexEntry*> indexEntries; // non leaf node�� ������ �ִ� index entry

	NonLeafNode() {
		NextLevelBID = 0;
	}
};

// key�� �������� dataEntry �������� ���� (LeafNode)
bool compLeafNode(DataEntry* a, DataEntry* b) {
	return a->key < b->key;
}

// key �������� indexEntry �������� ���� (NonLeafNode)
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
	string preStr;	// ������ ���� �ݺ��Ǵ� �� ��������
	vector<int> result;

	// point search�� �� ���ο� �ϳ��� ���ڰ� �־� ���� �ʿ� X
	if (command == 's') {
		while (feof(filePointer) == 0) {
			fgets(str, 100, filePointer);

			// ���๮�� ����
			if (str[strlen(str) - 1] == '\n') {
				str[strlen(str) - 1] = '\0';
			}

			// �Է��� ������ ����
			if (str[0] == '\0') {
				break;
			}

			// ���� ������ ���Ϳ� ����
			result.push_back(stoi(string(str)));	// string -> int
		}
	}
	// range search�� insertion�� �� ���ο� �� ���� ���ڰ� �־� ���� �ʿ�
	else {
		// ���� �����Ͱ� ������ ���� �ƴ� �� ��� �ݺ�
		while (feof(filePointer) == 0) {
			fgets(str, 100, filePointer);

			// ���๮�� ����
			if (str[strlen(str) - 1] == '\n') {
				str[strlen(str) - 1] = '\0';
			}

			char* context = NULL;	// �и��� �� ���� ���ڿ��� ��
			char* token = strtok_s(str, ", ", &context);

			// ������ ���� �ݺ��Ǵ� �� ��������
			if (preStr == token) {
				break;
			}
			preStr = token;

			// ���� ������ ��ū���� ������ ���Ϳ� ����
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
	// ��� ����
	Header header;			// Header ���� ����
	const char* fileName;	// ���� �̸�
	int totalBlockCount;	// Btree�� �� block ����

	// ������
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

		// size = ������ �� bytes
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);

		fclose(fp);

		// btree.bin �� blockSize�� 0�̸� -1 ����
		if (this->header.blockSize == 0) {
			return -1;
		}

		// ������ �� bytes - (12(header) / blockSize) = �� block�� ��
		return (size - 12) / this->header.blockSize;
	}

	// creation�� ������ command�� btree.bin�� �̿��Ͽ� btree�� �ʱ�ȭ ��
	//  --> btree.bin���κ��� btree�� ����� ������ �ʿ�
	// btree.bin���Ϸκ��� btree�� header����
	void readHeader() {
		FILE* fp = fopen(this->fileName, "r+b");	// ������ binary ���·� �а� �� �� �ֵ��� open

		if (fp == NULL) {
			return;
		}

		// blockSize, rootBID, depth
		int buffer[3];

		// ������ �о� header����
		fread(buffer, sizeof(int), 3, fp);
		this->header.blockSize = buffer[0];
		this->header.rootBID = buffer[1];
		this->header.depth = buffer[2];

		fclose(fp);
	}

	// ���ڷ� ���� rootBID�� depth�� btree.bin ���� header �κп� �ٽ� �ۼ�
	void writeHeader(int rootBID, int depth) {
		FILE* fp = fopen(this->fileName, "r+b");	// ������ binary ���·� �а� �� �� �ֵ��� open

		// rootBID�� �ִ� ������ Ŀ�� �̵�
		fseek(fp, sizeof(int), SEEK_SET);

		// rootBID, depth write
		fwrite(&rootBID, sizeof(int), 1, fp);
		fwrite(&depth, sizeof(int), 1, fp);

		fclose(fp);
	}

	// file btree ����
	void creation(const char* fileName, int blockSize) {
		// btree�� ���� �̸��� �ʱ�ȭ�Ǿ� ���� ������ ���� �߻� -> ����
		if (fileName == NULL) {
			cout << "������ ��ã�ҽ��ϴ�." << '\n';
			return;
		}

		FILE* filePointer = fopen(this->fileName, "wb");
		if (filePointer == NULL) {
			cout << "File Error from creation" << '\n';
			exit(1);
		}

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
		// leafNode�� �߰� �� dataEntry ����
		DataEntry* newDataEntry = new DataEntry(key, rid);

		// Ʈ���� ��������� ��Ʈ��� ����
  		if (this->header.rootBID == 0) {
			this->header.rootBID = 1;	// ���� ���� �� ��Ʈ BID = 1
			makeNewNode(this->header.rootBID);	// ���ο� ���(��Ʈ) ����
			writeHeader(this->header.rootBID, this->header.depth);	// btree ��� ���� ����
		}

		// searchKey�� �����ϴ� leafNode�� Ž��
		stack<int> routeBID = getRoute(key);
		int updateBID = routeBID.top();	// leafNode�� Key�� stack�� top�� ����
		routeBID.pop();

		// leafNode�� ���� ������ dataEntry�߰�, key���� �������� ����->split �ʿ� Ȯ��
   		LeafNode* leafNode = getLeaf(updateBID);
		leafNode->dataEntries.push_back(newDataEntry);
		sort(leafNode->dataEntries.begin(), leafNode->dataEntries.end(), compLeafNode);

		// split�ʿ����� ���� ���
		if (!isLeafNodeFull(leafNode)) {
			// split�� �ʿ����� ������ �̹� ���ĵ� �����̱� ������ �ٷ� rewrite�ϰ� ����
			rewriteLeafNode(updateBID, leafNode);
			return true;
		}
		else {
			// leafNode�� �� ���� leafNode split���� --> nonLeafNode�� ���ο� indexDentry insert �ʿ�
			IndexEntry* newIndexEntry = leafNodeSplit(leafNode, updateBID);

			// �θ� ��嵵 split�� �ʿ����� root���� �ö󰡸鼭 Ȯ��
			while (true) {
				// �θ� ��尡 ������� �θ� ���� �� ����
				if (routeBID.empty()) {
					// nonLeafNode ���� (��Ʈ ���)
					NonLeafNode* parentNode = new NonLeafNode();

					int parentBID = getTotalBlockCount() + 1;
					makeNewNode(parentBID);
					parentNode->NextLevelBID = updateBID;	// NextLevelBID�� split���� ���� ��带 ����Ŵ
					parentNode->indexEntries.push_back(newIndexEntry);	// split���� ������ ���κ��� ������� IndexEntry�� �߰�

					this->header.depth = this->header.depth + 1;	// �θ� ��� �߰� �� depth + 1
					this->header.rootBID = parentBID;	// �θ� ��� �߰� �� depth + 1
					writeHeader(parentBID, this->header.depth);	// �θ� ��� �߰� �� rootNodeBID����

					// NonLeafNode write�� ����
					rewriteNonLeafNode(parentBID, parentNode);
					break;
				}
				// �θ� �ִ� ��� 2����
				// 1. �θ� �ڸ��� �־ ���� ������ ���
				// 2. �θ� �ڸ��� ��� �ٽ� split�� �Ͼ�� ���
				else {
					// �θ����� BID
					int parentBID = routeBID.top();
					routeBID.pop();

					// �θ� ��� NonLeafNode�� split�ϸ� ���� IndexEntry�߰� �� ����
					NonLeafNode* parentNonLeafNode = getNonLeaf(parentBID);
					parentNonLeafNode->indexEntries.push_back(newIndexEntry);
					sort(parentNonLeafNode->indexEntries.begin(), parentNonLeafNode->indexEntries.end(), compNonLeafNode);

					// 1. �θ� �ڸ��� ��� �ٽ� split�� �Ͼ�� ���
					if (!isNonLeafNodeFull(parentNonLeafNode)) {
						// �θ� ��带 �ٽ� ���Ͽ� write -> ����(split ���� X)
						rewriteNonLeafNode(parentBID, parentNonLeafNode);
						break;
					}
					// 2. �θ� �ڸ��� �־ ���� ������ ���
					else {
						// split ���� ���� ����� IndexEntry
						newIndexEntry = nonLeafNodeSplit(parentNonLeafNode, parentBID);

						// ������ ���� �θ��������� updateBID ����
						updateBID = parentBID;
					}
				}
			}
		}
		return true;
	}

	// split���� �θ��忡 �߰��� IndexEnrty ��ȯ
	IndexEntry* leafNodeSplit(LeafNode* originLeafNode, int originLeafNodeBID) {
		// ���ø��� �ǰ��� ���ο� ��� ���� entry�� ���������� ���� leafNode�� ���� ���� leafNode�� ����Ű�� ����
		// ������ ���� ���� ��尡 �����ǰ���? --> Btree���� 12 + ��ϻ����� * ����� ���� �ϸ� ���� ���� offset�� �����ϱ� btree�� total block�� ������ ��������
		
		int newBlockID = this->totalBlockCount + 1;	// �� block �� ����
		makeNewNode(newBlockID);	// ���Ͽ� ���ο� ��� 0���� write
		LeafNode* newLeafNode = new LeafNode();	// split���� ����� ���ο� leafNode

		newLeafNode->nextLeafNode = originLeafNode->nextLeafNode;	// ���� node�� ����Ű�� nextBID�� split ���� ���� ����� ����� nextBID�� �־���
		originLeafNode->nextLeafNode = newBlockID;	// split�� ��忡�� split ���� ���� ����� ���� ����

		// ���� : [0 ~ dataEntries.size() / 2)
		// ������ : [dataEntries.size() / 2 ~ dataEntries.size())
		// 5�����
		// ���� 0, 1, 2
		// ������ 3, 4
		int left = originLeafNode->dataEntries.size() / 2;

		// ���� leaf���� ���� ���� leaf�� ���� ������ ����
		for (int i = left; i < originLeafNode->dataEntries.size(); i++) {

			DataEntry* newDataEntry = new DataEntry(originLeafNode->dataEntries[i]->key, originLeafNode->dataEntries[i]->value);
			newLeafNode->dataEntries.push_back(newDataEntry);
		}

		// ������ ��ŭ�� ������ ����
		originLeafNode->dataEntries.erase(originLeafNode->dataEntries.begin() + left, originLeafNode->dataEntries.end());

		rewriteLeafNode(originLeafNodeBID, originLeafNode);	// ���� ��� rewrite
		rewriteLeafNode(newBlockID, newLeafNode);	// split �ؼ� ���� ��� write

		// split���� �θ��忡 �߰��� IndexEnrty ���� (���� ����� ù ��° key��, ���� ����� BID)
		IndexEntry* newIndexEntry = new IndexEntry(newLeafNode->dataEntries[0]->key, newBlockID);

		return newIndexEntry;
	}

	// split���� �θ��忡 �߰��� IndexEntry ��ȯ
	IndexEntry* nonLeafNodeSplit(NonLeafNode* originNonLeafNode, int originNonLeafNodeBID) {
		// ���ø��� �ǰ��� ���ο� ��� ���� entry�� ���������� ���� ���� IndexEntry ����
		// ������ ���� ���� ��尡 ������ ��
		
		int newBlockBID = this->totalBlockCount + 1;	// �� block �� ����
		makeNewNode(newBlockBID);	// ���Ͽ� ���ο� ��� 0���� write
		NonLeafNode* newNonLeafNode = new NonLeafNode();	// split���� ����� ���ο� nonLeafNode

		int left = originNonLeafNode->indexEntries.size() / 2;
		newNonLeafNode->NextLevelBID = originNonLeafNode->indexEntries[left]->BID;	// split�Ͽ� ����� nonleafnode�� nextLevelBID�� ���������� ù ��° BID�� ����

		// ���� nonLeafNode���� ���� ���� nonLeafNode�� ������ ���� (���� ���� - 1)
		for (int i = left + 1; i < originNonLeafNode->indexEntries.size(); i++) {

			IndexEntry* newIndexEntry = new IndexEntry(originNonLeafNode->indexEntries[i]->key, originNonLeafNode->indexEntries[i]->BID);
			newNonLeafNode->indexEntries.push_back(newIndexEntry);
		}

		// �������� ù ��° key���� ���� BID�� �θ�� �ö󰡴� IndexEntry
		IndexEntry* newIndexEntry = new IndexEntry(originNonLeafNode->indexEntries[left]->key, newBlockBID);

		// �����Ѹ�ŭ�� ������ ����
		originNonLeafNode->indexEntries.erase(originNonLeafNode->indexEntries.begin() + left, originNonLeafNode->indexEntries.end());

		rewriteNonLeafNode(originNonLeafNodeBID, originNonLeafNode);	// ���� ��� rewrite
		rewriteNonLeafNode(newBlockBID, newNonLeafNode);	// split �ؼ� ���� ��� rewrite

		return newIndexEntry;
	}

	// BID�� LeafNode�� ���ڷ� �޾� ���Ͽ� rewrite
	void rewriteLeafNode(int BID, LeafNode* leafNode) {
		// BID�� ���� ��ġ ��ȯ
		int blockOffset = getBlockOffset(BID);

		FILE* fp = fopen(this->fileName, "r+b");

		// ó������ block�� ���� ��ġ���� Ŀ�� �̵�
		fseek(fp, blockOffset, SEEK_SET);

		// ���Ͽ� 0���� ���� ����
		int bufferByte = getNodeSize();
		int* buffer = new int[bufferByte]();

		// blockOffset��ġ���� nodeSize��ŭ 0���� write --> rewrite �ϱ� ���� ���� �غ�
		fwrite(buffer, sizeof(int), bufferByte, fp);

		// fwrite�� �ϸ� Ŀ���� �̵��Ǳ� ������ �ٽ� ����ġ�� �̵�
		fseek(fp, blockOffset, SEEK_SET);

		// leafNode�� dataEntry write
		for (int i = 0; i < leafNode->dataEntries.size(); i++) {
			// file�� write
			fwrite(&leafNode->dataEntries[i]->key, sizeof(int), 1, fp);
			fwrite(&leafNode->dataEntries[i]->value, sizeof(int), 1, fp);
		}

		// leafNode�� nextLeafNode ��ġ�� �̵�
		fseek(fp, blockOffset + this->header.blockSize - 4, SEEK_SET);

		// leafNode�� nextLeafNode write
		int nextLeafNode = leafNode->nextLeafNode;
		fwrite(&nextLeafNode, sizeof(int), 1, fp);

		fclose(fp);
	}

	// BID�� NonLeafNode�� ���ڷ� �޾� ���Ͽ� rewrite
	void rewriteNonLeafNode(int BID, NonLeafNode* nonLeafNode) {
		// BID�� ���� ��ġ ��ȯ
		int blockOffset = getBlockOffset(BID);

		FILE* fp = fopen(this->fileName, "r+b");

		// ó������ block�� ���� ��ġ���� Ŀ�� �̵�
		fseek(fp, blockOffset, SEEK_SET);

		// ���Ͽ� 0���� ���� ����
		int bufferByte = getNodeSize();
		int* buffer = new int[bufferByte]();

		// blockOffset��ġ���� nodeSize��ŭ 0���� write --> rewrite �ϱ� ���� ���� �غ�
		fwrite(buffer, sizeof(int), bufferByte, fp);
		
		// fwrite�� �ϸ� Ŀ���� �̵��Ǳ� ������ �ٽ� ����ġ�� �̵�
		fseek(fp, blockOffset, SEEK_SET);

		// nextLevelBID write
		int NextLevelBID = nonLeafNode->NextLevelBID;
		fwrite(&NextLevelBID, sizeof(int), 1, fp);

		// nonLeafNode�� indexEntry write
		for (int i = 0; i < nonLeafNode->indexEntries.size(); i++) {
			// file�� write
			fwrite(&nonLeafNode->indexEntries[i]->key, sizeof(int), 1, fp);
			fwrite(&nonLeafNode->indexEntries[i]->BID, sizeof(int), 1, fp);
		}

		// leafNode�� nextLeafNode ��ġ�� �̵�
		fseek(fp, blockOffset + this->header.blockSize - 4, SEEK_SET);

		fclose(fp);
	}

	// searchKey�� ���� block search
	//  --> searchKey ������ ������ �ִ� block�� ã�ư��� route�� BID����
	stack<int> getRoute(int searchKey) {
		stack<int> result;
		int curBID = this->header.rootBID;
		int totalDepth = this->header.depth;
		int curDepth = 0;
		result.push(curBID);	// root ��常 ������ �� while���� Ÿ�� ����

		// ��Ʈ���� key�� ������ �ִ� ��� Ž��
		while (totalDepth != curDepth) {
			NonLeafNode* curNonLeafNode = getNonLeaf(curBID);	// BID�� ������ NonLeafNode �о��
			bool searchFlag = false;

			int size = curNonLeafNode->indexEntries.size();
			// NonLeafNode : [nextBID, (key, value), (key, value)...]
			for (int i = 0; i < size; i++) {
				int nextKey = 0;
				int nextBID = 0;

				// i == 0�� ��� nextBID�� NonLeafNode�� NextLevelBID
				if (i == 0) {
					nextKey = curNonLeafNode->indexEntries[0]->key;
					nextBID = curNonLeafNode->NextLevelBID;
				}
				// i != 0�� ��� nextBID�� NonLeafNode->indexEntries[i-1]->BID
				// i != 0�� ��� nextKey�� NonLeafNode->indexEntries[i]->key
				else {
					nextKey = curNonLeafNode->indexEntries[i]->key;
					nextBID = curNonLeafNode->indexEntries[i - 1]->BID;
				}

				// searchKey�� �� ������ ���� BID ��� & ���� ��� Ž�� ����
				if (searchKey < nextKey) {
					curDepth++;
					searchFlag = true;
					curBID = nextBID;
					result.push(curBID);
					break;
				}
				// searchKey�� �� ũ�� ��忡�� ���������� �̵�
			}
			// ���� ��忡�� ��ã������ ���� ������ BID�� �̵�
			if (searchFlag == false) {
				curBID = curNonLeafNode->indexEntries.back()->BID;
				curDepth++;
				result.push(curBID);
			}
		}

		return result;
	}


	// Node�� Entry�� (blockSize - 4) / 8 �� ��ŭ �� �� ����
	// true�� split �ʿ�
	bool isLeafNodeFull(LeafNode* leafNode) {
		return leafNode->dataEntries.size() > ((this->header.blockSize - 4) / 8);
	}

	// Node�� Entry�� (blockSize - 4) / 8 �� ��ŭ �� �� ����
	// true�� split �ʿ�
	bool isNonLeafNodeFull(NonLeafNode* nonLeafNode) {
		return nonLeafNode->indexEntries.size() > (this->header.blockSize - 4) / 8;
	}

	// [(key, value), (key, value), ..., nextBID]
	// BID�� blockOffset�������� LeafNode Size��ŭ �о��
	LeafNode* getLeaf(int BID) {
		int bufferSize = getNodeSize();		
		int* buffer = new int[bufferSize]();	// 0���� �ʱ�ȭ
		int blockOffset = getBlockOffset(BID);

		FILE* fp = fopen(this->fileName, "rb");

		// ó������ blockOffset��ŭ Ŀ�� �̵�
		fseek(fp, blockOffset, SEEK_SET);

		// bufferSize��ŭ �о��
		fread(buffer, sizeof(int), bufferSize, fp);

		LeafNode* leafNode = new LeafNode();
		// ���� leafNode�� ����Ű�� BID�� buffer �� �������� ������
		leafNode->nextLeafNode = buffer[bufferSize - 1];

		for (int i = 0; i < bufferSize - 1; i=i+2) {
			// buffer�� 0�� ������ �����Ͱ� ���� ��

			// �ִ� �����͸� �㵵�� ��
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
	// BID�� blockOffset�������� LeafNode Size��ŭ �о��
	NonLeafNode* getNonLeaf(int BID) {
		int bufferSize = getNodeSize();
		int* buffer = new int[bufferSize]();	// 0���� �ʱ�ȭ
		int blockOffset = getBlockOffset(BID);

		FILE* fp = fopen(this->fileName, "rb");

		// ó������ blockOffset��ŭ Ŀ�� �̵�
		fseek(fp, blockOffset, SEEK_SET);

		// bufferSize��ŭ �о��
		fread(buffer, sizeof(int), bufferSize, fp);

		NonLeafNode* nonLeafNode = new NonLeafNode();
		for (int i = 1; i < bufferSize; i = i + 2) {
			// buffer�� 0�� ������ �����Ͱ� ���� ��

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

	// ���� 1, 2���
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
			// ���������� "," ����
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
		
		// depth�� 1�� ��� level1���� leafNode
		if (this->header.depth == 1) {
			for (int i = 0; i < rootNode->indexEntries.size(); i++) {
				// i�� ù ��°�� nextBID�� nonLeafNode�� ������ �ִ� BID
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
		// 1���� ���� ��� level1���� nonLeafNode
		else if (this->header.depth > 1) {
			for (int i = 0; i < rootNode->indexEntries.size() + 1; i++) {
				// i�� ù ��°�� nextBID�� nonLeafNode�� ������ �ִ� BID
				if (i == 0) {
					nextBID = rootNode->NextLevelBID;
				}
				else {
					nextBID = rootNode->indexEntries[i-1]->BID;
				}

				nonLeafNode = getNonLeaf(nextBID);
				for (int j = 0; j < nonLeafNode->indexEntries.size(); j++) {
					fputs(to_string(nonLeafNode->indexEntries[j]->key).c_str(), fp);

					// ������ "," ��� �ȵǵ��� �б�
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
		stack<int> routeBID = getRoute(searchKey);	// key�� �̿��Ͽ� root���� leaf���� BID�� ���� ���� ����

		// ���� �������� �ִ� ���Ұ� leafNode�� BID
		int leafNodeKey = routeBID.top();
		routeBID.pop();

		// BID�� �̿��Ͽ� leafNode ����
		leafNode = getLeaf(leafNodeKey);

		int key = -1;
		int value = -1;
		string comma = ",";
		string enter = "\n";

		// leafNode�� DataEntry�� key�� searchKey�� ���ذ��鼭 ���� �� ������ ���Ͽ� �ٷ� write (�ߺ��� ���� ���ٰ� ����)
		for (int i = 0; i < leafNode->dataEntries.size(); i++) {
			if (searchKey == leafNode->dataEntries[i]->key) {
				key = leafNode->dataEntries[i]->key;
				value = leafNode->dataEntries[i]->value;

				// (key, value\n) ���·� ���Ͽ� write
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
		stack<int> routeBID = getRoute(startRange);	// key�� �̿��Ͽ� root���� leaf���� BID�� ���� ���� ����

		// ���� �������� �ִ� ���Ұ� leafNode�� BID
		int leafNodeKey = routeBID.top();
		routeBID.pop();

		// BID�� �̿��Ͽ� leafNode ����
		leafNode = getLeaf(leafNodeKey);

		int key = -1;
		int value = -1;
		string comma = ",";
		string enter = "\n";
		string slash = " / ";
		
		while (true) {
			// ���� ������忡�� Ž��
			// leafNode�� DataEntry�� key�� startRange�� ���ذ��鼭 startRange���� ū ���� ������ ���Ͽ� �ٷ� write (���� Ű ���� ���� �� ����)
			for (int i = 0; i < leafNode->dataEntries.size(); i++) {
				// endPoint���� ũ�� "\n" �Է� �� Ž�� ����
				if (endRange < leafNode->dataEntries[i]->key) {
					fputs(enter.c_str(), fp);
					return;
				}
				
				// endPoint���� ������ startPoint���� ũ�� ���Ͽ� write
				if (startRange <= leafNode->dataEntries[i]->key) {
					key = leafNode->dataEntries[i]->key;
					value = leafNode->dataEntries[i]->value;

					fputs(to_string(key).c_str(), fp);
					fputs(comma.c_str(), fp);
					fputs(to_string(value).c_str(), fp);
					fputs(slash.c_str(), fp);
				}
			}
			// ���� ������忡�� Ž��
			int nextBID = leafNode->nextLeafNode;	
			leafNode = getLeaf(nextBID);
		}
		fclose(fp);
	}

	// blockID �� BID�� Block�� ���� ��ġ ����
	int getBlockOffset(int BID) {
		return 12 + ((BID - 1) * this->header.blockSize);
	}

	// ���ο� ��� ���� -> ������ ������ ��ġ�� nodeSize��ŭ�� 0���� write
	void makeNewNode(int blockBID) {
		FILE* fp = fopen(this->fileName, "ab");

		int blockOffset = getBlockOffset(blockBID);
		
		// ������ ó������ blockOffset��ŭ Ŀ�� �̵�
		fseek(fp, blockOffset, SEEK_SET);
		
		int bufferByte = getNodeSize();
		int* buffer = new int[bufferByte] ();		// nodeSize��ŭ �����Ҵ� -> ���Ͽ� 0���� ���� ����

		// blockOffset��ġ���� nodeSize��ŭ 0���� write
		fwrite(buffer, sizeof(int), bufferByte, fp);

		// ��� ���� ����
		this->totalBlockCount++;

		delete[] buffer;
		fclose(fp);
	}

	// nodeSize ���� : blockSize�� 4�� ������ ����
	int getNodeSize() {
		return this->header.blockSize / 4;
	}

	
};

int main(int argc, char* argv[]) {
	char command = argv[1][0];
	string fileName = argv[2];	// btree.bin
	string readFileName = "";		// read�� ���ϸ�
	string writeFileName = "";		// search ����� ���� ���ϸ�
	
	vector<int> data;	// �о�� �����͸� ������ ����

	int blockSize = 0;
	BTree* myBTree = new BTree(fileName.c_str());

	switch (command) {
	case 'c':
		// create index file
		blockSize = stoi(string(argv[3]));	// argv�κ��� blockSize ��������
		myBTree->creation(fileName.c_str(), blockSize);
		break;

	case 'i':
		// insert records from [records data file], ex) records.txt
		readFileName = argv[3];
		data = readFile(readFileName, command);

		// ����
		for (int i = 0; i < data.size(); i = i + 2) {
			//cout << data[i] << ", " << data[i + 1] << endl;
			 myBTree->insert(data[i], data[i+1]);
		}
		break;

	case 's':
		// search keys in [input file] and print results to [output file]
		readFileName = argv[3];
		writeFileName = argv[4];		// search ����� ���� ���ϸ�

		data = readFile(readFileName, command);

		// point search
		for (int i = 0; i < data.size(); i = i++) {
			myBTree->pointSearch(data[i], writeFileName);
		}
		break;

	case 'r':
		// search keys in [input file] and print results to [output file]
		readFileName = argv[3];
		writeFileName = argv[4];		// search ����� ���� ���ϸ�

		data = readFile(readFileName, command);

		// range search
		for (int i = 0; i < data.size(); i = i + 2) {
			myBTree->rangeSearch(data[i], data[i + 1], writeFileName);
		}
		break;

	case 'p':
		// print B+-tree structure to [output file]
		writeFileName = argv[3];		// print�� ���ϸ�

		myBTree->print(writeFileName);
		break;
	}
 }