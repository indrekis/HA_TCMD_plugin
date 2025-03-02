/***********************************************************************
  This file is part of HAWCX, a archiver plugin for Windows Commander.
  Copyright (C) 1999 Sergey Zharsky  e-mail:zharik@usa.net
***********************************************************************
  HAWCX : Main plugin module. 
  This code based on Christian Ghisler (support@ghisler.com) sources
***********************************************************************/

#include "StdAfx.h"

#include "HAEngine.h"
#include "wcxhead.h"
#include "config.h"
#include "routines.h"

/* Keep a list of currently open archives (up to a maximum of MAX_ARCHIVES) */
//static ArchiveRec ArchiveList[11];   /* 0 for packing, 1-10 for unpacking! */
ArchiveRec ArchiveList[MAX_ARCHIVES+1];             /* Important: array must be initialized to all 0! */

int __stdcall OpenArchive(tOpenArchiveData* ArchiveData)
{
	int ArcIdx;

    bool foundslot=false;
    for (int i=1;i<=MAX_ARCHIVES;i++)
      if(ArchiveList[i].m_pEngine==NULL) 
	  {
        foundslot=true;
        ArcIdx=i;
        break;
      }

    if (!foundslot) 
	{
      ArchiveData->OpenResult=E_NO_MEMORY;
      return 0;
    }

	ArchiveList[ArcIdx].m_pEngine=HAEngine::NewHAEngine();
	ArchiveList[ArcIdx].m_iCurOpenMode=ArchiveData->OpenMode;
	int iret;
	if(iret=ArchiveList[ArcIdx].m_pEngine->arc_open(ArchiveData->ArcName,ARC_RDO))
	{
		ArchiveData->OpenResult=iret;
		return 0;
	}
	ArchiveList[ArcIdx].m_pEngine->arc_reset();
	ArchiveList[ArcIdx].m_pEngine->SetWrapper(&ArchiveList[i]);
    return ArcIdx;  /* returns pseudohandle */
}

int __stdcall CloseArchive(int hArcData)
{
    if (hArcData>0 && hArcData<=MAX_ARCHIVES)
	{
      __try
		{
			ArchiveList[hArcData].m_pEngine->arc_close();

			delete ArchiveList[hArcData].m_pEngine;

			ArchiveList[hArcData].m_pEngine=NULL;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			switch(GetExceptionCode())
			{
			case ERR_REMOVE:
				return E_EWRITE;
			}
			return E_NOT_SUPPORTED;
		}
		return 0;
    } 
	else  
      return E_ECLOSE;
}

DWORD UnixTimeToDos(time_t UnixTime)
{
  struct tm *t;
  DWORD DosTime;
  t=localtime(&UnixTime);
  DosTime=(t->tm_sec/2)|(t->tm_min<<5)|(t->tm_hour<<11)|
          (t->tm_mday<<16)|((t->tm_mon+1)<<21)|((t->tm_year-80)<<25);
  return(DosTime);
}

int __stdcall ReadHeader(int hArcData,tHeaderData* HeaderData)
{ 
    if (hArcData>0 && hArcData<=MAX_ARCHIVES && ArchiveList[hArcData].m_pEngine) 
	{
		__try
		{

		Fheader *hd=ArchiveList[hArcData].m_pEngine->arc_seek();
		if(!hd)
		{
			ArchiveList[hArcData].m_pEngine->arc_reset();
			return E_END_ARCHIVE; 
		}
		HeaderData->PackSize=hd->clen;
		HeaderData->UnpSize=hd->olen;
		HeaderData->HostOS=0;//hd->type; ????
		HeaderData->Method=hd->type;
		HeaderData->FileCRC=hd->crc;
		HeaderData->FileTime=UnixTimeToDos(hd->time);
		HeaderData->FileAttr=hd->attr;
		unsigned char fpath[_MAX_PATH];
		char *p=(char *)fullpath((unsigned char *)hd->path,(unsigned char *)hd->name,fpath);
		strcpy(HeaderData->FileName,eat_drive((char *)ha2mdpath(( unsigned char *)p)));
		strcpy(HeaderData->ArcName,ArchiveList[hArcData].m_pEngine->ArcName());
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			switch(GetExceptionCode())
			{
			case ERR_SEEK:
				return E_EREAD;
			}
			return E_NOT_SUPPORTED;
		}
        return 0;
    } 
	else 
        return E_EREAD;
}

