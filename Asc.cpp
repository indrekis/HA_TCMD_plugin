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
	HA ASC method
***********************************************************************/

#include "StdAfx.h"

#include "HAEngine.h"

void HAEngine::asc_cleanup(void) 
{
    swd_cleanup();
}

void HAEngine::tabinit(U16B t[], U16B tl, U16B ival) 
{
    U16B i,j;
    
    for (i=tl;i<2*tl;++i) 
		t[i]=ival;

    for (i=tl-1,j=(tl<<1)-2;i;--i,j-=2) 
	{
		t[i]=t[j]+t[j+1];
    }
}

void HAEngine::tscale(U16B t[], U16B tl) 
{
    U16B i,j;
    
    for (i=(tl<<1)-1;i>=tl;--i) {
	if (t[i]>1) t[i]>>=1;
    }
    for (i=tl-1,j=(tl<<1)-2;i;--i,j-=2) {
	t[i]=t[j]+t[j+1];
    }
}

void HAEngine::tupd(U16B t[], U16B tl, U16B maxt, U16B step, U16B p) {

    S16B i;
    
    for (i=p+tl;i;i>>=1) t[i]+=step;
    if (t[1]>=maxt) tscale(t,tl);
}

void HAEngine::tzero(U16B t[], U16B tl, U16B p) 
{
    S16B i,step;
    for (i=p+tl,step=t[i];i;i>>=1) 
		t[i]-=step;
}

void HAEngine::model_init(void) 
{

    S16B i;
    
    ces=CTSTEP;
    les=LTSTEP;
    ccnt_asc=0;
    ttcon=0;
    npt=pmax=1;
    for (i=0;i<TTORD;++i) ttab[i][0]=ttab[i][1]=TTSTEP;
    tabinit(ltab,LTCODES,0);
    tabinit(eltab,LTCODES,1);
    tabinit(ctab,CTCODES,0);
    tabinit(ectab,CTCODES,1);
    tabinit(ptab,PTCODES,0);	
    tupd(ptab,PTCODES,MAXPT,PTSTEP,0);
}

void HAEngine::pack_init(void) 
{
    model_init();
    ac_init_encode();
}

void HAEngine::unpack_init(void) 
{
    model_init();
    ac_init_decode();
}

void HAEngine::ttscale(U16B con) 
{
    ttab[con][0]>>=1;
    if (ttab[con][0]==0) ttab[con][0]=1;
    ttab[con][1]>>=1;
    if (ttab[con][1]==0) ttab[con][1]=1;
}

void HAEngine::codepair(S16B la, S16B p) 
{
    U16B i,j,lt,k,cf,tot;
    
    i=ttab[ttcon][0]+ttab[ttcon][1];
    ac_out(ttab[ttcon][0],i,i+1);
    ttab[ttcon][1]+=TTSTEP;
    if (i>=MAXTT) ttscale(ttcon);
    ttcon=((ttcon<<1)|1)&TTOMASK;
    while (ccnt_asc>pmax) {
	tupd(ptab,PTCODES,MAXPT,PTSTEP,npt++);
	pmax<<=1;	
    }

    for (i=p,j=0;i;++j,i>>=1); 
    cf=ptab[PTCODES+j];
    tot=ptab[1];
    for (lt=0,i=PTCODES+j;i;i>>=1) {
	if (i&1) lt+=ptab[i-1];
	ptab[i]+=PTSTEP;
    }
    if (ptab[1]>=MAXPT) tscale(ptab,PTCODES);
    ac_out(lt,lt+cf,tot);
    if (p>1) {
	for (i=0x8000U;!(p&i);i>>=1);
	j=p&~i;
	if (i!=(pmax>>1)) ac_out(j,j+1,i);
	else ac_out(j,j+1,ccnt_asc-(pmax>>1));
    }
    i=la-MINLEN;
    if (i==LENCODES-1) i=SLCODES-1,j=0xffff;
    else if (i<SLCODES-1) j=0xffff; 
    else {
	j=(i-SLCODES+1)&LLMASK;
	i=((i-SLCODES+1)>>LLBITS)+SLCODES;	
    }
    if ((cf=ltab[LTCODES+i])==0) {
	ac_out(ltab[1],ltab[1]+les,ltab[1]+les);	
	for (lt=0,k=LTCODES+i;k;k>>=1) {
	    if (k&1) lt+=eltab[k-1];
	    ltab[k]+=LTSTEP;
	}
	if (ltab[1]>=MAXLT) tscale(ltab,LTCODES);
	ac_out(lt,lt+eltab[LTCODES+i],eltab[1]);
	tzero(eltab,LTCODES,i);
	if (eltab[1]!=0) les+=LTSTEP;
	else les=0;
	for (k=i<=LPLEN?0:i-LPLEN;
	     k<(i+LPLEN>=LTCODES-1?LTCODES-1:i+LPLEN);++k) {
	    if (eltab[LTCODES+k]) tupd(eltab,LTCODES,MAXLT,1,k);
	}
    }
    else {
	tot=ltab[1]+les;
	for (lt=0,k=LTCODES+i;k;k>>=1) {
	    if (k&1) lt+=ltab[k-1];
	    ltab[k]+=LTSTEP;
	}
	if (ltab[1]>=MAXLT) tscale(ltab,LTCODES);
	ac_out(lt,lt+cf,tot);
    }
    if (ltab[LTCODES+i]==LCUTOFF) les-=LTSTEP<les?LTSTEP:les-1; 
    if (j!=0xffff) ac_out(j,j+1,LLLEN); 
    if (ccnt_asc<POSCODES) {
	ccnt_asc+=la;
	if (ccnt_asc>POSCODES) ccnt_asc=POSCODES;
    }
}


