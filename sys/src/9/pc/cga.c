#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"

enum {
	Black,
	Blue,
	Green,
	Cyan,
	Red,
	Magenta,
	Brown,
	Grey,

	Bright = 0x08,
	Blinking = 0x80,

	Yellow = Bright|Brown,
	White = Bright|Grey,
};
	
enum {
	Width		= 80*2,
	Height		= 25,

	Attr		= (Black<<4)|Grey,	/* high nibble background
						 * low foreground
						 */
};

#define CGASCREENBASE	((uchar*)KADDR(0xB8000))

static int cgapos;
static Lock cgascreenlock;

static Rune cp437[256] = {
	0x2007,0x263A,0x263B,0x2665,0x2666,0x2663,0x2660,0x2022,
	0x25D8,0x25CB,0x25D9,0x2642,0x2640,0x266A,0x266B,0x263C,
	0x25BA,0x25C4,0x2195,0x203C,0x00B6,0x00A7,0x25AC,0x21A8,
	0x2191,0x2193,0x2192,0x2190,0x221F,0x2194,0x25B2,0x25BC,
	0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
	0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
	0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
	0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
	0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
	0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
	0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
	0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
	0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
	0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
	0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
	0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x2302,
	0x00C7,0x00FC,0x00E9,0x00E2,0x00E4,0x00E0,0x00E5,0x00E7,
	0x00EA,0x00EB,0x00E8,0x00EF,0x00EE,0x00EC,0x00C4,0x00C5,
	0x00C9,0x00E6,0x00C6,0x00F4,0x00F6,0x00F2,0x00FB,0x00F9,
	0x00FF,0x00D6,0x00DC,0x00A2,0x00A3,0x00A5,0x20A7,0x0192,
	0x00E1,0x00ED,0x00F3,0x00FA,0x00F1,0x00D1,0x00AA,0x00BA,
	0x00BF,0x2310,0x00AC,0x00BD,0x00BC,0x00A1,0x00AB,0x00BB,
	0x2591,0x2592,0x2593,0x2502,0x2524,0x2561,0x2562,0x2556,
	0x2555,0x2563,0x2551,0x2557,0x255D,0x255C,0x255B,0x2510,
	0x2514,0x2534,0x252C,0x251C,0x2500,0x253C,0x255E,0x255F,
	0x255A,0x2554,0x2569,0x2566,0x2560,0x2550,0x256C,0x2567,
	0x2568,0x2564,0x2565,0x2559,0x2558,0x2552,0x2553,0x256B,
	0x256A,0x2518,0x250C,0x2588,0x2584,0x258C,0x2590,0x2580,
	0x03B1,0x00DF,0x0393,0x03C0,0x03A3,0x03C3,0x00B5,0x03C4,
	0x03A6,0x0398,0x03A9,0x03B4,0x221E,0x03C6,0x03B5,0x2229,
	0x2261,0x00B1,0x2265,0x2264,0x2320,0x2321,0x00F7,0x2248,
	0x00B0,0x2219,0x00B7,0x221A,0x207F,0x00B2,0x25A0,0x00A0,
};

static uchar
cgaregr(int index)
{
	outb(0x3D4, index);
	return inb(0x3D4+1) & 0xFF;
}

static void
cgaregw(int index, int data)
{
	outb(0x3D4, index);
	outb(0x3D4+1, data);
}

static void
movecursor(void)
{
	cgaregw(0x0E, (cgapos/2>>8) & 0xFF);
	cgaregw(0x0F, cgapos/2 & 0xFF);
	CGASCREENBASE[cgapos+1] = Attr;
}

static void
cgascreenputc(Rune c)
{
	int i;
	uchar *p;

	if(c == '\n'){
		cgapos = cgapos/Width;
		cgapos = (cgapos+1)*Width;
	}
	else if(c == '\t'){
		i = 8 - ((cgapos/2)&7);
		while(i-->0)
			cgascreenputc(' ');
	}
	else if(c == '\b'){
		if(cgapos >= 2)
			cgapos -= 2;
		cgascreenputc(' ');
		cgapos -= 2;
	}
	else{
		for(i=0; i<nelem(cp437); i++)
			if(cp437[i] == c)
				break;
		if(i == nelem(cp437))
			i = 0xfe; /* peter */
		CGASCREENBASE[cgapos++] = i;
		CGASCREENBASE[cgapos++] = Attr;
	}
	if(cgapos >= Width*Height){
		memmove(CGASCREENBASE, &CGASCREENBASE[Width], Width*(Height-1));
		p = &CGASCREENBASE[Width*(Height-1)];
		for(i=0; i<Width/2; i++){
			*p++ = ' ';
			*p++ = Attr;
		}
		cgapos = Width*(Height-1);
	}
	movecursor();
}

static void
cgascreenputs(char* s, int n)
{
	Rune r;
	int i;

	if(!islo()){
		/*
		 * Don't deadlock trying to
		 * print in an interrupt.
		 */
		if(!canlock(&cgascreenlock))
			return;
	}
	else
		lock(&cgascreenlock);

	while(n > 0){
		i = chartorune(&r, s);
		cgascreenputc(r);
		s += i;
		n -= i;
	}

	unlock(&cgascreenlock);
}

void
screeninit(void)
{

	cgapos = cgaregr(0x0E)<<8;
	cgapos |= cgaregr(0x0F);
	cgapos *= 2;

	screenputs = cgascreenputs;
}

