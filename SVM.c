#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "SVM.h"

/*

Special [Olympics] VM is a 32 bit load/store RISC processor with some periphery.

Registers:

32 32-bit GRP.
IPR - instruction pointer; cannot be referenced directly.
FLR - flags register.
ARR - arithmetic register; contains overflows from MUL etc.
IRR - interrupt return register;
      holds address of instruction where interrupt has occurred;
IFLR - holds value of FLR before interrupt.

FLR layout: [ EQ | LT | GT | OV | IF | PG | ... ]
            0    1    2    3    4    5    6     31       
       ... = reserved

SVM has not FPU, because i'm too lazy to implement one.

Peripheral devices are managed through I/O ports.
`in' and `out instructions' handle port I/O.

Port mappings and hardware control will be described a bit later.

SVM has a MMU with support for paging. Pages are 1 MB in size.
loadpt instruction loads page table from physical address pointing to array of 4096 2-byte entries.
  First 12 bits of each entry is 1MB aligned address and bit 13 is a `present' flag.

SVM interrupt handlers are stored in 256-word long interrupt vector table(like in x86 real mode).
loadivt instruction loads IVT from physical address.

Interrupt mapping is fixed:
nonmaskable interrupts (exceptions):
0 division by zero
1 invalid instruction
2 page fault
3 double fault
4 alignment fault
5-15 reserved

maskable interrupts (can be disabled by IF FLR bit):
16 timer
17 keyboard
18 disk controller
19-31 reserved
32-255 free for use for other devices

Each instruction fits in 32-bit word.
load/store offsets must be aligned to byte/halfword/word addresses, or otherwise alignment fault will occur.

Instructions list:

jump rDst  -- jump indirectly to absolute address
  NOTE: JUMP OFFSETS, unlike load/store data offsets ARE MEASURED IN WORD, NOT IN BYTES
[ 0 | rDst | 0 | ... ]
    5     10   13    

iret  -- jump to address in IRR register, restore FLR from IFLR and IPR from IRR
  NOTE: JUMP OFFSETS, unlike load/store data offsets ARE MEASURED IN WORDS, NOT IN BYTES
[ 0 | ... | 1 | ... ]
    5     10  13

loadir rSrc -- load IRR from rSrc.
[ 0 | rSrc | 2 | ... ]
    5      10  13

storeir rDst -- store IRR into rDst.
[ 0 | rDst | 3 | ... ]
    5      10  13

loadifl rSrc -- load IFLR from rSrc.
[ 0 | rSrc | 4 | ... ]
    5      10  13

storeifl rDst -- store IFLR into rDst.
[ 0 | rDst | 5 | ... ]
    5      10  13

jumpr siOffset  -- jump to address relative to next instruction.
  NOTE: JUMP OFFSETS, unlike load/store data offsets ARE MEASURED IN WORDS, NOT IN BYTES
[ 1 | signed immediate ]
    5

jumpf flrBit rDst -- jump conditionally to absolute address.
  NOTE: JUMP OFFSETS, unlike load/store data offsets ARE MEASURED IN WORDS, NOT IN BYTES
[ 2 | FLR index | rDst | ... ]
    5           7      13

jumpfr flrBit siOffset -- jump conditionally to address relative to next insruction.
  NOTE: JUMP OFFSETS, unlike load/store data offsets ARE MEASURED IN WORDS, NOT IN BYTES
[ 3 | FLR index | signed immediate ]
    5           7

loadil rDst imm -- load immediate to low halfword of rDst.
[ 4 | rDst | 0 | ... | immediate ]
    5      10  14    16

loadilz rDst imm -- load immediate to low halfword of rDst with zero extension.
[ 4 | rDst | 1 | ... | immediate ]
    5      10  14    16

loadils rDst imm -- load immediate to low halfword of rDst with sign extension.
[ 4 | rDst | 2 | ... | immediate ]
    5      10  14    16

loadih rDst imm -- load immediate to high halfword of rDst.
[ 4 | rDst | 3 | ... | immediate ]
    5      10  14    16

loadihz rDst imm -- load immediate to high halfword of rDst while zeroing low halfword.
[ 4 | rDst | 4 | ... | immediate ]
    5     10   14

loadfl rSrc -- load FLR from rSrc.
[ 4 | rSRc | 5 | ... ]
    5     10   14

loadar rSRc -- load ARR from rSrc.
[ 4 | rSrc | 6 | ... ]
    5      10  14

loadivt rSrc -- load iterrupt vector table from address contained in rSrc.
[ 4 | rSrc | 7 | ... ]
    5      10  14

loadpt rSrc -- load page table from address contained in rSrc.
[ 4 | rSrc | 8 | ... ]
    5      10  14

loadb rDst rBase siOffset -- load byte from [rBase + siOffset] into low byte of rDst.
[ 5 | rDst | rBase | 0 | signed immediate ]
    5      10      15  18

loadbz rDst rBase siOffset -- load byte from [rBase + siOffset] into low byte of rDst with zero extension.
[ 5 | rDst | rBase | 1 | signed immediate ]
    5      10      15  18

loadbs rDst rBase siOffset -- load byte from [rBase + siOffset] into low byte of rDst with sign extension.
[ 5 | rDst | rBase | 2 | signed immediate ]
    5      10      15  18

loadh rDst rBase siOffset -- load halfword from [rBase + siOffset] into low halfword of rDst.
[ 5 | rDst | rBase | 3 | signed immediate ]
    5      10      15  18

loadhz rDst rBase siOffset --load halfword from [rBase + siOffset] into low halfword of rDst with zero extension.
[ 5 | rDst | rBase | 4 | signed immediate ]
    5      10      15  18

loadhs rDst rBase siOffset --load halfword from [rBase + siOffset] into low halfword of rDst with sign extension.
[ 5 | rDst | rBase | 5 | signed immediate ]
    5      10      15  18

loadw rDst rBase siOffset -- load word from [rBase + siOffset] into rDst.
[ 5 | rDst | rBase | 6 | signed immediate ]
    5      10      15  18

loadrb rDst siOffset -- load byte from [next IP + siOffset] into low byte of rDst.
[ 6 | rDst | 0 | signed immediate ]
    5      10  13 

loadrbz rDst siOffset -- load byte from [next IP + siOffset] into low byte of rDst with zero extension.
[ 6 | rDst | 1 | signed immediate ]
    5      10  13 

loadrbs rDst siOffset -- load byte from [next IP + siOffset] into low byte of rDst with sign extension.
[ 6 | rDst | 2 | signed immediate ]
    5      10  13 

loadrh rDst siOffset -- load halfword from [next IP + siOffset] into low halfword of rDst.
[ 6 | rDst | 3 | signed immediate ]
    5      10  13 

loadrhz rDst siOffset -- load halfword from [ next IP + siOffset ] into low halfword of rDst with zero extension.
[ 6 | rDst | 4 | signed immediate ]
    5      10  13

loadrhs rDst siOffset -- load halfword from [ next IP + siOffset ] into low halfword of rDst with sign extension.
[ 6 | rDst | 5 | signed immediate ]
    5      10  13

loadrw rDst siOffset -- load word from [ next IP + siOffset ] into rDst.
[ 6 | rDst | 6 | signed immediate ]
    5      10  13

storeb rSrc rBase siOffset -- store byte from rSrc into [ rBase + siOffset ].
[ 7 | rSrc | rBase | 0 | signed immediate ]
    5      10      15  17

storeh rSrc rBase siOffset -- store halfword from rSrc into [ rBase + siOffset ].
[ 7 | rSrc | rBase | 1 | signed immediate ]
    5      10      15  17

storew rSrc rBase siOffset -- store word from rSrc into [ rBase + siOffset ].
[ 7 | rSrc | rBase | 2 | signed immediate ]
    5      10      15  17

storefl rDst -- store FLR into rDst.
[ 7 | rDst | ... | 3 | ... ]
    5      10    15  17

storerb rSrc siOffset -- store byte from rSrc into [ next IP + siOffset ].
[ 8 | rSRc | 0 | signed immediate ]
    5      10  12 

storerh rSrc siOffset -- store halfword from rSrc into [ next IP + siOffset ].
[ 8 | rSRc | 1 | signed immediate ]
    5      10  12

storerw rSrc siOffset -- store word from rSrc into [ next IP + siOffset ].
[ 8 | rSRc | 2 | signed immediate ]
    5      10  12

storear rDst -- store ARR into rDst.
[ 8 | rDst | 3 |  ... ]
    5      10  12

in rDst rPort -- input word from port designated by rPort into rDst.
[ 9 | rDst | rPort | 0 | ... ]
    5      10      15 16

in rDst uiPort -- input word from port designated by uiPort into rDst.
[ 9 | rDst | ... | 1 | unsigned immediate ] 
    5      10    15  16

out rSrc rPort -- output word from rSrc into port designated by rPort.
[ 10 | rSrc | rPort | 0 | ... ]
     5      10      15  16

out rSrc uiPort -- output word from rSrc into port designated by uiPort.
[ 10 | rSrc | ... | 1 | unsigned immediate ]
     5      10    15  16

neg rDst rSrc -- negate signed word from rSrc and store result in rDst.
[ 11 | rDsr | rSrc | ... ]
     5      10     15

add  rDst rSrc1 rSrc2 -- add rSrc1 to rSrc2 and store result in rDst.
  This instruction sets OV flag to 1 on overflow and 0 otherwise.
[ 12 | rDst | rSrc1 | rSrc2 | 0 | ... ]
     5      10     15       20  22

addc rDst rSrc1 rSrc2 -- add rSrc1 to rSrc2 with carrying(i.e. OV) and store result in rDst
  This instruction sets OV flag to 1 on overflow and 0 otherwise.
[ 12 | rDst | rSrc1 | rSrc2 | 1 | ... ]
     5      10     15       20  22

sub rDst rSrc1 rSrc2 -- subtract rSrc2 from rSrc1 and store result in rDst.
  This instruction sets OV flag to 1 on overflow and 0 otherwise.
[ 12 | rDst | rSrc1 | rSrc2 | 2 | ... ]
     5      10     15       20  22

sub rDst rSrc1 rSrc2 -- subtract rSrc2 from rSrc1 with carrying(i.e. OV) and store result in rDst.
  This instruction sets OV flag to 1 on overflow and 0 otherwise.
[ 12 | rDst | rSrc1 | rSrc2 | 3 | ... ]
     5      10     15       20  22

mul rDst rSrc1 rSrc2 -- multiply signed words from rSrc1 and rSrc2.
  Store lower 32 bits in rDst and high 32 bits in ARR.
  This instruction sets OV flag to 1 on overflow and 0 otherwise.
[ 13 | rDst | rSrc1 | rSrc2 | 0 | ... ]
     5      10      15      20  22

mulu rDst rSrc1 rSrc2 -- multiply unsigned words from rSrc1 and rSrc2.
  Store lower 32 bits in rDst and high 32 bits in ARR.
  This instruction sets OV flag to 1 on overflow and 0 otherwise.
[ 13 | rDst | rSrc1 | rSrc2 | 1 | ... ]
     5      10      15      20  22

div rDst rSrc1 rSrc2 -- divide signed 64-bit word from ARR:rSrc1 by signed word in rSrc2.
  Store quotient in rDst and remainder in ARR.
  This instruction sets OV flag to 1 on overflow and 0 otherwise.
  This instruction can cause division by zero interrupt.
[ 13 | rDst | rSrc1 | rSrc2 | 2 | ... ]
     5      10      15      20  22

divu rDst rSrc1 rSrc2 -- divide unsigned 64-bit word from ARR:rSrc1 by unsigned word in rSrc2.
  Store quotient in rDst and remainder in ARR.
  This instruction sets OV flag to 1 on overflow and 0 otherwise.
  This instruction can cause division by zero interrupt.
[ 13 | rDst | rSrc1 | rSrc2 | 3 | ... ]
     5      10      15      20  22

cmp rSrc1 rSrc2 -- compare signed word from rSrc1 to signed word from rSrc2 and set FLR bits.
[ 14 | rSrc1 | rSrc2 | 0 | ... ]
     5       10      15  16

cmpu rSrc1 rSrc2 -- compare unsigned word from rSRc1 to unsigned word from rSRc2 and set FLR bits
[ 14 | rSrc1 | rSrc2 | 1 | ... ].
     5       10      15  16

not rDst rSrc -- bitwise complement rSrc and store result in rDst.
[ 15 | rDst | rSrc | ... ]
     5      10     15

and rDst rSrc1 rSrc2 -- perform bitwise and on operands from rSrc1 and rSrc2 and store result in rDst.
  First 4 bits of FLR is cleared then EQ bit is set according to sources.
[ 16 | rDst | rSRc1 | rSrc2 | ... ]
     5      10      15      20

or rDst rSrc1 rSrc2 -- perform bitwise or on operands from rSrc1 and rSrc2 and store result in rDst.
  First 4 bits of FLR is cleared then EQ bit is set according to sources.
[ 17 | rDst | rSRc1 | rSrc2 | ... ]
     5      10      15      20

xor rDst rSrc1 rSrc2 -- perform bitwise xor on operands from rSrc1 and rSrc2 and store result in rDst.
  First 4 bits of FLR is cleared then EQ bit is set according to sources.
[ 18 | rDst | rSrc1 | rSrc2 | ... ]
     5      10      15      20

shl rDst rSrc1 rSrc2 -- shift unsigned rSrc1 left according to first 6 bits of rSrc2 and store results in rDst.
[ 19 | rDst | rSrc1 | rSRc2 | 0 | ... ]
     5      10      15      20  22

shli rDst rSrc uiShift -- shift unsigned rSrc left according to uiShift and store results in rDst.
[ 19 | rDst | rSrc | ... | 1 | uiShift | ... ]
     5      10     15    20  22        28

sal rDst rSrc1 rSrc2 -- shift signed rSrc1 left according to first 6 bits of rSrc2 and store results in rDst.
  Shifted bits are stored in ARR.
  OV is set according to result.
[ 19 | rDst | rSrc1 | rSRc2 | 2 | ... ]
     5      10      15      20  22

sali rDst rSrc uiShift -- shift signed rSrc left according to uiShift and store results in rDst.
  Shifted bits are stored in ARR.
  OV is set according to result.
[ 19 | rDst | rSrc | ... | 3 | uiShift | ... ]
     5      10     15    20  22        28

shr rDst rSrc1 rSrc2 -- shift unsigned rSrc1 right according to first 6 bits of rSrc2 and store results in rDst.
[ 20 | rDst | rSrc1 | rSRc2 | 0 | ... ]
     5      10      15      20  22

shri rDst rSrc uiShift -- shift unsigned rSrc right according to uiShift and store results in rDst.
[ 20 | rDst | rSrc | ... | 1 | uiShift | ... ]
     5      10     15    20  22        28

sar rDst rSrc1 rSrc2 -- shift signed rSrc1 right according to first 6 bits of rSrc2 and store results in rDst.
  Shifted bits are stored in ARR.
[ 20 | rDst | rSrc1 | rSRc2 | 2 | ... ]
     5      10      15      20  22

sari rDst rSrc uiShift -- shift signed rSrc right according to uiShift and store results in rDst.
  Shifted bits are stored in ARR.
[ 20 | rDst | rSrc | ... | 3 | uiShift | ... ]
     5      10     15    20  22        28

halt -- halts processor execution until interrupt occurs.
[ 21 | ... ]
     5

*/

