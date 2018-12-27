
//#define DEBUG

#include <conio.h>
#include <string.h>
#include <6502.h>

unsigned char cpu_type = 0;
unsigned char ram_banks = 1;
unsigned char vic_pal = 0;

unsigned char screenoff = 0;
unsigned char dtvturbo = 0;
unsigned char c128turbo = 0;
unsigned char scpumode = 1; // default, none, full
unsigned char *ptr;

int count_cpu_names=9;
char *cpu_names[]={"6502 ","65C02","65816","4510 ","65SC*","65CE*","HUC6*","2A0X ","45GS*"};

void teststart(void)
{
    ptr = (unsigned char*)0xe000;
}

void testend(void)
{
    *ptr = 0x60; // rts
}

void waitframe(void) {
    while (((*(unsigned char*)0xd011) & 0x80) == 0) {
        asm ("nop");
    }
    while (((*(unsigned char*)0xd011) & 0x80) != 0) {
        asm ("nop");
    }
}

void delay_200ms(void)
{
    unsigned char n;
    for (n=0;n<10;++n) {
        waitframe();
    }
}

extern void savezpage(void);
extern void loadzpage(void);

extern void dtvturboon(void);
extern void dtvturbooff(void);

extern void set_vic_pal(void);
extern void set_ram_banks(void);

unsigned int dotest(void)
{
    static unsigned int res;
    asm("sei");
    asm("jsr _savezpage");
    asm("ldy #$35");
    asm("sty $01");
    if (dtvturbo) {
        dtvturboon();
    }
    if (screenoff) {
        asm("ldy #$0b");
        asm("sty $d011");
        waitframe();
    }
    if (c128turbo) {
        asm("ldy #$01");
        asm("sty $d030");
    }

    if (cpu_type == CPU_65816) {
        // scpumode
        asm("sty $d07e");
        if (scpumode == 0) {
            asm("sty $d077"); // V1 default (no optimization)
            asm("ldy #%%11000001");
            asm("sty $d0b3"); // V2 default (optimize zp and stack)
        } else if (scpumode == 1) {
            asm("sty $d077"); // V1 (no optimization)
            asm("ldy #%%11000000");
            asm("sty $d0b3"); // V2 (no optimization)
        } else {
            asm("sty $d076"); // V1 (BASIC optimization)
            asm("ldy #%%10000100");
            asm("sty $d0b3"); // V2 (Full optimization)
        }
        asm("sty $d07f");
    }

    asm("ldy #$7f");
    asm("sty $dc0d");
    asm("bit $dc0d");
    asm("ldy #0");
    asm("sty $dc0e");
    asm("ldy #$ff");
    asm("sty $dc05");
    asm("sty $dc04");
//    asm("ldy #%%00011001");
    asm("lda #$21");
    asm("ldy #$19");
    asm("sty $dc0e");
    asm("jsr $e000");
    asm("jsr $e000");
    asm("ldy #0");
    asm("sty $dc0e");
    res = *(unsigned int*)0xdc04;
//    asm("ldx $dc05");
//    asm("lda $dc04");
    if (dtvturbo) {
        dtvturbooff();
    }
    if (c128turbo) {
        asm("ldy #$00");
        asm("sty $d030");
    }

    if (cpu_type == CPU_65816) {
        // scpumode
        asm("sty $d07e");
        asm("sty $d077"); // V1 default (no optimization)
        asm("ldy #%%11000001");
        asm("sty $d0b3"); // V2 default (optimize zp and stack)
        asm("sty $d07f");
    }

    asm("ldy #$1b");
    asm("sty $d011");
    asm("ldy #$36");
    asm("sty $01");
    asm("ldy #$40");
    asm("sty $dc05");
    asm("ldy #$25");
    asm("sty $dc04");
    asm("ldy #$11");
    asm("sty $dc0e");
    asm("ldy #$81");
    asm("sty $dc0d");
    asm("bit $dc0d");
    asm("jsr _loadzpage");
    asm("cli");
    return res;
}

void rtsfunc (void)
{
    asm("rts");
}

unsigned int testjsr(void)
{
    unsigned int i;
    unsigned int a1;
    a1 = (unsigned int)rtsfunc;
    for (i = 0; i < 2000; ++i) {
        *ptr++ = 0x20; // jsr
        *ptr++ = a1 & 0xff; // lo
        *ptr++ = (a1 >> 8) & 0xff; // hi
    }
    return 0;
}

