//---------------------------------------------------------------------------
//
// Definitions to silence linker warnings
//
//---------------------------------------------------------------------------

#include <unistd.h>

extern "C" {

//---------------------------------------------------------------------------

int _close(int) { return 0; }
int _fstat(int, struct stat *) { return 0; }
int _isatty(int) { return 0; }
int _lseek(int, off_t, int) { return 0; }

//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
