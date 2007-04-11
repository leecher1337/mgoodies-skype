#define MS_DBV_VIRTUALIZE "DB3XV/Virtualize"
// wParam: 0: If DB is real - virtualise it, if it is virtual - realize it;
// wParam: 1: If DB is real - virtualise it;
// wParam: 2: if it is virtual - realize it;
// wParam: else: do nothing - just return the state
// lParam is not used;
// returns the virtual state;

#define MS_DBV_SAVEFILE "DB3XV/SaveFile"
// wParam: null: ask for file
// wParam: char *: the filename
// wparam: -1: write to current db, but stay virtual
// lParam is not used
// returns TRUE on success; FALSE on error