unsigned int testjmp(void)
{
    unsigned int i;
    unsigned int a1;
    for (i = 0; i < 1000; ++i) {
        a1 = ((unsigned int)ptr) + 3;
        *ptr++ = 0x4c; // jmp
        *ptr++ = a1 & 0xff; // lo
        *ptr++ = (a1 >> 8) & 0xff; // hi
        *ptr++ = 0x90; // bcc
        *ptr++ = 0x00;
        *ptr++ = 0xb0; // bcs
        *ptr++ = 0x00;
    }
    return 0;
}

extern void zpcode(void);

unsigned int testcodezp(void)
{
    memcpy(ptr, zpcode, 0xfc);
    ptr += 0xfc;
    return 0;
}

// 4000 * 2 cycles
unsigned int testnops(void)
{
    unsigned int i;
    for (i = 0; i < 4000; ++i) {
        *ptr++ = 0xea; // nop
    }
    return 0;
}

// 2000 * 8 cycles
unsigned int testregops(void)
{
    unsigned int i;
    for (i = 0; i < 2000; ++i) {
        *ptr++ = 0xe8; // inx
        *ptr++ = 0xc8; // iny
        *ptr++ = 0x69; // adc
        *ptr++ = 0x23; // imm
    }
    return 0;
}

// 2000 * 4 cycles
unsigned int testload(void)
{
    unsigned int a1;
    unsigned int i;
    for (i = 0; i < 2000; ++i) {
        a1 = 0x1000 + (i & 0x0fff);
        *ptr++ = 0xad; // lda abs
        *ptr++ = a1 & 0xff; // lo
        *ptr++ = (a1 >> 8) & 0xff; // hi
    }
    return 0;
}

// 2000 * 4 cycles
unsigned int teststore(void)
{
    unsigned int a2;
    unsigned int i;
    for (i = 0; i < 2000; ++i) {
        a2 = 0xfe00 + (i & 0x00ff);
        *ptr++ = 0x8d; // sta abs
        *ptr++ = a2 & 0xff; // lo
        *ptr++ = (a2 >> 8) & 0xff; // hi
    }
    return 0;
}

// 1000 * 8 cycles
unsigned int testmove(void)
{
    unsigned int i;
    unsigned int a1, a2;
    for (i = 0; i < 1000; ++i) {
        a1 = 0x1000 + (i & 0x0fff);
        a2 = 0xfe00 + (i & 0x00ff);
        *ptr++ = 0xad; // lda abs
        *ptr++ = a1 & 0xff; // lo
        *ptr++ = (a1 >> 8) & 0xff; // hi
        *ptr++ = 0x8d; // sta abs
        *ptr++ = a2 & 0xff; // lo
        *ptr++ = (a2 >> 8) & 0xff; // hi
    }
    return 0;
}

// 2000 * 6 cycles
unsigned int testinc(void)
{
    unsigned int a2;
    unsigned int i;
    for (i = 0; i < 2000; ++i) {
        a2 = 0xfe00 + (i & 0x00ff);
        *ptr++ = 0xee; // inc abs
        *ptr++ = a2 & 0xff; // lo
        *ptr++ = (a2 >> 8) & 0xff; // hi
    }
    return 0;
}

// 2000 * 4 cycles
unsigned int testloadzp(void)
{
    unsigned int a1;
    unsigned int i;
    for (i = 0; i < 2000; ++i) {
        a1 = 0x0080 + (i & 0x007f);
        *ptr++ = 0xa5; // lda zp
        *ptr++ = a1 & 0xff; // lo
    }
    return 0;
}

// 2000 * 4 cycles
unsigned int teststorezp(void)
{
    unsigned int a2;
    unsigned int i;
    for (i = 0; i < 2000; ++i) {
        a2 = 0x0080 + (i & 0x007f);
        *ptr++ = 0x85; // sta zp
        *ptr++ = a2 & 0xff; // lo
    }
    return 0;
}

