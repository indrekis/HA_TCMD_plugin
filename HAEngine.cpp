/***********************************************************************
  This file is part of HAWCX, a archiver plugin for Windows Commander.
  Copyright (C) 1999 Sergey Zharsky  e-mail:zharik@usa.net
***********************************************************************
  HAEngine.cpp 
***********************************************************************/

#include "StdAfx.h"

#include "HAEngine.h"
#include "wcxhead.h"
#include "config.h"
#include "routines.h"

HAEngine::HAEngine()
{
	crctabok=0;
	outname=NULL,inname=NULL;
	ll=NULL,cr=NULL,best=NULL;
	b=NULL;ccnt=NULL;
	arccnt=0;
	ZeroMemory(&fhdr,sizeof(Fheader));
	nextheader=4;
	m_pWrapper=NULL;
}

void HAEngine::pack(void)
{
	switch(gPackMetod)
	{
	case M_CPY:
		cpy();
		break;
	case M_ASC:
		asc_pack();
		break;
	case M_HSC:
		hsc_pack();
		break;
	}
}

void HAEngine::unpack(void)
{
	switch(fhdr.type/*gPackMetod*/)
	{
	case M_CPY:
		cpy();
		break;
	case M_ASC:
		asc_unpack();
		break;
	case M_HSC:
		hsc_unpack();
		break;
	}
}

void HAEngine::cleanup(bool bUnpack)
{
	switch((bUnpack)?fhdr.type:gPackMetod)
	{
	case M_CPY:
		break;
	case M_ASC:
		asc_cleanup();
		break;
	case M_HSC:
		hsc_cleanup();
		break;
	}
}

void HAEngine::ExtractFile(char *ofname)
{
	int of;

	if(makepath(md2hapath((unsigned char *)strippath(ofname))))
		error(0,ERR_MKDIR,ofname);

	of=open(ofname,_O_WRONLY|_O_BINARY|_O_CREAT|_O_EXCL,_S_IREAD | _S_IWRITE);
	setinput(arcfile,0,arcname);
	setoutput(of,CRCCALC|PROGDISP,ofname);
	if (fhdr.olen!=0) 
	{
		totalsize=fhdr.olen;
		unpack();
		cleanup(true);
	}
	close(of);
	if (fhdr.crc!=getcrc()) error(0,ERR_CRC,NULL);

	struct _utimbuf buf;
	buf.actime=buf.modtime=fhdr.time;
	int i=_utime(ofname,&buf);

	DWORD dwAttr=FILE_ATTRIBUTE_NORMAL;
	if(fhdr.attr&_A_RDONLY)dwAttr|=FILE_ATTRIBUTE_READONLY;
	if(fhdr.attr&_A_ARCH)dwAttr|=FILE_ATTRIBUTE_ARCHIVE;
	if(fhdr.attr&_A_HIDDEN)dwAttr|=FILE_ATTRIBUTE_HIDDEN;
	if(fhdr.attr&_A_SYSTEM)dwAttr|=FILE_ATTRIBUTE_SYSTEM;
	SetFileAttributes(ofname,dwAttr);
}

void HAEngine::TestArchive(char *ofname)
{
	setinput(arcfile,0,arcname);
	setoutput(-1,CRCCALC|PROGDISP,"none ??");
	if (fhdr.olen!=0) {
	totalsize=fhdr.olen;
	unpack();
	cleanup(true);
	}
	if (fhdr.crc!=getcrc()) error(0,ERR_CRC,NULL);
}

// = end of file haengine.cpp =