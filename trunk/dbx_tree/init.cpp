
#include <windows.h>
#include "btree.h"

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved)
{
//	hInstance = hInstDLL;

	CBTree<int, int,4,true> b(NULL);
	CBTree<int, int,4,true>::iterator i = b.Insert(1,1);
	i++;
	i--;
	if (i) i = b.Find(1);
	b.Delete(1);
	b.Delete(i);
		
	return TRUE;
}