/***********************************************************************
  This file is part of HAWCX, a archiver plugin for Windows Commander.
  Copyright (C) 1999 Sergey Zharsky  e-mail: zharik@usa.net
  Copyright (C) 2025 Oleg Farenyuk   e-mail: indrekis@gmail.com
***********************************************************************
  HAEngine.h - class HAEngine declaration
***********************************************************************/

#ifndef _HAENGINE_H_
	#define _HAENGINE_H_

#include <cstdint>

// typedef short int S16B;
// typedef unsigned short int U16B;
// typedef long S32B;
// typedef unsigned long U32B;
typedef int16_t  S16B;
typedef uint16_t U16B;
typedef int32_t  S32B;
typedef uint32_t U32B;

//ASC defines
#define POSCODES 31200
#define SLCODES	16
#define LLCODES 48
#define LLLEN	16
#define LLBITS	4
#define LLMASK	(LLLEN-1)
#define LENCODES (SLCODES+LLCODES*LLLEN)
#define LTCODES (SLCODES+LLCODES)
#define CTCODES 256
#define PTCODES 16
#define LTSTEP 8
#define MAXLT (750*LTSTEP)
#define CTSTEP 1
#define MAXCT (1000*CTSTEP)
#define PTSTEP 24
#define MAXPT (250*PTSTEP) 
#define TTSTEP 40
#define MAXTT (150*TTSTEP)
#define TTORD 4
#define TTOMASK (TTORD-1);
#define LCUTOFF (3*LTSTEP)
#define CCUTOFF (3*CTSTEP)
#define CPLEN 8
#define LPLEN 4
#define MINLENLIM 4096

// SWDICT defines
#define MINLEN 	3	/* Minimum possible match lenght for this */
					/* implementation */

//HAIO defines
#define BLOCKLEN 	8192

#define getbyte() (ibl>0?(--ibl,ib[ibf++]):(bread(),(ibl>0?--ibl,ib[ibf++]:-1)))
#define putbyte(c) {ob[obl++]=(c);if(obl==BLOCKLEN)bwrite();}
#define flush() bwrite()

#define CRCCALC		1	/* flag to setinput/setoutput */
#define PROGDISP	2	/* flog to setinput/setoutput */

//	HSC Finite context model

#define HASH(s,l,h)	{				          \
			  h=0;                                    \
			  if (l) h=hrt[s[0]];                     \
			  if (l>1) h=hrt[(s[1]+h)&(HTLEN-1)];     \
			  if (l>2) h=hrt[(s[2]+h)&(HTLEN-1)];     \
			  if (l>3) h=hrt[(s[3]+h)&(HTLEN-1)];     \
			}                                                     

#define move_context(c) curcon[3]=curcon[2],curcon[2]=curcon[1], \
			curcon[1]=curcon[0],curcon[0]=c

// ERROR defines
#define	ERR_UNKNOWN	1	/* Unknown error 			*/
#define ERR_NOTIMP	2	/* Command not implemented 		*/
#define ERR_ARCOPEN	3	/* Could not open archive		*/
#define ERR_MEM		4	/* Out of memory			*/
#define ERR_NOHA	5	/* Not a ha archive			*/
#define ERR_WRITE	6	/* Write error				*/
#define ERR_READ	7	/* Read error 				*/
#define ERR_INT		8	/* Got signal...			*/
#define ERR_NOFILES	9	/* No files found			*/
#define ERR_REMOVE	10	/* Could not remove 			*/
#define ERR_INVSW	11	/* Invalid switch			*/
#define ERR_TOONEW	12	/* Version identifier too high		*/
#define ERR_TOOOLD	13	/* Version identifier too old		*/
#define ERR_UNKMET	14	/* Unknown compression method		*/
#define ERR_SEEK	15	/* Lseek error				*/
#define ERR_OPEN	16	/* Could not open file			*/
#define ERR_MKDIR	17	/* Could not make directory		*/
#define ERR_CRC		18	/* CRC error 				*/
#define ERR_WRITENN	19	/* Write error (no name)		*/
#define ERR_STAT	20	/* Stat failed				*/
#define ERR_DIROPEN     21      /* Open dir                             */
#define ERR_CORRUPTED   22      /* Corrupted archive                    */
#define ERR_SIZE        23      /* Wrong data type size                 */
#define ERR_HOW         24      /* How to handle                        */
#define ERR_RDLINK      25      /* Readlink() error                     */
#define ERR_MKLINK      26      /* Symlinklink() error                  */
#define ERR_MKFIFO      27      /* Mkfifo() error                       */