// 1000 * 8 cycles
unsigned int testmovezp(void)
{
    unsigned int i;
    unsigned int a1, a2;
    for (i = 0; i < 1000; ++i) {
        a1 = 0x0080 + (i & 0x007f);
        a2 = 0x0080 + (i & 0x007f) ^ 0x7f;
        *ptr++ = 0xa5; // lda zp
        *ptr++ = a1 & 0xff; // lo
        *ptr++ = 0x85; // sta zp
        *ptr++ = a2 & 0xff; // lo
    }
    return 0;
}

// 2000 * 6 cycles
unsigned int testinczp(void)
{
    unsigned int a2;
    unsigned int i;
    for (i = 0; i < 2000; ++i) {
        a2 = 0x0080 + (i & 0x007f);
        *ptr++ = 0xe6; // inc zp
        *ptr++ = a2 & 0xff; // lo
    }
    return 0;
}

// 2000 * 4 cycles
unsigned int testloadcolram(void)
{
    unsigned int a1;
    unsigned int i;
    for (i = 0; i < 2000; ++i) {
        a1 = 0xd800 + (i & 0x03ff);
        *ptr++ = 0xad; // lda abs
        *ptr++ = a1 & 0xff; // lo
        *ptr++ = (a1 >> 8) & 0xff; // hi
    }
    return 0;
}

// 2000 * 4 cycles
unsigned int teststorecolram(void)
{
    unsigned int a2;
    unsigned int i;
    for (i = 0; i < 2000; ++i) {
        a2 = 0xd800 + (i & 0x03ff);
        *ptr++ = 0x8d; // sta abs
        *ptr++ = a2 & 0xff; // lo
        *ptr++ = (a2 >> 8) & 0xff; // hi
    }
    return 0;
}

// 1000 * 8 cycles
unsigned int testmovecolram(void)
{
    unsigned int i;
    unsigned int a1, a2;
    for (i = 0; i < 1000; ++i) {
        a1 = 0xd800 + (i & 0x03ff);
        a2 = 0xd800 + (i & 0x03ff) ^ 0x3ff;
        *ptr++ = 0xad; // lda abs
        *ptr++ = a1 & 0xff; // lo
        *ptr++ = (a1 >> 8) & 0xff; // hi
        *ptr++ = 0x8d; // sta abs
        *ptr++ = a2 & 0xff; // lo
        *ptr++ = (a2 >> 8) & 0xff; // hi
    }
    return 0;
}

// 2000 * 6 cycles
unsigned int testinccolram(void)
{
    unsigned int a2;
    unsigned int i;
    for (i = 0; i < 2000; ++i) {
        a2 = 0xd800 + (i & 0x03ff);
        *ptr++ = 0xee; // inc abs
        *ptr++ = a2 & 0xff; // lo
        *ptr++ = (a2 >> 8) & 0xff; // hi
    }
    return 0;
}

// 2000 * 4 cycles
unsigned int testloadio(void)
{
    unsigned int a1;
    unsigned int i;
    for (i = 0; i < 2000; ++i) {
        a1 = 0xd020 + (i & 1);
        *ptr++ = 0xad; // lda abs
        *ptr++ = a1 & 0xff; // lo
        *ptr++ = (a1 >> 8) & 0xff; // hi
    }
    return 0;
}

// 2000 * 4 cycles
unsigned int teststoreio(void)
{
    unsigned int a2;
    unsigned int i;
    for (i = 0; i < 2000; ++i) {
        a2 = 0xd020 + (i & 1);
        *ptr++ = 0x8d; // sta abs
        *ptr++ = a2 & 0xff; // lo
        *ptr++ = (a2 >> 8) & 0xff; // hi
    }
    return 0;
}

// 2000 * 6 cycles
unsigned int testincio(void)
{
    unsigned int a2;
    unsigned int i;
    for (i = 0; i < 2000; ++i) {
        a2 = 0xd020 + (i & 1);
        *ptr++ = 0xee; // inc abs
        *ptr++ = a2 & 0xff; // lo
        *ptr++ = (a2 >> 8) & 0xff; // hi
    }
    return 0;
}

// 2000 * 5 cycles
unsigned int testloadlong(void)
{
    unsigned long a1;
    unsigned int i;
    for (i = 0; i < 2000; ++i) {
        a1 = 0x020000 + i;
        *ptr++ = 0xaf; // lda abs long
        *ptr++ = a1 & 0xff; // lo
        *ptr++ = (a1 >> 8) & 0xff; // hi
        *ptr++ = (a1 >> 16) & 0xff; // bank
    }
    return 0;
}

