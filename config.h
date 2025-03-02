/***********************************************************************
  This file is part of HAWCX, a archiver plugin for Windows Commander.
  Copyright (C) 1999 Sergey Zharsky  e-mail: zharik@usa.net
  Copyright (C) 2025 Oleg Farenyuk   e-mail: indrekis@gmail.com
***********************************************************************
  Config.h
***********************************************************************/

#ifndef _CONFIG_H_
	#define _CONFIG_H_

extern HINSTANCE hInstance;

struct ArchiveRec
{
	HAEngine *m_pEngine;
	int m_iCurOpenMode;
	tChangeVolProc ChangeVolProc;
	tProcessDataProc ProcessDataProc;
};

extern int gPackMetod;

extern ArchiveRec ArchiveList[];

#define MAX_ARCHIVES 10

#endif //_CONFIG_H_

// = end of file Config.h =