/***********************************************************************
  This file is part of HAWCX, a archiver plugin for Windows Commander.
  Copyright (C) 1999 Sergey Zharsky  e-mail:zharik@usa.net
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
	HA archive handling
***********************************************************************/

#include "StdAfx.h"

#include "HAEngine.h"
#include "routines.h"
#include "wcxhead.h"
#include "config.h"

U32B HAEngine::getvalue(int len) 
{
    unsigned char buf[4];
    U32B val;
    int i;
    
    if (read(arcfile,buf,len)!=len) error(1,ERR_READ,arcname);
    for (val=i=0;i<len;++i) val|=(U32B)buf[i]<<(i<<3);
    return val;
}

void HAEngine::putvalue(U32B val, int len) 
{
    unsigned char buf[4];
    int i;
    
    for (i=0;i<len;++i,val>>=8) buf[i]=(unsigned char) val&0xff;
    if (write(arcfile,&buf,len)!=len) error(1,ERR_WRITE,arcname);
}

char *HAEngine::getstring(char *str) 
{
    int offset;
    
    for (offset=0;;offset++) {
	if (read(arcfile,str+offset,1)!=1) error(1,ERR_READ,arcname);
	if (str[offset]==0) break;
    }
    return str;
}

void HAEngine::putstring(char *string) 
{
    int len;

    len=strlen(string)+1;
    if (write(arcfile,string,len)!=len) error(1,ERR_WRITE,arcname);
}

Fheader *HAEngine::getheader(void) 
{
    static Fheader hd={0,0,0,0,0,0,NULL,NULL,0};

    if ((hd.ver=getvalue(1))!=0xff) {
	hd.type=hd.ver&0xf;
	hd.ver>>=4;
	if (hd.ver>MYVER) error(1,ERR_TOONEW);
	if (hd.ver<LOWVER) error(1,ERR_TOOOLD);
	if (hd.type!=M_SPECIAL && hd.type!=M_DIR && hd.type>=M_UNK) 
	  error(1,ERR_UNKMET,hd.type);
    }
    hd.clen=getvalue(4);
    hd.olen=getvalue(4);
    hd.crc=getvalue(4);
    hd.time=getvalue(4);
    getstring(hd.path);
    getstring(hd.name);
    hd.mdilen=(unsigned)getvalue(1);
    hd.mylen=hd.mdilen+20+strlen(hd.path)+strlen(hd.name);
    char buf[255];
	read(arcfile,buf,hd.mdilen);
	switch(buf[0])
	{
	case MSDOSMDH:
		hd.attr=buf[1];
		break;
/*	case UNIXMDH:
		hd.attr=buf[1];//|(buf[2]<<8);
		break;*/
	default: // look like uncorrect but Harry do the same thing in
		hd.attr=_A_ARCH;  // his MSDOS HA sources ... 8-)
		break;
	}
    return &hd;
}

void HAEngine::putheader(Fheader *hd) 
{
    putvalue((hd->ver<<4)|hd->type,1);
    putvalue(hd->clen,4); 
    putvalue(hd->olen,4); 
    putvalue(hd->crc,4); 
    putvalue(hd->time,4); 
    putstring(hd->path);
    putstring(hd->name);
    putvalue(hd->mdilen,1);

	unsigned char buf[2];
    buf[0]=MSDOSMDH;
    buf[1]=hd->attr;
    write(arcfile,buf,2);
}	

static int ExceptFilter(LPEXCEPTION_POINTERS lpe)
{
	::MessageBox(GetFocus(),(const char *)(lpe->ExceptionRecord->ExceptionInformation[0]),"HA Archiver plugin Warning",MB_ICONWARNING);
	return EXCEPTION_EXECUTE_HANDLER;
}
void HAEngine::arc_clean(void) 
{
    U32B ipos,opos,cpylen;
    int len;
    unsigned cnt;
    Fheader *hd;
   
	__try
	{
		ipos=opos=4;
		for (cnt=arccnt;cnt;--cnt) {
		if (lseek(arcfile,ipos,SEEK_SET)<0) error(1,ERR_SEEK,"arc_clean()");
		for (;;) {
			hd=getheader();
			if (hd->ver!=0xff) break;
			ipos+=hd->clen+hd->mylen;
			if (lseek(arcfile,ipos,SEEK_SET)<0) 
			  error(1,ERR_SEEK,"arc_clean()");
		} 
		if (ipos==opos) ipos=opos+=hd->clen+hd->mylen;
		else {
			cpylen=hd->clen+hd->mylen;
			while (cpylen) {
			if (lseek(arcfile,ipos,SEEK_SET)<0) 
			  error(1,ERR_SEEK,"arc_clean()");
			len=read(arcfile,ib,BLOCKLEN>cpylen?(unsigned)cpylen:BLOCKLEN);
			if (len<=0) error(1,ERR_READ,arcname);
			cpylen-=len;
			ipos+=len;
			if (lseek(arcfile,opos,SEEK_SET)<0) 
			  error(1,ERR_SEEK,"arc_clean()");
			if (write(arcfile,ib,len)!=len) error(1,ERR_WRITE,arcname);
			opos+=len;
			}
		}
		}
		chsize(arcfile,opos);	
	}
	__except(ExceptFilter(GetExceptionInformation()))
	{
	}
}

