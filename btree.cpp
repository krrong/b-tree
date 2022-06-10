#pragma warning(disable:4996)
#include <iostream>
#include <vector>
#include <cstdio>
#include <string>
using namespace std;

// ����
// fwrite(�����ּ�, ���Ͽ� �� byte ��, ���Ͽ� �� ������ ����, ���� ������)

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

class BTree {
public:
	// ��� ����
	Header header;			// Header ���� ����
	const char* fileName;	// ���� �̸�
	int blockSize;			// ��� ������

	// ������
	BTree(const char* fileName) {
		this->fileName = fileName;
		readHeader();
	}

	// creation�� ������ command�� btree.bin�� �̿��Ͽ� btree�� �ʱ�ȭ ��
	// --> btree.bin���κ��� btree�� ����� ������ �ʿ�
	void readHeader() {
		FILE* fp = fopen(this->fileName, "r+b");	// ������ binary ���·� �а� �� �� �ֵ��� open

		// blockSize, rootBID, depth
		int buffer[3];

		fread(buffer, sizeof(int), 3, fp);

		this->header.blockSize = buffer[0];
		this->header.rootBID = buffer[1];
		this->header.depth = buffer[2];

		fclose(fp);
	}

	// ���ڷ� ���� rootBID�� depth�� ���Ͽ� �ٽ� �ۼ�
	void setHeader(int rootBID, int depth) {
		FILE* fp = fopen(this->fileName, "r+b");	// ������ binary ���·� �а� �� �� �ֵ��� open

		// rootBID�� �ִ� ������ Ŀ�� �̵�
		fseek(fp, sizeof(int), SEEK_SET);

		// 
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
		// Ʈ���� ��������� ��Ʈ��� ����
		if (this->header.rootBID == 0) {
			this->header.rootBID = 1;	// ���� ���� �� ��Ʈ BID = 1
			makeNewNode(this->header.rootBID);
			setHeader(this->header.rootBID, this->header.depth);
		}
		
		writeData(this->header.rootBID, key, rid);

		return true;
	}

	// Key�� ������ �ִ� block search
	int searchBlock(int searchKey) {
		int curBID = this->header.rootBID;
		int totalDepth = this->header.depth;
		int curDepth = 0;

		// ��Ʈ���� key�� ������ �ִ� ��� Ž��
		while (totalDepth != curDepth) {
			NonLeafNode* curNonLeafNode = getNonLeafNode(curBID);	// BID�� ������ NonLeafNode �о��
			bool searchFlag = false;

			// NonLeafNode : [nextBID, (key, value), (key, value)...]
			for (int i = 0; i < curNonLeafNode->indexEntries.size(); i++) {
				int nextBID = 0;
				int nextKey = 0;

				// i == 0�� ��� nextBID�� NonLeafNode�� NextLevelBID
				if (i == 0) {
					nextBID = curNonLeafNode->NextLevelBID;
					nextKey = curNonLeafNode->indexEntries.front()->key;
				}
				// i != 0�� ��� nextBID�� NonLeafNode->indexEntries[i-1]->BID
				else {
					nextBID = curNonLeafNode->indexEntries[i - 1]->BID;
					nextKey = curNonLeafNode->indexEntries[i]->key;
				}

				// searchKey�� �� ������ ���� BID ��� & ���� ��� Ž�� ����
				if (searchKey < nextKey) {
					curBID = nextBID;
					curDepth++;
					searchFlag = true;
					break;
				}
				// searchKey�� �� ũ�� ���������� �̵�
			}
			// ���� ��忡�� ��ã������ ���� ������ BID�� �̵�
			if (searchFlag == false) {
				curBID = curNonLeafNode->indexEntries.back()->BID;
				curDepth++;
			}
		}

		return curBID;
	}

	// ���Ͽ� ������ write
	void writeData(int BID, int key, int value) {
		FILE* fp = fopen(this->fileName, "r+b");

		// ó������ block�� ���� ��ġ���� Ŀ�� �̵�
		int blockOffset = getBlockOffset(BID);
		fseek(fp, blockOffset, SEEK_SET);

		int k = key;
		int v = value;

		fwrite(&k, sizeof(int), 1, fp);
		fwrite(&v, sizeof(int), 1, fp);

		fclose(fp);
	}

	// Node�� Entry�� (blockSize - 4) / 8 �� ��ŭ �� �� ����
	// true�� split �ʿ�
	bool isLeafNodeFull(LeafNode* leafNode) {
		return leafNode->dataEntries.size() > (this->blockSize - 4) / 8;
	}

	// Node�� Entry�� (blockSize - 4) / 8 �� ��ŭ �� �� ����
	// true�� split �ʿ�
	bool isNonLeafNodeFull(NonLeafNode* nonLeafNode) {
		return nonLeafNode->indexEntries.size() > (this->blockSize - 4) / 8;
	}

	// [(key, value), (key, value), ..., nextBID]
	// BID�� blockOffset�������� LeafNode Size��ŭ �о��
	LeafNode* getLeafNode(int BID) {
		int bufferSize = getNodeSize();		
		int* buffer = new int[bufferSize]();	// 0���� �ʱ�ȭ
		int blockOffset = getBlockOffset(BID);

		FILE* fp = fopen(this->fileName, "rb");

		// ó������ blockOffset��ŭ Ŀ�� �̵�
		fseek(fp, blockOffset, SEEK_SET);

		// bufferSize��ŭ �о��
		fread(buffer, sizeof(int), bufferSize, fp);

		LeafNode* leafNode = new LeafNode();
		
		for (int i = 0; i < bufferSize - 1; i=i+2) {
			// buffer�� 0�� ������ �����Ͱ� ���� ��

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
	// BID�� blockOffset�������� LeafNode Size��ŭ �о��
	NonLeafNode* getNonLeafNode(int BID) {
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

	// ���
	void print() {

	}

	// point search
	int* search(int key) {

	}

	// range search
	int* search(int startRange, int endRange) {

	}

	// blockID �� BID�� Block�� ���� ��ġ ����
	int getBlockOffset(int BID) {
		return 12 + ((BID - 1) * this->blockSize);
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

		delete[] buffer;
		fclose(fp);
	}

	// nodeSize ���� : blockSize�� 4�� ������ ����
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
		blockSize = stoi(string(argv[3]));	// argv�κ��� blockSize ��������
		myBTree->creation(fileName, blockSize);
		break;
	case 'i':
		// insert records from [records data file], ex) records.txt
		//readFileName = argv[3];
		//data = readFile(readFileName, command);

		//// Ȯ��
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

		// Ȯ��
		for (int i = 0; i < data.size(); i = i++) {
			cout << data[i] << endl;
		}
		break;

	case 'r':
		// search keys in [input file] and print results to [output file]
		readFileName = argv[3];
		data = readFile(readFileName, command);

		// Ȯ��
		for (int i = 0; i < data.size(); i = i + 2) {
			cout << data[i] << ", " << data[i + 1] << endl;
		}
		break;

	case 'p':
		// print B+-tree structure to [output file]
		break;
	}
 }