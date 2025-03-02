                 HA archives plugin for Windows Commander

  What it does:

This is full-functional version !!! It support viewing, unpacking, 
packing and deleting files within archives in HA format. Uses original 
HA sources by Harry Hirvola.

  IMPORTANT!!! For users of previous versions (1,0,0,3 and later) !!!

If you have installed Windows Commander 5.0 and want to use last its features
- searching inside of archives and opening by ID - you 
should uninstall old version before installing this version!!! If you don't want to 
use tat features or have older versions of Windows Commander installed - go to Install
section.

   UNINSTALL

How to do UNINSTALL previous version:
1. In Windows Commander 4.0 (or newer), choose Configuration - Options
2. Open the 'Packer' page
3. Click 'Configure packer extension DLLs'
4. Select "ha" in "All files with extension (ending with): as the 
   extension" combobox.
5. Clear "Associate with:" editbox.
7. Click OK

   INSTALL

How to install this plugin (32 bit only):
1. Unzip the WCX to the Wincmd directory (usually c:\wincmd)
2. In Windows Commander 4.0 (or newer), choose Configuration - Options
3. Open the 'Packer' page
4. Click 'Configure packer extension DLLs'
5. type  ha  as the extension
6. Click 'new type', and select the ha.wcx
7. Click OK

  USEFUL INFORMATION!!! If you very accurate and observant user you can see some strange things with compressed files times. It's all OK when you use HAWCX for packing viewing and unpacking files. But if you use DOS-version of HA archiver for pack archive and after that try to browse into it with WC, you can see that time of original files and compressed ones is different.It is happens because DOS version HA looking for TZ environment variable and when it can't find one, it think that we are in USA. 8-) If you concerned this problem just set TZ in you autoexec.bat in correct value. For Belarus it may look like this:

SET TZ=SBT-2GDT

where SBT is arbitrary prefix that describe name of your timezone (Standard Belarusian Time) +2 is offset from UTC and GDT is daylight savings indicator. But if use only HAWCX you not need to do it! You not need to change TZ in case of archives already packed HA.EXE. Relax.

  "MANUAL" CALLING OF CONFIGURATION DIALOG

From version 1.1.0 you can open plugin configuration dialog by running following command from Windows command-line:

"rundll32 ha.wcx,Settings"

A small hint: You can set-up "Start" menu or button bar of WindowsCommander to call this command. NOTE: Current directory should be set into that one where ha.wcx live. Yet another small hint - there is a icon for toolbar inside ha.wcx - try it!

======================================================

   (c) 1999-2002 by Siarzhuk Zharski

   http://zharik.cjb.net
   mailto: imker@gmx.li