#define ERR_ABORTED     28      /* Processing aborted                   */

// CleanUp defines
#define CU_CANRELAX    0x01
#define CU_RELAXED     0x02
#define CU_FUNC        0x04
#define CU_RMFILE      0x08
#define CU_RMDIR       0x10

//ARCHIVE definitions

#define MYVER	2			/* Version info in archives 	*/
#define LOWVER	2			/* Lowest supported version 	*/

#define ARC_OLD	0			/* Mode flags for arc_open()	*/
#define ARC_NEW 1		
#define ARC_RDO 2

#define AO_FLAGS (O_RDWR|O_BINARY)
#define AO_RDOFLAGS (O_RDONLY|O_BINARY)
#define AC_FLAGS O_RDWR|O_BINARY|O_CREAT,S_IREAD|S_IWRITE

#define STRING	300

#define MSDOSMDH	1		/* Identifiers for machine 	*/
#define UNIXMDH		2		/*   specific header data 	*/

typedef struct {			/* Header of file in archive 	*/
    unsigned char type;
    unsigned char ver;
    U32B clen;
    U32B olen;
    U32B time;
    U32B crc;
    char path[_MAX_PATH];
    char name[_MAX_PATH];
	U16B mdilen;
	U16B mylen;
	//DWORD attr;
	unsigned char attr;
} Fheader;

enum PackMode {M_CPY=0,M_ASC,M_HSC,M_UNK,M_DIR=14,M_SPECIAL};

struct ArchiveRec;
 
class HAEngine
{
	ArchiveRec *m_pWrapper;
public:
	HAEngine();

	static HAEngine *NewHAEngine(){return new HAEngine();};

	char * ArcName(){return arcname;};

	void ExtractFile(char *ofname);
	void TestArchive(char *ofname);

	Fheader *CurrentFile(){return &fhdr;}

	void SetWrapper(ArchiveRec *wrapper){m_pWrapper=wrapper;}

private:

	void pack(void);
	void unpack(void);
	void cleanup(bool bUnpack);

	//ARCHIVE section
	// data
	int arcfile = 0;
	char arcname[_MAX_PATH] = {};
	struct stat arcstat = {};
	unsigned arccnt = 0;
	int dirty = 0, addtries = 0;
	U32B nextheader = 0, thisheader = 0, arcsize = 0, bestpos = 0, trypos = 0;
	Fheader fhdr{};
	Fheader newhdr{};

	//ARCHIVE functions - helpers
	U32B getvalue(int len);
	void putvalue(U32B val, int len);
	char *getstring(char *);
	void putstring(char *string);
	Fheader *getheader(void);
	void putheader(Fheader *hd);
	void arc_clean(void);
	U32B arc_scan(void);
	void delold(void);

public:
	//ARCHIVE functions
	int arc_open(char *arcname, int mode);
	void arc_reset(void);	
	Fheader *arc_seek(void);
	void arc_delete(void);
	void arc_newfile(char *mdpath, char *name,struct _finddata_t *ft);
	void arc_accept(int method);
	void arc_trynext(void);
	int arc_addfile(void);
	void arc_close(void);
	int addfile(char *path, char *name,int move,int usepath);

private:
	// HAIO section
	void makecrctab(void);
	void setoutput(int fh, int mode, char *name);
	void setinput(int fh, int mode, char *name);
	U32B getcrc(void);
	void clearcrc(void);
	void bread(void);
	void bwrite(void);

