#pragma once

#include <queue>
#include <stack>

typedef struct {} TEmpty;

template <typename TKey, typename TData = TEmpty, int SizeParam = 4, bool UniqueKeys = true>
class CBTree 
{
private:
	static const unsigned char cIsLeafMask = 0x80;
	static const unsigned char cKeyCountMask = 0x7F;
	static const unsigned char cFullNode = SizeParam * 2 - 1;
	static const unsigned char cEmptyNode = SizeParam - 1;



	#pragma pack(push)  /* push current alignment to stack */
	#pragma pack(1)     /* set alignment to 1 byte boundary */

		typedef struct TNode {
			unsigned int Parent;
			unsigned char Info;
			TKey Key[SizeParam * 2 - 1];
			TData Data[SizeParam * 2 - 1];
			unsigned int Child[SizeParam * 2];
		} TNode;

	#pragma pack(pop)

	bool InNodeFind(const TNode & Node, const TKey & Key, int & GreaterEqual);
	void SplitNode(unsigned int Node, unsigned int & Left, unsigned int & Right, TKey & UpKey, TData & UpData);
	unsigned int MergeNodes(unsigned int Left, unsigned int Right, const TKey & DownKey, const TData & DownData);

	
	void ReadNode(unsigned int Node, TNode & Dest);
	void WriteNode(unsigned int Node, TNode & Source);

protected:
	unsigned int m_Root;

	virtual unsigned int CreateNewNode();
	virtual void DeleteNode(unsigned int Node);
	virtual void RootChanged();
	virtual void Read(unsigned int Node, int Offset, int Size, TNode & Dest);
	virtual void Write(unsigned int Node, int Offset, int Size, TNode & Source);
	
	virtual void ClearTree();
	
public:
	
	class iterator {
	private:
		unsigned int m_Node;
		int m_Index;
		CBTree* m_Tree;

	public:
		iterator();
		iterator(CBTree* Tree, unsigned int Node, int Index);
		iterator(const iterator& Other);
		~iterator();

		TKey* getKey();
		TKey& getKey() const;

		TData* getData();
		TData& getData() const;

		operator bool() const;
		bool operator !() const;
		
		iterator& operator =(const iterator& Other);
		void operator ++(int);
		void operator --(int);

		TData* operator ->();
		TData& operator * (); 

	};


	CBTree(unsigned int RootNode = NULL);
	~CBTree();

	iterator& Insert(const TKey & Key, const TData & Data) const;
	iterator& Find(const TKey & Key) const;
	bool Delete(const TKey& Key);
	void Delete(const iterator& Item);

};