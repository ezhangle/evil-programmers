/*
    ab_selcolor.cpp
    Copyright (C) 2000-2008 zg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <tchar.h>
#include "plugin.hpp"
#include "abplugin.h"
#include "ab_main.h"
#include "guid.h"
#include <initguid.h>
// {24BDD4B4-E0C3-4118-B840-6CEB534E943C}
DEFINE_GUID(ColorGuid, 0x24bdd4b4, 0xe0c3, 0x4118, 0xb8, 0x40, 0x6c, 0xeb, 0x53, 0x4e, 0x94, 0x3c);

static FarColor Colors[]=
{
  {FCF_FG_4BIT|FCF_BG_4BIT,0x7,0x0,NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,0x0,0x4,NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,0x7,0x8,NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,0x0,0xc,NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,0x7,0x1,NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,0x0,0x5,NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,0x0,0x9,NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,0x0,0xd,NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,0x0,0x2,NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,0x0,0x6,NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,0x0,0xa,NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,0x0,0xe,NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,0x0,0x3,NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,0x0,0x7,NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,0x0,0xb,NULL},
  {FCF_FG_4BIT|FCF_BG_4BIT,0x0,0xf,NULL}
};

#define GetCheck(i) (int)Info.SendDlgMessage(hDlg,DM_GETCHECK,i,0)
static inline DWORD GetFlags(int i,HANDLE hDlg)
{
  DWORD flags=0;
  FarDialogItem DialogItem;
  if(Info.SendDlgMessage(hDlg,DM_GETDLGITEMSHORT,i,&DialogItem))
  {
    flags=DialogItem.Flags;
  }
  return flags;
}

INT_PTR WINAPI ColorDialogProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
  FarDialogItem DialogItem;
  switch(Msg)
  {
    case DN_INITDIALOG:
    case DN_BTNCLICK:
      break;
    case DN_CTLCOLORDLGITEM:
      {
        size_t shift=0;
        if(Param1>0&&Param1<17) shift=1;
        if(Param1>17&&Param1<34) shift=18;
        if(Param1>35&&Param1<39)
        {
          FarColor color={FCF_FG_4BIT|FCF_BG_4BIT,0,0,NULL};
          for(int i=1;i<17;i++)
          {
            if(GetCheck(i))
            {
              color.ForegroundColor=Colors[i-1].BackgroundColor;
              break;
            }
          }
          for(int i=18;i<34;i++)
          {
            if(GetCheck(i))
            {
              color.BackgroundColor=Colors[i-18].BackgroundColor;
              break;
            }
          }
          static_cast<FarDialogItemColors*>(Param2)->Colors[0]=color;
        }
        if(shift)
        {
          static_cast<FarDialogItemColors*>(Param2)->Colors[0]=Colors[Param1-shift];
        }
      }
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}

bool SelectColor(FarColor& color)
{
  bool result=false;
  //Show dialog
  /*
    000000000011111111112222222222333333333
    012345678901234567890123456789012345678
  00                                       00
  01   浜様様様様様� Color 様様様様様様�   01
  02   � � Foreground � � Background � �   02
  03   � �            � �          * � �   03
  04   � �            � �            � �   04
  05   � �            � �            � �   05
  06   � �          * � �            � �   06
  07   � 青陳陳陳陳陳潰 青陳陳陳陳陳潰 �   07
  08   � Text Text Text Text Text Text �   08
  09   � Text Text Text Text Text Text �   09
  10   � Text Text Text Text Text Text �   10
  11   把陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳�   11
  12   �      [ Set ]  [ Cancel ]      �   12
  13   藩様様様様様様様様様様様様様様様�   13
  14                                       14
    000000000011111111112222222222333333333
    012345678901234567890123456789012345678
  */
  static struct InitDialogItem InitItems[]={
  /* 0*/  {DI_SINGLEBOX,   5,2,18,7,0,0,(const TCHAR*)mColorForeground},
  /* 1*/  {DI_RADIOBUTTON, 6,3,0,0,0,DIF_GROUP|DIF_MOVESELECT,_T("")},
  /* 2*/  {DI_RADIOBUTTON, 6,4,0,0,0,          DIF_MOVESELECT,_T("")},
  /* 3*/  {DI_RADIOBUTTON, 6,5,0,0,0,          DIF_MOVESELECT,_T("")},
  /* 4*/  {DI_RADIOBUTTON, 6,6,0,0,0,          DIF_MOVESELECT,_T("")},
  /* 5*/  {DI_RADIOBUTTON, 9,3,0,0,0,          DIF_MOVESELECT,_T("")},
  /* 6*/  {DI_RADIOBUTTON, 9,4,0,0,0,          DIF_MOVESELECT,_T("")},
  /* 7*/  {DI_RADIOBUTTON, 9,5,0,0,0,          DIF_MOVESELECT,_T("")},
  /* 8*/  {DI_RADIOBUTTON, 9,6,0,0,0,          DIF_MOVESELECT,_T("")},
  /* 9*/  {DI_RADIOBUTTON,12,3,0,0,0,          DIF_MOVESELECT,_T("")},
  /*10*/  {DI_RADIOBUTTON,12,4,0,0,0,          DIF_MOVESELECT,_T("")},
  /*11*/  {DI_RADIOBUTTON,12,5,0,0,0,          DIF_MOVESELECT,_T("")},
  /*12*/  {DI_RADIOBUTTON,12,6,0,0,0,          DIF_MOVESELECT,_T("")},
  /*13*/  {DI_RADIOBUTTON,15,3,0,0,0,          DIF_MOVESELECT,_T("")},
  /*14*/  {DI_RADIOBUTTON,15,4,0,0,0,          DIF_MOVESELECT,_T("")},
  /*15*/  {DI_RADIOBUTTON,15,5,0,0,0,          DIF_MOVESELECT,_T("")},
  /*16*/  {DI_RADIOBUTTON,15,6,0,0,0,          DIF_MOVESELECT,_T("")},
  /*17*/  {DI_SINGLEBOX,  20,2,33,7,0,0,(const TCHAR*)mColorBackground},
  /*18*/  {DI_RADIOBUTTON,21,3,0,0,0,DIF_GROUP|DIF_MOVESELECT,_T("")},
  /*19*/  {DI_RADIOBUTTON,21,4,0,0,0,          DIF_MOVESELECT,_T("")},
  /*20*/  {DI_RADIOBUTTON,21,5,0,0,0,          DIF_MOVESELECT,_T("")},
  /*21*/  {DI_RADIOBUTTON,21,6,0,0,0,          DIF_MOVESELECT,_T("")},
  /*22*/  {DI_RADIOBUTTON,24,3,0,0,0,          DIF_MOVESELECT,_T("")},
  /*23*/  {DI_RADIOBUTTON,24,4,0,0,0,          DIF_MOVESELECT,_T("")},
  /*24*/  {DI_RADIOBUTTON,24,5,0,0,0,          DIF_MOVESELECT,_T("")},
  /*25*/  {DI_RADIOBUTTON,24,6,0,0,0,          DIF_MOVESELECT,_T("")},
  /*26*/  {DI_RADIOBUTTON,27,3,0,0,0,          DIF_MOVESELECT,_T("")},
  /*27*/  {DI_RADIOBUTTON,27,4,0,0,0,          DIF_MOVESELECT,_T("")},
  /*28*/  {DI_RADIOBUTTON,27,5,0,0,0,          DIF_MOVESELECT,_T("")},
  /*29*/  {DI_RADIOBUTTON,27,6,0,0,0,          DIF_MOVESELECT,_T("")},
  /*30*/  {DI_RADIOBUTTON,30,3,0,0,0,          DIF_MOVESELECT,_T("")},
  /*31*/  {DI_RADIOBUTTON,30,4,0,0,0,          DIF_MOVESELECT,_T("")},
  /*32*/  {DI_RADIOBUTTON,30,5,0,0,0,          DIF_MOVESELECT,_T("")},
  /*33*/  {DI_RADIOBUTTON,30,6,0,0,0,          DIF_MOVESELECT,_T("")},
  /*34*/  {DI_BUTTON,      0,12,0,0,0,DIF_CENTERGROUP|DIF_DEFAULTBUTTON,(const TCHAR*)mColorSet},
  /*35*/  {DI_BUTTON,      0,12,0,0,0,DIF_CENTERGROUP,(const TCHAR*)mColorCancel},
  /*36*/  {DI_TEXT,        5,8,0,0,0,0,(const TCHAR*)mColorTest1},
  /*37*/  {DI_TEXT,        5,9,0,0,0,0,(const TCHAR*)mColorTest2},
  /*38*/  {DI_TEXT,        5,10,0,0,0,0,(const TCHAR*)mColorTest3},
  /*39*/  {DI_DOUBLEBOX,   3,1,35,13,0,0,(const TCHAR*)mColorMain},
  /*40*/  {DI_TEXT,       -1,11,0,0,0,DIF_SEPARATOR,_T("")},
  };
  const unsigned char color_translate[]={0,4,8,0xc,1,5,9,0xd,2,6,0xa,0xe,3,7,0xb,0xf};
  struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];
  InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
  DialogItems[color_translate[color.ForegroundColor]+1].Flags|=DIF_FOCUS;
  DialogItems[color_translate[color.ForegroundColor]+1].Selected=true;
  DialogItems[color_translate[color.BackgroundColor]+18].Selected=true;
  int DlgCode=-1;
  HANDLE hDlg=Info.DialogInit(&MainGuid,&ColorGuid,-1,-1,39,15,_T("Color"),DialogItems,(sizeof(DialogItems)/sizeof(DialogItems[0])),0,0,ColorDialogProc,0);
  if(hDlg!=INVALID_HANDLE_VALUE) DlgCode=Info.DialogRun(hDlg);
  if(DlgCode==34)
  {
    for(int i=1;i<17;i++)
      if(GetCheck(i))
      {
        color.ForegroundColor=Colors[i-1].BackgroundColor;
        break;
      }
    for(int i=18;i<34;i++)
      if(GetCheck(i))
      {
        color.BackgroundColor=Colors[i-18].BackgroundColor;
        break;
      }
    result=true;
  }
  if(hDlg!=INVALID_HANDLE_VALUE) Info.DialogFree(hDlg);
  return result;
}