	// HAIO data section
	int ibl = 0, ibf = 0, obl = 0;
	unsigned char ib[BLOCKLEN] = {}, ob[BLOCKLEN] = {};
	U32B icnt = 0, ocnt = 0, totalsize = 0;

	int infile = 0, outfile = 0;
	U32B crc = 0;
	U32B crctab[256] = {};
	unsigned char r_crc = 0, w_crc = 0, r_progdisp = 0, w_progdisp = 0;
	int write_on = 0, crctabok = 0;
	char *outname = nullptr, *inname = nullptr;

	void (*outspecial)(unsigned char *obuf, unsigned oblen) = nullptr;
	unsigned (*inspecial)(unsigned char *ibuf, unsigned iblen) = nullptr;

	// SWDICT section
	void swd_init(U16B maxl, U16B bufl);	/* maxl=max len to be found  */
											/* bufl=dictionary buffer len */
											/* bufl+2*maxl-1<32768 !!! */
	void swd_cleanup(void);
	void swd_accept(void);
	void swd_findbest(void);
	void swd_dinit(U16B bufl);
	void swd_dpair(U16B l, U16B p);
	void swd_dchar(S16B c);

	//SWDICT data section
	U16B swd_bpos = 0, swd_mlf = 0;
	S16B swd_char = 0;
	U16B cblen = 0, binb = 0;
	U16B bbf = 0, bbl = 0, inptr = 0;
	U16B *ccnt = nullptr, *ll = nullptr, *cr = nullptr, *best = nullptr;
	unsigned char *b = nullptr;
	U16B blen = 0, iblen = 0;

	//ACODER section
	// ACODER data
	U16B h = 0, l = 0, v = 0;
	S16B s = 0;
	S16B gpat = 0, ppat = 0;

	// ACODER funcs
	void ac_init_encode(void);
	void ac_end_encode(void);
	void ac_init_decode(void);
	void ac_out(U16B low, U16B high, U16B tot);
	U16B ac_threshold_val(U16B tot);
	void ac_in(U16B low, U16B high, U16B tot);

	// CPY Section
	void cpy(void);

	// ASC Section
	// ASC data
	U16B ltab[2 * LTCODES] = {};
	U16B eltab[2*LTCODES] = {};
	U16B ptab[2*PTCODES] = {};
	U16B ctab[2*CTCODES] = {};
	U16B ectab[2*CTCODES] = {};
	U16B ttab[TTORD][2] = {};
	U16B ccnt_asc = 0, pmax = 0, npt = 0;
	U16B ces = 0;
	U16B les = 0;
	U16B ttcon = 0;

	// ASC function
	// helpers
	void tabinit(U16B t[], U16B tl, U16B ival); 
	void tscale(U16B t[], U16B tl);
	void tupd(U16B t[], U16B tl, U16B maxt, U16B step, U16B p);
	void tzero(U16B t[], U16B tl, U16B p);
	void model_init(void);
	void pack_init(void);
	void unpack_init(void);
	void ttscale(U16B con);
	void codepair(S16B l, S16B p);
	void codechar(S16B c);

	// ASC main function
	void asc_pack(void);
	void asc_unpack(void);
	void asc_cleanup(void);

	// HSC Section
	// function helpers
	void init_model(void);
	void init_pack(void);
	void init_unpack(void);
	void release_cfblocks(void);
	U16B make_context(unsigned char conlen, S16B c);
	void el_movefront(U16B cp);
	void add_model(S16B c);
	U16B find_next(void);
	U16B find_longest(void);
	U16B adj_escape_prob(U16B esc, U16B cp);
	S16B code_first(U16B cp, S16B c);
	S16B code_rest(U16B cp, S16B c);
	void code_new(S16B c);
	S16B decode_first(U16B cp);
	S16B decode_rest(U16B cp);
	S16B decode_new(void);

	// HSC main functions
	void hsc_pack(void);
	void hsc_unpack(void);
	void hsc_cleanup(void);

	// ERROR section
	void error(int fatal, int number, ...);
};

#endif //_HAENGINE_H_

// = End of file HAEngine.h =