void HAEngine::arc_close(void) 
{
    if (arcfile>=0) 
	{
		if (dirty) arc_clean();
		close(arcfile);
		if (!arccnt) 
		{
			if (remove(arcname)) 
				error(0,ERR_REMOVE,arcname);
			else
				arcfile =-1;
		}
    }
}

U32B HAEngine::arc_scan(void) 
{
    U32B pos;
    unsigned i;
    Fheader *hd;
    
    pos=4;
    for (i=0;i<arccnt;++i) {
	if (pos>=arcsize) {
	    error(0,ERR_CORRUPTED);
	    arccnt=i;
	    return pos;
	}
	if (lseek(arcfile,pos,SEEK_SET)<0) error(1,ERR_SEEK,"arc_seek()");
	hd=getheader();
	pos+=hd->clen+hd->mylen;
	if (hd->ver==0xff) {
	    dirty=1;
	    --i;
	}
    }
    if (pos!=arcsize) dirty=1;
    return pos;
}

int HAEngine::arc_open(char *aname,int mode) 
{
    char id[2];
    dirty=0;
	arcfile=-1;
	__try
	{
		strcpy(arcname,aname);
		if ((arcfile=open(arcname,(mode&ARC_RDO)?AO_RDOFLAGS:AO_FLAGS))>=0) 
		{
		if (fstat(arcfile,&arcstat)!=0) error(1,ERR_STAT,arcname);
		arcsize=arcstat.st_size;
		if (read(arcfile,id,2)!=2 || id[0]!='H' || id[1]!='A') 
		{
			error(1,ERR_NOHA,arcname);
		}
		arccnt=(unsigned)getvalue(2);
		arcsize=arc_scan();
		}
		else if ((mode&ARC_NEW) && (arcfile=open(arcname,AC_FLAGS))>=0) 
		{
		if (fstat(arcfile,&arcstat)!=0) error(1,ERR_STAT,arcname);
		if (write(arcfile,"HA\000",4)!=4) error(1,ERR_WRITE,arcname);
		arccnt=0;
		arcsize=4;
		}
		else error(1,ERR_ARCOPEN,arcname);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		int iret=0;
		switch(GetExceptionCode())
		{
		case ERR_NOHA:
			iret=E_UNKNOWN_FORMAT;
			break;
		case ERR_WRITE:
			iret=E_EWRITE;
			break;
		case ERR_ARCOPEN:
		case ERR_STAT:
			iret=E_EOPEN;
			break;
		default:
			iret=E_NOT_SUPPORTED;
			break;
		}
		if(arcfile>=0)
		{
			close(arcfile);
			arcfile=-1;			
		}
		return iret;
	}
	return 0;
}

void HAEngine::arc_reset(void) 
{			
    nextheader=4;
}

Fheader *HAEngine::arc_seek(void) 
{
    Fheader *hd;
    
    for (;;) {
	if (nextheader>=arcsize) return NULL;
	if (lseek(arcfile,nextheader,SEEK_SET)<0) 
	  error(1,ERR_SEEK,"arc_seek()");
	hd=getheader();
	thisheader=nextheader;
	nextheader+=hd->clen+hd->mylen;
	if (hd->ver==0xff) dirty=1;
	else 
	{
		fhdr=*hd;
		return hd;
	}
    }
}

void HAEngine::arc_delete(void) 
{
 
    if (lseek(arcfile,thisheader,SEEK_SET)<0) 
      error(1,ERR_SEEK,"arc_delete()");	
    if (write(arcfile,"\xff",1)!=1) error(1,ERR_WRITE,arcname);
    if (lseek(arcfile,2,SEEK_SET)<0) error(1,ERR_SEEK,"arc_delete()");	
    putvalue(--arccnt,2);
    dirty=1;
}

void HAEngine::arc_newfile(char *mdpath, char *name,struct _finddata_t *ft) 
{
	newhdr.ver=MYVER;
    newhdr.olen=ft->size;
	
	newhdr.time=ft->time_write;
	if(mdpath)
		strcpy(newhdr.path,(char *)md2hapath((unsigned char *)mdpath));
	else
		newhdr.path[0]='\0';
    strcpy(newhdr.name,(char *)md2hapath((unsigned char *)name));
    newhdr.mdilen=2;
    newhdr.mylen=newhdr.mdilen+20+strlen(newhdr.path)+strlen(newhdr.name);
    bestpos=trypos=arcsize+newhdr.mylen;
    addtries=0;
    dirty|=2;
}

