// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/***********************************************************************
  This file is part of HAWCX, a archiver plugin for Windows Commander.
  Copyright (C) 1999 Sergey Zharsky  e-mail: zharik@usa.net
  Copyright (C) 2025 Oleg Farenyuk   e-mail: indrekis@gmail.com
***********************************************************************/

/***********************************************************************
  This file is part of HA, a general purpose file archiver.
  Copyright (C) 1995 Harri Hirvola

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
************************************************************************
       HA I/O routines
***********************************************************************/

#include "StdAfx.h"

#include "HAEngine.h"
#include "wcxhead.h"
#include "config.h"

#define uppdcrc(_crc,_c) _crc=(crctab[((int)(_crc)^(_c))&0xff]^((_crc)>>8))
#define CRCMASK		0xffffffffUL
#define CRCP		0xEDB88320UL

void HAEngine::makecrctab(void) 
{
    U16B i,j;
    U32B tv;
	
    for (i=0;i<256;i++) {
	tv=i;
	for (j=8;j>0;j--) {
	    if (tv&1) tv=(tv>>1)^CRCP;
	    else tv>>=1;
	}
	crctab[i]=tv;
    }
}

void HAEngine::setoutput(int fh, int mode, char *name) 
{
    outname=name;
    outspecial=NULL;
    if (fh>=0) write_on=1;
    else write_on=0;
    obl=0;
    ocnt=0;
    outfile=fh;
    w_crc=mode&CRCCALC;
    if (w_crc) {
	if (!crctabok) makecrctab();
	crc=CRCMASK;
    }
    w_progdisp=mode&PROGDISP;
}


void HAEngine::setinput(int fh, int mode, char *name) 
{
    inname=name;
    inspecial=NULL;
    ibl=0;
    icnt=0;
    infile=fh;
    r_crc=mode&CRCCALC;
    if (r_crc) {
	if (!crctabok) makecrctab();
	crc=CRCMASK;
    }
    r_progdisp=mode&PROGDISP;
}


U32B HAEngine::getcrc(void) 
{
    return crc^CRCMASK;
}

void HAEngine::clearcrc(void) 
{
    crc=CRCMASK;
}

void HAEngine::bread(void) 
{
    S16B i;
    unsigned char *ptr;

    if (inspecial!=NULL) {
	ibl=(*inspecial)(ib,BLOCKLEN);
	ibf=0;
	return;
    }
    else {
	ibl=read(infile,ib,BLOCKLEN);
	if (ibl<0) error(1,ERR_READ,inname);
	ibf=0;
    }
    if (ibl) {
	icnt+=ibl;
	if (r_progdisp && m_pWrapper) {
		// this bug was fixed by Christian Ghisler !!! 8-)
		if(!m_pWrapper->ProcessDataProc(inname,ibl))
				error(1,ERR_ABORTED);
	}
	if (r_crc) {
	    for (i=0,ptr=ib;i<ibl;++i) {
		uppdcrc(crc,*(ptr++));
	    }
	}
    }
}

void HAEngine::bwrite(void) 
{
    S16B i;
    unsigned char *ptr;

    if (obl) {
	if (outspecial!=NULL) {
	    (*outspecial)(ob,obl);
	}
	else {
	    if (write_on && write(outfile,ob,obl)!=obl) 
	      error(1,ERR_WRITE,outname);
	    ocnt+=obl;
	    if (w_progdisp && m_pWrapper) {
			if(!m_pWrapper->ProcessDataProc(outname,obl))
				error(1,ERR_ABORTED);
	    }
	    if (w_crc) {
		for (i=0,ptr=ob;i<obl;++i) {
		    uppdcrc(crc,*(ptr++));
		}
	    }
	}
	obl=0;
    }
}



// = end of file haio.cpp =