// 2000 * 5 cycles
unsigned int teststorelong(void)
{
    unsigned long a2;
    unsigned int i;
    for (i = 0; i < 2000; ++i) {
        a2 = 0x030000 + i;
        *ptr++ = 0x8f; // sta abs long
        *ptr++ = a2 & 0xff; // lo
        *ptr++ = (a2 >> 8) & 0xff; // hi
        *ptr++ = (a2 >> 16) & 0xff; // bank
    }
    return 0;
}

// 1000 * 10 cycles
unsigned int testmovelong(void)
{
    unsigned int i;
    unsigned long a1, a2;
    for (i = 0; i < 1000; ++i) {
        a1 = 0x020000 + i;
        a2 = 0x030000 + i;
        *ptr++ = 0xaf; // lda abs long
        *ptr++ = a1 & 0xff; // lo
        *ptr++ = (a1 >> 8) & 0xff; // hi
        *ptr++ = (a1 >> 16) & 0xff; // bank
        *ptr++ = 0x8f; // sta abs long
        *ptr++ = a2 & 0xff; // lo
        *ptr++ = (a2 >> 8) & 0xff; // hi
        *ptr++ = (a2 >> 16) & 0xff; // bank
    }
    return 0;
}

void fixscreen (void)
{
    memset((unsigned char*)0xd800, 14, 0x3e8);
    bordercolor(14);
    bgcolor(6);
    textcolor(14);
}

typedef struct
{
    unsigned char cpu_type;
    unsigned char ram_banks;
    char *name;
    unsigned int (*generator)(void);
    unsigned int cycles;
    unsigned int weight;
    unsigned int result;
} TESTINFO;

TESTINFO testinfo[] =
{
    // Any cpu
    {CPU_6502, 1, "nop", testnops, 8000, 8},
    {CPU_6502, 1, "register ops", testregops, 12000, 1},
    {CPU_6502, 1, "function calls", testjsr, 24000, 1},
    {CPU_6502, 1, "jumps and branches", testjmp, 8000, 1},
    {CPU_6502, 1, "code in zeropage", testcodezp, 20696 / 2, 1},
    {CPU_6502, 1, "ram load", testload, 8000, 1},
    {CPU_6502, 1, "ram store", teststore, 8000, 1},
    {CPU_6502, 1, "ram move", testmove, 8000, 1},
    {CPU_6502, 1, "ram rmw", testinc, 12000, 1},
    {CPU_6502, 1, "zeropage load", testloadzp, 6000, 1},
    {CPU_6502, 1, "zeropage store", teststorezp, 6000, 1},
    {CPU_6502, 1, "zeropage move", testmovezp, 6000, 1},
    {CPU_6502, 1, "zeropage rmw", testinczp, 10000, 1},
    {CPU_6502, 1, "color ram load", testloadcolram, 8000, 4},
    {CPU_6502, 1, "color ram store", teststorecolram, 8000, 4},
    {CPU_6502, 1, "color ram move", testmovecolram, 8000, 4},
    {CPU_6502, 1, "color ram rmw", testinccolram, 12000, 8},
    {CPU_6502, 1, "i/o load", testloadio, 8000, 4},
    {CPU_6502, 1, "i/o store", teststoreio, 8000, 4},
    {CPU_6502, 1, "i/o rmw", testincio, 12000, 8},
    // 65816 only, exclude from weighted rating
    {CPU_65816, 4, "long load", testloadlong, 10000, 0},
    {CPU_65816, 4, "long store", teststorelong, 10000, 0},
    {CPU_65816, 4, "long move", testmovelong, 10000, 0},
    {0, 0, NULL, NULL, 0, 0},
};

void printpercent(unsigned int res, unsigned int cycles)
{
    unsigned long r, c;
    unsigned int a,b;

    if (res == 0) {
        return;
    }

    c = ((unsigned long)cycles) * 100L;
    r = ((unsigned long)res);

    a = (c / r) / 100;
    b = (c / r) % 100;

    cprintf("%2d.%02dx", a, b);
}