void HAEngine::codechar(S16B c) 
{
    U16B i,lt,tot,cf;

    i=ttab[ttcon][0]+ttab[ttcon][1];
    ac_out(0,ttab[ttcon][0],i+1);
    ttab[ttcon][0]+=TTSTEP;
    if (i>=MAXTT) ttscale(ttcon);
    ttcon=(ttcon<<1)&TTOMASK;
    if ((cf=ctab[CTCODES+c])==0) {
	ac_out(ctab[1],ctab[1]+ces,ctab[1]+ces);
	for (lt=0,i=CTCODES+c;i;i>>=1) {
	    if (i&1) lt+=ectab[i-1];
	    ctab[i]+=CTSTEP;
	}
	if (ctab[1]>=MAXCT) tscale(ctab,CTCODES);
	ac_out(lt,lt+ectab[CTCODES+c],ectab[1]);
	tzero(ectab,CTCODES,c);
	if (ectab[1]!=0) ces+=CTSTEP;
	else ces=0;
	for (i=c<=CPLEN?0:c-CPLEN;
	     i<(c+CPLEN>=CTCODES-1?CTCODES-1:c+CPLEN);++i) {
	    if (ectab[CTCODES+i]) tupd(ectab,CTCODES,MAXCT,1,i);
	}
    }
    else {
	tot=ctab[1]+ces;
	for (lt=0,i=CTCODES+c;i;i>>=1) {
	    if (i&1) lt+=ctab[i-1];
	    ctab[i]+=CTSTEP;
	}
	if (ctab[1]>=MAXCT) tscale(ctab,CTCODES);
	ac_out(lt,lt+cf,tot);
    }
    if (ctab[CTCODES+c]==CCUTOFF) ces-=CTSTEP<ces?CTSTEP:ces-1; 
    if (ccnt_asc<POSCODES) ++ccnt_asc;
}


void HAEngine::asc_pack(void) 
{
    
    S16B oc;
    U16B omlf,obpos;
    
    swd_init(LENCODES+MINLEN-1,POSCODES);
    pack_init();
    for (swd_findbest();swd_char>=0;) 
	{
		if (swd_mlf>MINLEN || (swd_mlf==MINLEN && swd_bpos<MINLENLIM)) 
		{
			omlf=swd_mlf;
			obpos=swd_bpos;
			oc=swd_char;
			swd_findbest();
			if (swd_mlf>omlf) 
				codechar(oc);
			else 
			{
				swd_accept();
				codepair(omlf,obpos);
				swd_findbest();
			}
		}
		else 
		{
			swd_mlf=MINLEN-1;
			codechar(swd_char);
			swd_findbest();
		}
    }

    ac_out(ttab[ttcon][0]+ttab[ttcon][1],
	   ttab[ttcon][0]+ttab[ttcon][1]+1,ttab[ttcon][0]+ttab[ttcon][1]+1);
    ac_end_encode();
    asc_cleanup();
}


