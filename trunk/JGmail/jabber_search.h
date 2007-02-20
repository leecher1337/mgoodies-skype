typedef struct _tagJabberSearchFieldsInfo
{
	TCHAR * szFieldName;
	TCHAR * szFieldCaption;
	HWND hwndCaptionItem;	
	HWND hwndValueItem;
} JabberSearchFieldsInfo;

typedef struct _tagJabberSearchData
{
	JabberSearchFieldsInfo *  pJSInf;
	int nJSInfCount;
	int lastRequestIq;
	int CurrentHeight;
	int curPos;
	int frameHeight;
	RECT frameRect;
	BOOL fSearchRequestIsXForm;
}JabberSearchData;

typedef struct tag_Data
{
	TCHAR *Label;
	TCHAR * Var;
	int Order;

} Data;


typedef struct tagJABBER_CUSTOMSEARCHRESULTS
{
	size_t nSize;	
	int nFieldCount;
	TCHAR ** pszFields;	
	JABBER_SEARCH_RESULT jsr;
}JABBER_CUSTOMSEARCHRESULTS;

static HWND searchHandleDlg=NULL;

//local functions declarations
static int JabberSearchFrameProc(HWND hwnd, int msg, WPARAM wParam, LPARAM lParam);
static int JabberSearchAddField(HWND hwndDlg, TCHAR *Label, TCHAR * Var, int Order );
static void JabberIqResultGetSearchFields( XmlNode *iqNode, void *userdata );
static void JabberSearchFreeData(HWND hwndDlg, JabberSearchData * dat);
static void JabberSearchRefreshFrameScroll(HWND hwndDlg, JabberSearchData * dat);
static int JabbeSearchrRenewFields(HWND hwndDlg, JabberSearchData * dat);
static BOOL CALLBACK JabberSearchAdvancedDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static void JabberSearchDeleteFromRecent(TCHAR * szAddr,BOOL deleteLastFromDB);
static void JabberSearchAddToRecent(TCHAR * szAddr, HWND hwnd);

// Implementation of MAP class (the list 
template <typename _KEYTYPE , int (*COMPARATOR)(_KEYTYPE*, _KEYTYPE*) > 
class UNIQUE_MAP
{
private:
	typedef struct _tagRECORD
	{
		_tagRECORD(_KEYTYPE * key, TCHAR * value=NULL)	{ _key=key; _value=value; _order=0;	}
		_KEYTYPE *_key; 
		TCHAR * _value;
		int _order;
	} _RECORD;

	int _nextOrder;
	LIST<_RECORD> _Records;

	static int _KeysEqual( const _RECORD* p1, const _RECORD* p2 )
	{
		if (COMPARATOR)
			return (int)(COMPARATOR((p1->_key),(p2->_key)));
		else
			return (int) (p1->_key < p2->_key);
	}

	inline int _remove(_RECORD* p)
	{
		int _itemOrder=p->_order;
		if (_Records.remove(p))
		{
			_nextOrder--;
			for (int i=0; i<_Records.getCount(); i++)
			{
				_RECORD * temp=_Records[i];
				if (temp && temp->_order>_itemOrder) 
					temp->_order--;
			}
			return 1;
		}
		return 0;
	}
	inline _RECORD * _getUnorderedRec (int index)
	{
		for (int i=0; i<_Records.getCount(); i++)
		{
			_RECORD * rec=_Records[i];
			if (rec->_order==index)	return rec;
		}
		return NULL;
	}

public:		
	UNIQUE_MAP(int incr):_Records(incr,_KeysEqual)
	{
		_nextOrder=0;
	};
	~UNIQUE_MAP()
	{
		_RECORD * record;
		int i=0;
		while (record=_Records[i++]) delete record;
	}

	int insert(_KEYTYPE* Key, TCHAR *Value)
	{
		_RECORD * rec= new _RECORD(Key,Value);
		int index=_Records.getIndex(rec);
		if (index<0)
		{
			if(!_Records.insert(rec)) delete rec;
			else 
			{
				index=_Records.getIndex(rec);
				rec->_order=_nextOrder++;
			}
		}
		else
		{
			_Records[index]->_value=Value;		
			delete rec;
		}
		return index;
	}
	inline TCHAR* operator[]( _KEYTYPE* _KEY ) const
	{
		_RECORD rec(_KEY);
		int index=_Records.getIndex(&rec);
		_RECORD * rv=_Records[index];		
		if (rv) 
		{
			if (rv->_value)
				return rv->_value;
			else
				return _T("");
		}
		else 
			return NULL;
	}
	inline TCHAR* operator[]( int index ) const
	{
		_RECORD * rv=_Records[index];		
		if (rv) return rv->_value;
		else return NULL;
	}
	inline _KEYTYPE* getKeyName(int index)
	{
		_RECORD * rv=_Records[index];		
		if (rv) return rv->_key;
		else return NULL;
	}
	inline TCHAR * getUnOrdered(int index)
	{
		_RECORD * rec=_getUnorderedRec(index);
		if (rec) return rec->_value;
		else return NULL;
	}
	inline _KEYTYPE * getUnOrderedKeyName(int index)
	{
		_RECORD * rec=_getUnorderedRec(index);
		if (rec) return rec->_key;
		else return NULL;
	}
	inline int getCount() 
	{
		return _Records.getCount();
	}
	inline int removeUnOrdered(int index)
	{
		_RECORD * p=_getUnorderedRec(index);
		if (p) return _remove(p);
		else return 0;
	}
	inline int remove(int index)
	{
		_RECORD * p=_Records[index];
		if (p) return _remove(p);
		else return 0;
	}
	inline int getIndex(_KEYTYPE * key)
	{
		_RECORD temp(key);
		return _Records.getIndex(&temp);
	}
};
