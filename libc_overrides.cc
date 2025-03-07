//---------------------------------------------------------------------------
//
// Definitions to improve the performance of standard C functions.
//
//---------------------------------------------------------------------------

#include <string.h>

//---------------------------------------------------------------------------

#if JAVELIN_CPU_CORTEX_M0

extern "C" [[gnu::naked]] size_t strlen(const char *p) {
  // spell-checker: disable
  asm volatile(R"(
    push  {r0, r4}

    lsl   r1, r0, #30
    beq   2f            // p is aligned

  1:
    ldrb  r3, [r0]
    cmp   r3, #0
    beq   5f            // terminating '\0' found

    add   r0, #1
    lsl   r1, r0, #30
    bne   1b            // Loop until aligned

  2:                    // p is aligned
    ldr   r2, =#0x01010101
    lsl   r3, r2, #7

  3:                    // Process 4 bytes
    ldmia r0!, {r1}
    sub   r4, r1, r2
    bic   r4, r1
    and   r4, r3
    beq   3b

  4:                    // Trailer of loop.
    sub   r0, #4
    lsr   r4, #8
    bcs   5f

    add   r0, #1
    lsr   r4, #8
    bcs   5f

    add   r0, #1
    lsr   r4, #8
    bcs   5f

    add   r0, #1

  5:
    pop   {r1, r4}
    sub   r0, r1
    bx    lr
  )");
  // spell-checker: enable
}

#elif JAVELIN_CPU_CORTEX_M33

extern "C" [[gnu::naked]] size_t strlen(const char *p) {
  // spell-checker: disable
  asm volatile(R"(
    add.w r3, r0, #4 // Force wide version to align loop below without nop.
    ldr   r2, =#0xfefefefe

  1: // Process 4 bytes
    ldr     r1, [r0], #4
    uqadd8  r1, r1, r2
    mvns    r1, r1
    beq     1b

    // Trailer of loop.
    rbit  r1, r1
    subs  r0, r3
    clz   r1, r1
    add   r0, r0, r1, lsr #3

    bx    lr
  )");
  // spell-checker: enable
}

#endif

//---------------------------------------------------------------------------
