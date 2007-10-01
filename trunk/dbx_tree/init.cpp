
#include <windows.h>
#include "btree.h"

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved)
{
//	hInstance = hInstDLL;

	CBTree<int, int,4,true> b(NULL);
	b.Insert(1,1);
	return TRUE;
}