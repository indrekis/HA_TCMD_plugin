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
	HA error handling
***********************************************************************/

#include "StdAfx.h"

#include "HAEngine.h"

int inerror=0,lasterror=0;

const char *error_string[] = {
    "Error in error handling !!!",			
    "Unknown error",
    "Command %c not implemented",	
    "Could not open archive %s",
    "Out of memory in %s",
    "%s is not a HA archive",
    "Write error on %s",
    "Read error on %s",
    "Got signal %d",
    "No files found",
    "Could not remove %s",
    "Invalid switch %c",
    "Archive needs newer version of HA",
    "Archive made with unsupported version",
    "Unknown compression method %d",
    "Lseek error in %s",
    "Could not open file %s",
    "Could not make directory %s",
    "CRC error",
    "Write error",
    "Could not get file status for %s",
    "Could not open directory %s",
    "Archive corrupted !",
    "Wrong size for %s",
    "Don't know how to handle %s",
    "Could not read symlink %s",
    "Could not link %s to %s",
    "Could not make fifo %s",
	"Processing aborted",
};

char errorStr[512];

void HAEngine::error(int fatal, int number, ...)
{
    va_list argptr;
    
    if (inerror) {
#ifndef _WIN64
        RaiseException(0, 0, 1, (CONST DWORD*)error_string[0]);
#else
		RaiseException(0, 0, 1, (CONST ULONG_PTR*)error_string[0]);
#endif
    }
    inerror=number;

    va_start(argptr,number);
    vsprintf(errorStr,error_string[number],argptr);

    if (!fatal) 
	{
		lasterror=inerror;
		inerror=0;
		MessageBox(GetFocus(),errorStr,"HA Archiver plugin Error",MB_ICONWARNING);
		return;
    }
#ifndef _WIN64
	RaiseException(number,0,1,(CONST DWORD *)errorStr);
#else
	RaiseException(number, 0, 1, (CONST ULONG_PTR*)errorStr);
#endif

}

// = end of file error.cpp =
