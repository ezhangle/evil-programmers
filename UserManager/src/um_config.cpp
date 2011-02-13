/*
    um_config.cpp
    Copyright (C) 2002-2007 zg

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <stdio.h>
#include "far_helper.h"
#include "umplugin.h"
#include "guid.h"

static INT_PTR WINAPI ConfigDialogProc(HANDLE hDlg, int Msg,int Param1,INT_PTR Param2)
{
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

int Config(void)
{
  //Show dialog
  /*
    0000000000111111111122222222223333333333444444444455555555556666666666777777
    0123456789012345678901234567890123456789012345678901234567890123456789012345
  00                                                                            00
  01   浜様様様様様様様様様様様様� Event viewer 様様様様様様様様様様様様様様�   01
  02   � [x] Add to Disks menu                                              �   02
  03   �   2 Disks menu hotkey ('1'-'9'). Leave empty to autoassign         �   03
  04   � [ ] Add to Plugins menu                                            �   04
  05   � [ ] Add to Config menu                                             �   05
  06   把陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳超   06
  07   � [ ] Full user names                                                �   07
  08   把陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳超   08
  09   � Command line prefix to start the properties:                       �   09
  10   � acl                                                                �   10
  11   把陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳超   11
  12   �                         [ Ok ]  [ Cancel ]                         �   12
  13   藩様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様夕   13
  14                                                                            14
    0000000000111111111122222222223333333333444444444455555555556666666666777777
    0123456789012345678901234567890123456789012345678901234567890123456789012345
  */

  FarDialogItem DialogItems[]=
  {
  /* 0*/  {DI_DOUBLEBOX,3, 1,72,13,{0},NULL,NULL,                 0,                                0,GetMsg(mName),                 0},
  /* 1*/  {DI_CHECKBOX, 5, 2, 0, 0,{0},NULL,NULL,                 0,                                0,GetMsg(mConfigAddToDisksMenu), 0},
  /* 2*/  {DI_FIXEDIT,  7, 3, 7, 3,{0},NULL,NULL,                 DIF_FOCUS,                        0,_T(""),                        0},
  /* 3*/  {DI_TEXT,     9, 3, 0, 0,{0},NULL,NULL,                 0,                                0,GetMsg(mConfigDisksMenuDigit), 0},
  /* 4*/  {DI_CHECKBOX, 5, 4, 0, 0,{0},NULL,NULL,                 0,                                0,GetMsg(mConfigAddToPluginMenu),0},
  /* 5*/  {DI_CHECKBOX, 5, 5, 0, 0,{0},NULL,NULL,                 0,                                0,GetMsg(mConfigAddToConfigMenu),0},
  /* 6*/  {DI_TEXT,    -1, 6, 0, 0,{0},NULL,NULL,                 DIF_SEPARATOR,                    0,_T(""),                        0},
  /* 7*/  {DI_CHECKBOX, 5, 7, 0, 0,{0},NULL,NULL,                 0,                                0,GetMsg(mConfigFullUserName),   0},
  /* 8*/  {DI_TEXT,    -1, 8, 0, 0,{0},NULL,NULL,                 DIF_SEPARATOR,                    0,_T(""),                        0},
  /* 9*/  {DI_TEXT,     5, 9, 0, 0,{0},NULL,NULL,                 0,                                0,GetMsg(mConfigPrefix),         0},
  /*10*/  {DI_FIXEDIT,  5,10,19,12,{0},NULL,_T("AAAAAAAAAAAAAAA"),DIF_MASKEDIT,                     0,_T(""),                        0},
  /*11*/  {DI_TEXT,    -1,11, 0, 0,{0},NULL,NULL,                 DIF_SEPARATOR,                    0,_T(""),                        0},
  /*12*/  {DI_BUTTON,   0,12, 0, 0,{0},NULL,NULL,                 DIF_CENTERGROUP|DIF_DEFAULTBUTTON,0,GetMsg(mConfigSave),           0},
  /*13*/  {DI_BUTTON,   0,12, 0, 0,{0},NULL,NULL,                 DIF_CENTERGROUP,                  0,GetMsg(mConfigCancel),         0}
  };

  TCHAR digit[21];

  DialogItems[1].Selected=Opt.AddToDisksMenu;
  if (Opt.DisksMenuDigit)
  {
    FSF.sprintf(digit,_T("%d"),Opt.DisksMenuDigit);
    DialogItems[2].PtrData=digit;
  }
  DialogItems[4].Selected=Opt.AddToPluginsMenu;
  DialogItems[5].Selected=Opt.AddToConfigMenu;
  DialogItems[7].Selected=Opt.FullUserNames;
  INIT_DLG_DATA(DialogItems[10],Opt.Prefix);

  CFarDialog dialog;
  int DlgCode=dialog.Execute(MainGuid,ConfigGuid,-1,-1,76,15,_T("Config"),DialogItems,(sizeof(DialogItems)/sizeof(DialogItems[0])),0,0,ConfigDialogProc,0);
  if (DlgCode!=12)
    return FALSE;
  Opt.AddToDisksMenu=dialog.Check(1);
  Opt.DisksMenuDigit=FSF.atoi(dialog.Str(2));
  Opt.AddToPluginsMenu=dialog.Check(4);
  Opt.AddToConfigMenu=dialog.Check(5);
  Opt.FullUserNames=dialog.Check(7);
  _tcscpy(Opt.Prefix,dialog.Str(10));
  FSF.Trim(Opt.Prefix);
  HKEY hKey;
  DWORD Disposition;
  if((RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition)) == ERROR_SUCCESS)
  {
    RegSetValueEx(hKey,_T("AddToDisksMenu"),0,REG_DWORD,(LPBYTE)&Opt.AddToDisksMenu,sizeof(Opt.AddToDisksMenu));
    RegSetValueEx(hKey,_T("DisksMenuDigit"),0,REG_DWORD,(LPBYTE)&Opt.DisksMenuDigit,sizeof(Opt.DisksMenuDigit));
    RegSetValueEx(hKey,_T("AddToPluginsMenu"),0,REG_DWORD,(LPBYTE)&Opt.AddToPluginsMenu,sizeof(Opt.AddToPluginsMenu));
    RegSetValueEx(hKey,_T("AddToConfigMenu"),0,REG_DWORD,(LPBYTE)&Opt.AddToConfigMenu,sizeof(Opt.AddToConfigMenu));
    RegSetValueEx(hKey,_T("FullUserNames"),0,REG_DWORD,(LPBYTE)&Opt.FullUserNames,sizeof(Opt.FullUserNames));
    RegSetValueEx(hKey,_T("Prefix"),0,REG_SZ,(LPBYTE)Opt.Prefix,(_tcslen(Opt.Prefix)+1)*sizeof(TCHAR));
    RegCloseKey(hKey);
  }
  return TRUE;
}