void HAEngine::arc_accept(int method) 
{
    bestpos=trypos;
    newhdr.type=method;
    trypos+=newhdr.clen=ocnt;
    newhdr.crc=getcrc();
}

void HAEngine::arc_trynext(void) 
{
    if (lseek(arcfile,trypos,SEEK_SET)<0) error(1,ERR_SEEK,"arc_trynext()");
    if (addtries++) dirty=1;
}

void HAEngine::delold(void) 
{

    U32B pos,oldpos;
    unsigned i;
    Fheader *hd;
    
    pos=4;
    for (i=arccnt;i>0;--i) {
	if (pos>=arcsize) break;
	if (lseek(arcfile,pos,SEEK_SET)<0) error(1,ERR_SEEK,"delold()");
	hd=getheader();
	oldpos=pos;
	pos+=hd->clen+hd->mylen;
	if (hd->ver==0xff) {
	    dirty=1;
	    ++i;
	}
	else {
        if (!strcmp(md_strcase(hd->path), newhdr.path) &&
            !stricmp(md_strcase(hd->name),newhdr.name)) 
		{
		if (lseek(arcfile,oldpos,SEEK_SET)<0) 
		  error(1,ERR_SEEK,"delold()");
		if (write(arcfile,"\xff",1)!=1) error(1,ERR_WRITE,arcname);
		dirty=1;
		--arccnt;
	    }
	}
    }
}

int HAEngine::arc_addfile(void) 
{
    U32B basepos,len;
    int cplen;
    
    if ((basepos=arcsize+newhdr.mylen)!=bestpos) {
	if (lseek(arcfile,basepos,SEEK_SET)<0) 
	  error(1,ERR_SEEK,"arc_addfile()");
	len=newhdr.clen;
	while (len) {
	    if (lseek(arcfile,bestpos,SEEK_SET)<0) 
	      error(1,ERR_SEEK,"arc_addfile()");
	    cplen=BLOCKLEN>len?(int)len:BLOCKLEN;
	    if (read(arcfile,ib,cplen)!=cplen) error(1,ERR_READ,arcname);
	    len-=cplen;
	    bestpos+=cplen;
	    if (lseek(arcfile,basepos,SEEK_SET)<0) 
	      error(1,ERR_SEEK,"arc_addfile()");
	    if (write(arcfile,ib,cplen)!=cplen) error(1,ERR_WRITE,arcname);
	    basepos+=cplen;
	}
    }
    if (lseek(arcfile,arcsize,SEEK_SET)<0) error(1,ERR_SEEK,"arc_addfile()");
    putheader(&newhdr);
    dirty&=1;
    delold();
    ++arccnt;
    arcsize+=newhdr.mylen+newhdr.clen;
    if (lseek(arcfile,2,SEEK_SET)<0) error(1,ERR_SEEK,"arc_addfile()");
    putvalue(arccnt,2);
    return 1;
}

int HAEngine::addfile(char *path, char *name,int move,int usepath)
{
	char strp[_MAX_PATH];
	char *fullname;
	int inf=-1;
	long lFindHandle=-1;
	//struct stat filestat;
	struct _finddata_t ft;

	__try
	{
		fullname=md_pconcat(0,path,name,strp);
		lFindHandle=_findfirst(fullname,&ft);
		if(lFindHandle == -1)
			error(0,ERR_NOFILES,fullname);

		totalsize=ft.size;
		
		newhdr.attr=ft.attrib;

		arc_newfile((usepath)?strippath(name):NULL,stripname(name),&ft);
		
		if ((inf=open(fullname,O_RDONLY|O_BINARY))<0) 
		{
			error(0,ERR_OPEN,fullname);
			//free(fullname);
			//_findclose(lFindHandle);
			return 0;
		}
		setoutput(arcfile,0,arcname);
		setinput(inf,CRCCALC|PROGDISP,fullname);
		if (totalsize) 
		{
			arc_trynext();
			pack();
			arc_accept(gPackMetod);
		}
		else 
			arc_accept(M_CPY);

		arc_addfile();
		close(inf);

		if (move) 
		{
			if (remove(fullname)<0) 
				error(0,ERR_REMOVE,fullname);
		}
	}
	__finally
	{
	//free(fullname);
		if(lFindHandle!=-1)
			_findclose(lFindHandle);
		if(inf != -1)
			close(inf);
	}
	return 1;
}

// = end of file archive.cpp =