void printinfo(unsigned char n, unsigned int res)
{
    gotoxy(0,n + 1); cclear(40);
    gotoxy(0,n + 1); cprintf("%-20s %5u   ", testinfo[n].name, res);
    printpercent(res, testinfo[n].cycles * 2);
    fixscreen();
}

void printrating(void)
{
    unsigned long allresult, allcycles;
    unsigned char n;
    unsigned long r, c;
    unsigned int a,b;

    allresult = 0;
    allcycles = 0;

    n = 0;
    while (testinfo[n].generator) {
        if (testinfo[n].weight > 0) {
            allresult += testinfo[n].result / testinfo[n].weight;
            allcycles += (testinfo[n].cycles * 2) / testinfo[n].weight;
        }
        ++n;
    }

    c = ((unsigned long)allcycles) * 100L;
    r = ((unsigned long)allresult);

    a = (c / r) / 100;
    b = (c / r) % 100;

    revers(1);
    gotoxy(0,24); cclear(40);
    gotoxy(0,24); cprintf("Accumulated weighted rating: %2d.%02dx", a, b);
    revers(0);
}

void tests (void)
{
    unsigned int res;
    unsigned char n;
    clrscr();
    revers(1);
           //0123456789012345678901234567890123456789
    cprintf("testing... press any key to abort.      ");
    revers(0);
    n = 0;
    while (testinfo[n].generator) {
        testinfo[n].result = 0;
        if (((cpu_type == testinfo[n].cpu_type) || (testinfo[n].cpu_type == CPU_6502)) && (ram_banks >= testinfo[n].ram_banks)) {
            printinfo(n, 0);
        }
        ++n;
    }

    while(1) {
        n = 0;
        while (testinfo[n].generator) {
            if (((cpu_type == testinfo[n].cpu_type) || (testinfo[n].cpu_type == CPU_6502)) && (ram_banks >= testinfo[n].ram_banks)) {
                gotoxy(39,n + 1); cputc('*');
                teststart();
                testinfo[n].generator();
                testend();
                res = (0xffff - 29) - dotest();
                testinfo[n].result = res;
                printrating();
                printinfo(n, res);
            }
            ++n;
            if (kbhit()) {
                break;
            }
        }
        delay_200ms();
        if (kbhit()) {
            break;
        }
    }
    while (kbhit()) {
        cgetc();
    }
}

void menu (void)
{
    unsigned char ch;
    clrscr();
    revers(1);          //1234567890123456789012345678901234567890
    gotoxy(0,0); cprintf("                        SynthMark64 v0.2");
    gotoxy(0,24); cprintf("CPU %s, RAM %5d KB (%3d BANKS) %s",
        cpu_type < count_cpu_names ? cpu_names[cpu_type] : "?????",
        ram_banks * 64, ram_banks, vic_pal ? "PAL " : "NTSC");
    revers(0);
    while (1) {
        gotoxy(1,2); cprintf("[F1] disable screen during tests: %s", screenoff ? "yes" : "no ");
        gotoxy(1,4); cprintf("[F3] use C128 fast mode: %s", c128turbo ? "yes" : "no ");
        gotoxy(1,6); cprintf("[F5] use DTV fast mode(s): %s", dtvturbo ? "yes" : "no ");
        gotoxy(1,8); cprintf("[F7] use SCPU optimization: %s", scpumode == 0 ? "default" : scpumode == 1 ? "none   " : "full   ");
        gotoxy(1,10); cprintf("[RETURN] start benchmark");
        ch = cgetc();
        if (ch == 0x0d) {
            break;
        }
#ifdef DEBUG
        gotoxy (0,0); cprintf("%02x", ch);
#endif
        switch (ch) {
            case 0x85:
                screenoff ^= 1;
                break;
            case 0x86:
                c128turbo ^= 1;
                break;
            case 0x87:
                dtvturbo ^= 1;
                break;
            case 0x88:
                scpumode++;
                if (scpumode > 2) {
                    scpumode = 0;
                }
                break;
        }
    }
    while (kbhit()) {
        cgetc();
    }
}

void check_ram_banks (void)
{
    if (cpu_type == CPU_65816) {
        set_ram_banks();
    } else {
        ram_banks = 1;
    }
}

void main (void)
{
    cpu_type = getcpu();
    set_vic_pal();
    check_ram_banks();
    fixscreen();
    while (1) {
        menu();
        tests();
    }
}
