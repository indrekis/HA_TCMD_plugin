/***********************************************************************
  This file is part of HAWCX, a archiver plugin for Windows Commander.
  Copyright (C) 1999 Sergey Zharsky  e-mail:zharik@usa.net
***********************************************************************
  Routines.cpp
***********************************************************************/

#include "StdAfx.h"
#include "HAEngine.h"

unsigned char *fullpath(unsigned char *path, unsigned char *name, unsigned char *fpath) 
{
    if (path==NULL || *path==0) 
		strcpy((char *)fpath,(char *)name);
	else
	{
		int need_delim;
    
		if ((unsigned char)path[strlen((char *)path)-1]!=0xff) 
			need_delim=1;
		else 
			need_delim=0;

		strcpy((char *)fpath,(char *)path);
		strcpy((char *)(fpath+strlen((char *)fpath)+need_delim),(char *)name);
		if (need_delim) 
			fpath[strlen((char *)fpath)]=(unsigned char)0xff;
	}
    return fpath;
}

char * eat_drive(char *filepath)
{
	char drive[_MAX_DRIVE],dir[_MAX_DIR],file[_MAX_FNAME],ext[_MAX_EXT];
    
	_splitpath(filepath,drive,dir,file,ext);
    if (strlen(drive))
		_makepath(filepath,NULL,(*dir == '\\')?dir+1:dir,file,ext);
    return filepath;
}

unsigned char *ha2mdpath(unsigned char *hapath) 
{
    static char p[_MAX_PATH];
    int i,j;
	
    for (i=j=0;hapath[i];++i) 
	{
		switch(hapath[i])
		{ 
		  case 0xff :
			p[j++]='\\';
			break;
		  default :
			p[j++]=hapath[i];
			break;
		}
	}
    p[j]=0;

    return (unsigned char *)p;
}

int makepath(unsigned char *hapath) 
{
    unsigned char *last,*path;
    
    for (last=(unsigned char *)strchr((const char *)hapath,0xff);last!=NULL;last=(unsigned char *)strchr((char *)(last+1),0xff)) 
	{
		*last=0;
		if (access((char *)(path=ha2mdpath(hapath)),06)) 
		{
			/*if (*/mkdir((char *)path);//<0) 
				//return ERR_MKDIR;
		}
		*last=0xff;
    }	
	return 0;
}

char *md_strcase(char *s) 
{
    int i;
    
    for (i=0;s[i];++i) 
		s[i]=tolower(s[i]);

    return s;
}

unsigned char *md2hapath(unsigned char *mdpath) 
{
    static unsigned char p[_MAX_PATH];
    int i,j;
    
    for (i=0;mdpath[i];++i) 
		if (mdpath[i]!='.' && mdpath[i]!='\\') 
			break;

    while (i>0 && mdpath[i-1]=='.') 
		--i;

    for (j=0;mdpath[i];++i) 
	{
		switch(mdpath[i]) 
		{
		case 0xff :
			p[j++]=(unsigned char)0xdf;
			break;
		case '\\' :
			p[j++]=(unsigned char)0xff;
			break;
		default :
			p[j++]=mdpath[i];
		    break;
		}
    }
    p[j]=0;
    return (unsigned char *)md_strcase((char *)p);
}

char *strippath(char *mdfullpath) 
{
    char drive[_MAX_DRIVE],dir[_MAX_DIR],file[_MAX_FNAME],ext[_MAX_EXT];
    static char path[_MAX_DRIVE+_MAX_DIR]; 	

	_splitpath(mdfullpath,drive,dir,file,ext);
    _makepath(path,drive,dir,"","");
    return path;
}

char *stripname(char *mdfullpath) 
{
    char drive[_MAX_DRIVE],dir[_MAX_DIR],file[_MAX_FNAME],ext[_MAX_EXT];
    static char name[_MAX_FNAME+_MAX_EXT]; 	

	_splitpath(mdfullpath,drive,dir,file,ext);
    _makepath(name,"","",file,ext);
    return name;
}

BOOL match_for_delete(char * patterns[],int  patcnt,Fheader *hd)
{
	int bRet=FALSE;
	for(int i=0;i<patcnt && !bRet;i++)
	{
		char *fullname=patterns[i];
		char *path=strippath(patterns[i]);
		char *name=stripname(patterns[i]);
		unsigned char fpath[_MAX_PATH];
		char *fullhapath=(char *)fullpath((unsigned char *)hd->path,(unsigned char *)hd->name,fpath);

		if(!strcmp(name,"*.*"))
		{
			if(!strncmp(path,(char *)ha2mdpath((unsigned char *)fullhapath),strlen(path)))
				bRet=TRUE;	
		}
		else
		{
			if(!stricmp(fullname,(char *)ha2mdpath((unsigned char *)fullhapath)))
				bRet=TRUE;
		}
	}
	return bRet;
}

char *md_pconcat(int delim, char *head, char *tail, char *newpath) 
{
    int headlen,delim1;

    delim1=0;
    if ((headlen=strlen(head))!=0)  
	{
		if (head[headlen-1]!='\\' && head[headlen-1]!=':') 
			delim1=1;
    }

    if (headlen!=0) 
		strcpy(newpath,head);

    if (delim1) 
		newpath[headlen]='\\';

    strcpy(newpath+headlen+delim1,tail);

    if (delim) 
		strcpy(newpath+strlen(newpath),"\\");

    return newpath;
}

// = end of file routines.cpp =