static SVM_WORD regs[32];
static SVM_WORD ipr;
static SVM_WORD flr;
static SVM_WORD iflr;
static SVM_WORD arr;
static SVM_WORD irr;
static SVM_IR_DESC irStack[3];

static SVM_WORD irVectorTable[256];
static SVM_HALF pageTable[4096];

static SVM_BYTE *memory;
static SVM_WORD memSize;

static clock_t timer;

#define SVM_EQ (0x1)
#define SVM_LT (0x2)
#define SVM_GT (0x4)
#define SVM_OV (0x8)
#define SVM_IF (0x10)
#define SVM_PG (0x20)

SVM_WORD SVMMain()
{
  clock_t newTimer;
  SVM_INT ir = -1, irLevel = 0, key;
  SVM_WORD iAddr;
  SVM_HALF pge;
  memSize = SVMGetMemSize();
  timer = clock();
checkIr:
  if(SVM_IR_DIV0 == ir)
    goto enterUnmaskable;
  else if(SVM_IR_INVALID_INST == ir)
    goto enterUnmaskable;
  else if(SVM_IR_PAGE_FAULT == ir)
    goto enterUnmaskable;
  else if(SVM_IR_DOUBLE_FAULT == ir)
    goto enterUnmaskable;
  else if(SVM_IR_ALIGNMENT_FAULT == ir)
    goto enterUnmaskable;
  else if((newTimer = clock()) - timer > SVM_TIMER_CLOCKS)
  {
    timer = newTimer;
    ir = SVM_IR_TIMER;
    goto enterMaskable;
  }
  else if((key = SVMPeekConsoleInput()) > 0)
  {
    ir = SVM_IR_KEYBOARD;
    goto enterMaskable;
  }
  // TDB: disk controller
  else if(ir > -1 && ir < 16)  
    goto enterUnmaskable;
  else if(ir > 15 && ir < 256)
    goto enterMaskable;
  goto fetch;
enterUnmaskable:
  irStack[irLevel].ir = ir;
  irStack[irLevel].ipr = irr = ipr;
  irStack[irLevel].flr = iflr = flr;
  flr &= ~SVM_IF;
  if(irLevel > 1)
  {
    SVMLogPrint("********** Triple fault! Game over. **********\n"
                "Interrupt was: %d IPR: %08X FLR: %08X\n"
                "Previous interrupt was: %d IPR: %08X FLR: %08X\n"
                "First interrupt was: %d IPR: %08X FLR: %08X\n",
                irStack[2].ir, irStack[2].ipr, irStack[2].flr,
                irStack[1].ir, irStack[1].ipr, irStack[1].flr,
                irStack[0].ir, irStack[0].ipr, irStack[0].flr);
    SVMLogContext();
    SVMLogPrint("**********************************************\n");
    return 0;
  }
  else if(irLevel > 0)
  {
    SVMLogPrint("***** Double fault! Your Kitten of Death awaits! *****\n"
                "Interrupt was: %d IPR: %08X FLR: %08X\n"
                "First interrupt was: %d IPR: %08X FLR: %08X\n",
                irStack[1].ir, irStack[1].ipr, irStack[1].flr,
                irStack[0].ir, irStack[0].ipr, irStack[0].flr);
    SVMLogContext();
    SVMLogPrint("******************************************************\n");
    ipr = irVectorTable[SVM_IR_DOUBLE_FAULT];
    ++irLevel;
    goto fetch;
  }  
  ipr = irVectorTable[ir];
  ++irLevel;
  goto fetch;
enterMaskable:
  if(flr & SVM_IF)
    goto enterUnmaskable;
  goto fetch;
fetch:  
  ir = -1;
  if(flr & SVM_PG)    
  {
    iAddr = ipr & 0xFFFFF;
    pge = pageTable[ipr >> 20];
    if(!(pge & 0x1000))
    {
      ir = SVM_IR_PAGE_FAULT;
      goto checkIr;
    }
    iAddr &= (pge & 0xFFF) << 20;
  }
  else  
    iAddr = ipr;
  if(iAddr > memSize)
  {
    SVMLogPrint("***** IPR out of physical memory range! Machine halted. *****\n");
    SVMLogContext();
    SVMLogPrint("*************************************************************\n");
    return 0;
  }
decode:
  SVMLogPrint("Instruction decoder is not yet implemented.\n");
  return 0;
  goto checkIr;
  return 1;
}

SVM_WORD SVMLogContext()
{
  int i;
  SVMLogPrint("IPR: %08X FLR: %08X ARR: %08X\n", ipr, irr, flr, arr);
  for(i = 0; i < 32; ++i)
  {
    SVMLogPrint("GPR %d: \n", i, regs[i]);
  }
  return 1;
}

SVM_BYTE* SVMInitMemory()
{
  SVM_WORD size = SVMGetMemSize();
  SVM_BYTE *mem = (SVM_BYTE*)malloc(size);
  if(!mem)
  {
      fprintf(stderr,
              "Unable to allocate SVM physical memory (%d MB).\n",
              size >> 20);
  }
  return mem;
}

SVM_WORD SVMCloseMemory()
{
  if(memory)
  {
    free(memory);
    return 1;
  }
  return 0;
}
