//---------------------------------------------------------------------------

#include "javelin/orthography.h"
#include "pico_spinlock.h"

//---------------------------------------------------------------------------

#if USE_ORTHOGRAPHY_CACHE

void StenoCompiledOrthography::LockCache() { spinlock9->Lock(); }
void StenoCompiledOrthography::UnlockCache() { spinlock9->Unlock(); }

#endif

//---------------------------------------------------------------------------
