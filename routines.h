/***********************************************************************
  This file is part of HAWCX, a archiver plugin for Windows Commander.
  Copyright (C) 1999 Sergey Zharsky  e-mail:zharik@usa.net
***********************************************************************
  Routines.h - some miscelaneous routines ...
***********************************************************************/

#ifndef _ROUTINES_H_
	#define _ROUTINES_H_

unsigned char *fullpath(unsigned char *path, unsigned char *name, unsigned char *fpath);
char * eat_drive(char *filepath);
unsigned char *ha2mdpath(unsigned char *hapath);
int makepath(unsigned char *hapath);
unsigned char *md2hapath(unsigned char *mdpath);
char *strippath(char *mdfullpath);
char *stripname(char *mdfullpath);
BOOL match_for_delete(char * patterns[],int patcnt,Fheader *hd);
char *md_strcase(char *s);
char *md_pconcat(int delim, char *head, char *tail, char *path);

#endif //_ROUTINES_H_

// = End of file routines.h =