void HAEngine::asc_unpack(void) 
{
    U16B li,p,tv,i,lt;
    
    swd_dinit(POSCODES);
    unpack_init();
    for (;;) 
	{
		tv=ac_threshold_val(ttab[ttcon][0]+ttab[ttcon][1]+1);		
		i=ttab[ttcon][0]+ttab[ttcon][1];
		if (ttab[ttcon][0]>tv) 
		{
			ac_in(0,ttab[ttcon][0],i+1);
			ttab[ttcon][0]+=TTSTEP;
			if (i>=MAXTT) 
				ttscale(ttcon);

			ttcon=(ttcon<<1)&TTOMASK;
			tv=ac_threshold_val(ctab[1]+ces);
			if (tv>=ctab[1]) 
			{
				ac_in(ctab[1],ctab[1]+ces,ctab[1]+ces);
				tv=ac_threshold_val(ectab[1]);
				for (li=2,lt=0;;) 
				{
					if (lt+ectab[li]<=tv) 
					{
						lt+=ectab[li];
						++li;
					}
					if (li>=CTCODES) 
					{
						li-=CTCODES;
						break;
					}
				    li<<=1;
				}

				ac_in(lt,lt+ectab[CTCODES+li],ectab[1]);
				tzero(ectab,CTCODES,li);
				if (ectab[1]!=0) 
					ces+=CTSTEP;
				else 
					ces=0;
				for (i=li<CPLEN?0:li-CPLEN;
						i<(li+CPLEN>=CTCODES-1?CTCODES-1:li+CPLEN);++i) 
				{
					if (ectab[CTCODES+i]) 
						tupd(ectab,CTCODES,MAXCT,1,i);
				}
			}
			else 
			{
				for (li=2,lt=0;;) 
				{
					if (lt+ctab[li]<=tv) 
					{
						lt+=ctab[li];
						li++;
					}
					if (li>=CTCODES) 
					{
						li-=CTCODES;
						break;
					}
					li<<=1;
				}
				ac_in(lt,lt+ctab[CTCODES+li],ctab[1]+ces);
			}
			tupd(ctab,CTCODES,MAXCT,CTSTEP,li);
			if (ctab[CTCODES+li]==CCUTOFF) 
				ces-=CTSTEP<ces?CTSTEP:ces-1; 

			swd_dchar(li);
			if (ccnt_asc<POSCODES) 
				++ccnt_asc;
		}
		else 
			if (i>tv) 
			{
				ac_in(ttab[ttcon][0],i,i+1);
				ttab[ttcon][1]+=TTSTEP;
				if (i>=MAXTT) 
					ttscale(ttcon);
				ttcon=((ttcon<<1)|1)&TTOMASK;

				while (ccnt_asc>pmax) 
				{
					tupd(ptab,PTCODES,MAXPT,PTSTEP,npt++);
					pmax<<=1;	
				}

				tv=ac_threshold_val(ptab[1]);
				for (p=2,lt=0;;) 
				{
					if (lt+ptab[p]<=tv) 
					{
						lt+=ptab[p];
						p++;
					}

					if (p>=PTCODES) 
					{
						p-=PTCODES;
						break;
					}
					p<<=1;
				}

				ac_in(lt,lt+ptab[PTCODES+p],ptab[1]);
				tupd(ptab,PTCODES,MAXPT,PTSTEP,p);
				if (p>1) 
				{
					for (i=1;p;i<<=1,--p);
					i>>=1;
					if (i==(pmax>>1)) 
						li=ccnt_asc-(pmax>>1);
					else 
						li=i;

					p=ac_threshold_val(li);
					ac_in(p,p+1,li);
					p+=i;
				}
				tv=ac_threshold_val(ltab[1]+les);
				if (tv>=ltab[1]) 
				{
					ac_in(ltab[1],ltab[1]+les,ltab[1]+les);
					tv=ac_threshold_val(eltab[1]);
					for (li=2,lt=0;;) 
					{
						if (lt+eltab[li]<=tv) 
						{
							lt+=eltab[li];
							++li;
						}

						if (li>=LTCODES) 
						{
							li-=LTCODES;
							break;
						}
						li<<=1;
					}	

					ac_in(lt,lt+eltab[LTCODES+li],eltab[1]);
					tzero(eltab,LTCODES,li);
					if (eltab[1]!=0)
						les+=LTSTEP;
					else 
						les=0;

					for (i=li<LPLEN?0:li-LPLEN;
							i<(li+LPLEN>=LTCODES-1?LTCODES-1:li+LPLEN);++i) 
					{
						if (eltab[LTCODES+i]) 
							tupd(eltab,LTCODES,MAXLT,1,i);
					}
				}
				else 
				{
					for (li=2,lt=0;;) 
					{
						if (lt+ltab[li]<=tv) 
						{
							lt+=ltab[li];
							++li;
						}
						if (li>=LTCODES) 
						{
							li-=LTCODES;
							break;
						}
					    li<<=1;
					}
					ac_in(lt,lt+ltab[LTCODES+li],ltab[1]+les);
				}

				tupd(ltab,LTCODES,MAXLT,LTSTEP,li);
				if (ltab[LTCODES+li]==LCUTOFF)
					les-=LTSTEP<les?LTSTEP:les-1; 

				if (li==SLCODES-1) 
					li=LENCODES-1;
			    else 
					if (li>=SLCODES) 
					{
						i=ac_threshold_val(LLLEN);
						ac_in(i,i+1,LLLEN);
						li=((li-SLCODES)<<LLBITS)+i+SLCODES-1;
					}
				li+=3;
				if (ccnt_asc<POSCODES) 
				{
					ccnt_asc+=li;
					if (ccnt_asc>POSCODES) 
						ccnt_asc=POSCODES;
				}
				swd_dpair(li,p);				
			}
			else 
			{
				ac_in(i,i+1,i+1);
				flush();
				asc_cleanup();
				return;
			}
    }
}
 

// = end of file asc.cpp =