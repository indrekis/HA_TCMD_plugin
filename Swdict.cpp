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
	HA sliding window dictionary
***********************************************************************/

#include "StdAfx.h"
#include "HAEngine.h"

#define HSIZE	16384
#define HSHIFT	3
#define HASH_SW(p) ((b[(p)]^((b[(p)+1]^(b[(p)+2]<<HSHIFT))<<HSHIFT))&(HSIZE-1))
#define MAXCNT	1024	

void HAEngine::swd_cleanup(void) 
{
    if (ccnt!=NULL) free(ccnt),ccnt=NULL;
    if (ll!=NULL) free(ll),ll=NULL;
    if (cr!=NULL) free(cr),cr=NULL;
    if (b!=NULL) free(b),b=NULL;	
    if (best!=NULL) free(best),best=NULL;	
}

void HAEngine::swd_init(U16B maxl, U16B bufl) 
{
    S16B i;
    
    iblen=maxl; 
    cblen=bufl;
    blen=cblen+iblen;
    ll=(U16B *)malloc(blen*sizeof(*ll));
    best=(U16B *)malloc(blen*sizeof(*best));
    ccnt=(U16B *)malloc(HSIZE*sizeof(*ccnt));
    cr=(U16B *)malloc(HSIZE*sizeof(*cr));
    b=(unsigned char *)malloc((blen+iblen-1)*sizeof(*b));
    if (ll==NULL || ccnt==NULL || cr==NULL || b==NULL) {
	swd_cleanup();
	error(1,ERR_MEM,"swd_init()");
    }
    for (i=0;i<HSIZE;++i) ccnt[i]=0; 
    binb=bbf=bbl=inptr=0;
    while (bbl<iblen) {
	if ((i=getbyte())<0) break;
	b[inptr++]=i;
	bbl++;
    }
    swd_mlf=MINLEN-1;
}

#ifndef PCASM

void HAEngine::swd_accept(void) 
{
    S16B i,j;
    
    j=swd_mlf-2;
    do {   		/* Relies on non changed swd_mlf !!! */
	if (binb==cblen) --ccnt[HASH_SW(inptr)];
	else ++binb;
	i=HASH_SW(bbf);
	ll[bbf]=cr[i];
	cr[i]=bbf;
	best[bbf]=30000;
	ccnt[i]++;
	if (++bbf==blen) bbf=0;
	if ((i=getbyte())<0) {
	    --bbl;
	    if(++inptr==blen) inptr=0;
	    continue;
	}
	if (inptr<iblen-1) {
	    b[inptr+blen]=b[inptr]=i;
	    ++inptr;
	}
	else {
	    b[inptr]=i;
	    if(++inptr==blen) inptr=0;
	}
    } while (--j);
    swd_mlf=MINLEN-1;
}

void HAEngine::swd_findbest(void) 
{
    U16B i,ref,cnt,ptr,start_len;
    S16B c;
    
    i=HASH_SW(bbf);
    if ((cnt=ccnt[i]++)>MAXCNT) cnt=MAXCNT;
    ptr=ll[bbf]=cr[i];
    cr[i]=bbf;
    swd_char=b[bbf];
    if ((start_len=swd_mlf)>=bbl) {
	if (bbl==0) swd_char=-1;
	best[bbf]=30000;
    }
    else {
	for (ref=b[bbf+swd_mlf-1];cnt--;ptr=ll[ptr]) {
	    if (b[ptr+swd_mlf-1]==ref && 
		b[ptr]==b[bbf] && b[ptr+1]==b[bbf+1]) {
		{
		    unsigned char *p1=b+ptr+3,*p2=b+bbf+3;
		    for (i=3;i<bbl;++i) {
			if (*p1++!=*p2++) break; 
		    }
		}
		if (i<=swd_mlf) continue;
		swd_bpos=ptr;				
		if ((swd_mlf=i)==bbl || best[ptr]<i) break;
		ref=b[bbf+swd_mlf-1];
	    }
	}
	best[bbf]=swd_mlf;
	if (swd_mlf>start_len) {
	    if (swd_bpos<bbf) swd_bpos=bbf-swd_bpos-1;
	    else swd_bpos=blen-1-swd_bpos+bbf;
	}
    }
    if (binb==cblen) --ccnt[HASH_SW(inptr)];
    else ++binb;
    if (++bbf==blen) bbf=0;
    if ((c=getbyte())<0) {
	--bbl;
	if(++inptr==blen) inptr=0;
	return;
    }
    if (inptr<iblen-1) {
	b[inptr+blen]=b[inptr]=c;
	++inptr;
    }
    else {
	b[inptr]=c;
	if (++inptr==blen) inptr=0;
    }
}

#endif

void HAEngine::swd_dinit(U16B bufl) 
{
    cblen=bufl;
    b=(unsigned char *)malloc(cblen*sizeof(unsigned char));	
    if (b==NULL) {
	swd_cleanup();
	error(1,ERR_MEM,"swd_dinit()");
    }
    bbf=0;
}

void HAEngine::swd_dpair(U16B la, U16B p) 
{
    if (bbf>p) p=bbf-1-p;
    else p=cblen-1-p+bbf;
    while (la--) {
	b[bbf]=b[p];
	putbyte(b[p]);
	if (++bbf==cblen) bbf=0;
	if (++p==cblen) p=0;
    }
}

void HAEngine::swd_dchar(S16B c) 
{
    b[bbf]=c;
    putbyte(c);
    if (++bbf==cblen) bbf=0;
}
// = End of SWDict.cpp =