int __stdcall ProcessFile(int hArcData,int Operation,char* DestPath,char* DestName)
{
    if (hArcData<1 || hArcData>MAX_ARCHIVES)
        return E_EREAD;

	if (ArchiveList[hArcData].m_iCurOpenMode == PK_OM_LIST)
		Operation=PK_SKIP;

    if (Operation==PK_SKIP)
        return 0;

	if (ArchiveList[hArcData].m_pEngine && 
        ((Operation==PK_EXTRACT && DestName !=NULL) || (Operation==PK_TEST))) 
	{
		__try
		{

		if (Operation==PK_EXTRACT)
			ArchiveList[hArcData].m_pEngine->ExtractFile(DestName);
		else
			ArchiveList[hArcData].m_pEngine->TestArchive(DestName);

		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			switch(GetExceptionCode())
			{
			case ERR_SEEK: 
				return E_EREAD; 
			case ERR_ABORTED: 
				return E_EABORTED;
			}
			return E_NOT_SUPPORTED;
		}
		return 0;
	} 
	else
		return E_EREAD;
}

int __stdcall PackFiles(char *PackedFile,char *SubPath,char *SrcPath,
      char *AddList,int Flags)
{
	int l=0;
	unsigned int i;
	int patcnt,patcntorg,idircount=0;
	char **patterns;
	char *p=AddList;
				
	for(patcnt=0;l=strlen(p);patcnt++,p+=l+1);

	if(!patcnt)
		return E_NO_FILES;

	if(!(patterns=(char **)malloc(patcnt*sizeof(char *))))
		return E_NO_MEMORY;

	patcntorg=patcnt;

	p=AddList;

	for(i=0;l=strlen(p);p+=l+1)
	{
		char *name=stripname(p);
		if(strlen(name))
			patterns[i++]=p;
		else
		{
			patcnt--;
			patterns[patcntorg-idircount-1]=p;
			idircount++;
		}
	}

	HAEngine *engine=HAEngine::NewHAEngine();

	engine->SetWrapper(&ArchiveList[0]);
	
	int iret=0;
	if(iret=engine->arc_open(PackedFile,ARC_OLD|ARC_NEW))
		return iret;

	__try
	{
		for(i=0;i<patcnt;i++)
			engine->addfile(SrcPath,patterns[i],Flags&PK_PACK_MOVE_FILES,Flags&PK_PACK_SAVE_PATHS);

		if(idircount && Flags&PK_PACK_MOVE_FILES)
			for(i=patcntorg-idircount;i<patcntorg;i++)
				rmdir(patterns[i]);
   	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		switch(GetExceptionCode())
		{
		case ERR_SEEK: 
			iret=E_EREAD; 
			break;
		case ERR_ABORTED: 
			iret=E_EABORTED;
			break;
		default:
			iret=E_NOT_SUPPORTED;
		}
	}

	engine->arc_close();
	free(patterns);
	delete engine;
	return iret;
}

int __stdcall DeleteFiles(char *PackedFile,char *DeleteList)
{
	Fheader *hd;
	int l,i,patcnt;
	char *p=DeleteList;
	char **patterns;
	
	for(patcnt=0;l=strlen(p);patcnt++,p+=l+1);

	if(!patcnt)
		return E_NO_FILES;

	if(!(patterns=(char **)malloc(patcnt*sizeof(char *))))
		return E_NO_MEMORY;

	p=DeleteList;

	for(i=0;l=strlen(p);p+=l+1)
		patterns[i++]=p;

	HAEngine *engine=HAEngine::NewHAEngine();
	
	int iret=0;
	if(iret=engine->arc_open(PackedFile,ARC_OLD))
		return iret;

	engine->arc_reset();

	__try
	{
		if ((hd=engine->arc_seek())==NULL) return E_NO_FILES;
		do {
			if(match_for_delete(patterns,patcnt,hd))
				engine->arc_delete();
		} while ((hd=engine->arc_seek())!=NULL);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		switch(GetExceptionCode())
		{
		case ERR_NOFILES: 
			iret=E_NO_FILES; 
			break;
		case ERR_SEEK: 
			iret= E_EREAD;
			break;
		case ERR_MEM: 
			iret=E_NO_MEMORY;
			break;
		case ERR_WRITE: 
			iret=E_EWRITE;
			break;
		default:
			iret=E_NOT_SUPPORTED;
		}
	}

	engine->arc_close();
	free(patterns);
	delete engine;
	
	return iret;
}

BOOL __stdcall CanYouHandleThisFile(char* filename)
{
  BOOL bRet = FALSE;
	int arcfile = open(filename, _O_RDONLY|_O_BINARY);
  if(arcfile != -1)
  {
    char id[2]; 
    if ( !(read(arcfile, id, 2) != 2 || id[0]!='H' || id[1]!='A')) 
      bRet= TRUE;

    close(arcfile);
  }

  return bRet;
}



// = end of file hawcx.cpp = 