//---------------------------------------------------------------------------

#include "javelin/orthography.h"
#include "pico_spinlock.h"

//---------------------------------------------------------------------------

#if USE_ORTHOGRAPHY_CACHE

void StenoCompiledOrthography::LockCache() { spinlock19->Lock(); }
void StenoCompiledOrthography::UnlockCache() { spinlock19->Unlock(); }

#endif

//---------------------------------------------------------------------------
