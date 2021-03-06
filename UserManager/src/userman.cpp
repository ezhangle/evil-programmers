/*
    userman.cpp
    Copyright (C) 2001-2007 zg

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
#include <stdlib.h>
#include <limits.h>
#include "far_helper.h"
#include <lm.h>
#include <ntsecapi.h>
#include "umplugin.h"
#include "memory.h"
#include <initguid.h>
#include "guid.h"
#include "bootstrap/umversion.h"

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
BOOL IsOldFAR=TRUE;

struct Options Opt={false,true,true,true,_T("acl")};

WellKnownSID AddSID[]=
{
  {{{SECURITY_NULL_SID_AUTHORITY}},SECURITY_NULL_RID},
  {{{SECURITY_WORLD_SID_AUTHORITY}},SECURITY_WORLD_RID},
  {{{SECURITY_LOCAL_SID_AUTHORITY}},SECURITY_LOCAL_RID},
  {{{SECURITY_LOCAL_SID_AUTHORITY}},1}, //SECURITY_LOCAL_LOGON_RID
  {{{SECURITY_CREATOR_SID_AUTHORITY}},SECURITY_CREATOR_OWNER_RID},
  {{{SECURITY_CREATOR_SID_AUTHORITY}},SECURITY_CREATOR_GROUP_RID},
  {{{SECURITY_CREATOR_SID_AUTHORITY}},SECURITY_CREATOR_OWNER_SERVER_RID},
  {{{SECURITY_CREATOR_SID_AUTHORITY}},SECURITY_CREATOR_GROUP_SERVER_RID},
  {{{SECURITY_CREATOR_SID_AUTHORITY}},4}, //???
  {{{SECURITY_NT_AUTHORITY}},SECURITY_DIALUP_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_NETWORK_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_BATCH_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_INTERACTIVE_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_SERVICE_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_ANONYMOUS_LOGON_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_PROXY_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_ENTERPRISE_CONTROLLERS_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_PRINCIPAL_SELF_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_AUTHENTICATED_USER_RID},
  {{{SECURITY_NT_AUTHORITY}},SECURITY_RESTRICTED_CODE_RID},
  {{{SECURITY_NT_AUTHORITY}},13}, //SECURITY_TERMINAL_SERVER_USER_RID
  {{{SECURITY_NT_AUTHORITY}},14}, //SECURITY_REMOTE_INTERACTIVE_LOGON_RID
  {{{SECURITY_NT_AUTHORITY}},SECURITY_LOCAL_SYSTEM_RID},
  {{{SECURITY_NT_AUTHORITY}},19}, //SECURITY_LOCAL_SERVICE_RID
  {{{SECURITY_NT_AUTHORITY}},20}, //SECURITY_NETWORK SERVICE_RID
  {{{SECURITY_MANDATORY_LABEL_AUTHORITY}},SECURITY_MANDATORY_UNTRUSTED_RID},
  {{{SECURITY_MANDATORY_LABEL_AUTHORITY}},SECURITY_MANDATORY_LOW_RID},
  {{{SECURITY_MANDATORY_LABEL_AUTHORITY}},SECURITY_MANDATORY_MEDIUM_RID},
  {{{SECURITY_MANDATORY_LABEL_AUTHORITY}},SECURITY_MANDATORY_HIGH_RID},
  {{{SECURITY_MANDATORY_LABEL_AUTHORITY}},SECURITY_MANDATORY_SYSTEM_RID},
  {{{SECURITY_MANDATORY_LABEL_AUTHORITY}},SECURITY_MANDATORY_PROTECTED_PROCESS_RID},
};

const wchar_t *AddPriveleges[]=
{
  L"SeAssignPrimaryTokenPrivilege",
  L"SeAuditPrivilege",
  L"SeBackupPrivilege",
  L"SeChangeNotifyPrivilege",
  L"SeCreatePagefilePrivilege",
  L"SeCreatePermanentPrivilege",
  L"SeCreateTokenPrivilege",
  L"SeDebugPrivilege",
  L"SeEnableDelegationPrivilege",
  L"SeIncreaseBasePriorityPrivilege",
  L"SeIncreaseQuotaPrivilege",
  L"SeLoadDriverPrivilege",
  L"SeLockMemoryPrivilege",
  L"SeMachineAccountPrivilege",
  L"SeProfileSingleProcessPrivilege",
  L"SeRemoteShutdownPrivilege",
  L"SeRestorePrivilege",
  L"SeSecurityPrivilege",
  L"SeShutdownPrivilege",
  L"SeSyncAgentPrivilege",
  L"SeSystemEnvironmentPrivilege",
  L"SeSystemProfilePrivilege",
  L"SeSystemtimePrivilege",
  L"SeTakeOwnershipPrivilege",
  L"SeTcbPrivilege",
  L"SeUndockPrivilege",
  L"SeUnsolicitedInputPrivilege",
  L"SeManageVolumePrivilege",
  L"SeInteractiveLogonRight",
  L"SeNetworkLogonRight",
  L"SeBatchLogonRight",
  L"SeServiceLogonRight",
};

const TCHAR *AddPrivelegeNames[]=
{
  _T("Replace a process level token"),
  _T("Generate security audits"),
  _T("Back up files and directories"),
  _T("Bypass traverse checking"),
  _T("Create a paging file"),
  _T("Create permanent shared objects"),
  _T("Create a token object"),
  _T("Debug programs"),
  _T("Enable computer and user accounts to be trusted for delegation"),
  _T("Increase scheduling priority"),
  _T("Increase quotas"),
  _T("Load and unload device drivers"),
  _T("Lock pages in memory"),
  _T("Add workstations to domain"),
  _T("Profile single process"),
  _T("Force shutdown from a remote system"),
  _T("Restore files and directories"),
  _T("Manage auditing and security log"),
  _T("Shut down the system"),
  _T("Synchronize directory service data"),
  _T("Modify firmware environment values"),
  _T("Profile system performance"),
  _T("Change the system time"),
  _T("Take ownership of files or other objects"),
  _T("Act as part of the operating system"),
  _T("Remove computer from docking station"),
  _T("Read unsolicited input from a terminal device"),
  _T("Manage volume"),
  _T("Log on locally"),
  _T("Access this computer from the network"),
  _T("Log on as a batch job"),
  _T("Log on as a service"),
};

static const TCHAR *default_column_data=_T("");

static const TCHAR* groups_shortcut=_T(":Groups");
static const TCHAR* rights_shortcut=_T(":Rights");

extern LSA_HANDLE GetPolicyHandle(wchar_t *computer);

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=MAKEFARVERSION(VER_MAJOR,VER_MINOR,0,VER_BUILD,VS_RELEASE);
  Info->Guid=MainGuid;
  Info->Title=L"User Manager";
  Info->Description=L"User Manager";
  Info->Author=L"Vadim Yegorov";
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
  memset(&::Info,0,sizeof(::Info));
  memmove(&::Info,Info,(Info->StructSize>(int)sizeof(::Info))?sizeof(::Info):Info->StructSize);
  IsOldFAR=FALSE;
  ::FSF=*Info->FSF;
  ::Info.FSF=&::FSF;

  HKEY hKey;
  DWORD Type;
  DWORD DataSize=0;
  {
    CFarSettings set(MainGuid);
    set.Get(_T("AddToDisksMenu"),Opt.AddToDisksMenu);
    set.Get(_T("AddToPluginsMenu"),Opt.AddToPluginsMenu);
    set.Get(_T("AddToConfigMenu"),Opt.AddToConfigMenu);
    set.Get(_T("FullUserNames"),Opt.FullUserNames);
    set.Get(_T("Prefix"),Opt.Prefix,ArraySize(Opt.Prefix));
    if(!Opt.Prefix[0])
      _tcscpy(Opt.Prefix,_T("acl"));
  }
  if(!IsPrivilegeEnabled(SE_SECURITY_NAME))
    EnablePrivilege(SE_SECURITY_NAME);
  if(!IsPrivilegeEnabled(SE_TAKE_OWNERSHIP_NAME))
    EnablePrivilege(SE_TAKE_OWNERSHIP_NAME);
  if(!IsPrivilegeEnabled(SE_RESTORE_NAME))
    EnablePrivilege(SE_RESTORE_NAME);
  if(!IsPrivilegeEnabled(SE_BACKUP_NAME))
    EnablePrivilege(SE_BACKUP_NAME);

  EnablePrivilege(SE_CREATE_TOKEN_NAME);
  EnablePrivilege(SE_ASSIGNPRIMARYTOKEN_NAME);
  EnablePrivilege(SE_LOCK_MEMORY_NAME);
  EnablePrivilege(SE_INCREASE_QUOTA_NAME);
  EnablePrivilege(SE_UNSOLICITED_INPUT_NAME);
  EnablePrivilege(SE_MACHINE_ACCOUNT_NAME);
  EnablePrivilege(SE_TCB_NAME);
  EnablePrivilege(SE_SECURITY_NAME);
  EnablePrivilege(SE_TAKE_OWNERSHIP_NAME);
  EnablePrivilege(SE_LOAD_DRIVER_NAME);
  EnablePrivilege(SE_SYSTEM_PROFILE_NAME);
  EnablePrivilege(SE_SYSTEMTIME_NAME);
  EnablePrivilege(SE_PROF_SINGLE_PROCESS_NAME);
  EnablePrivilege(SE_INC_BASE_PRIORITY_NAME);
  EnablePrivilege(SE_CREATE_PAGEFILE_NAME);
  EnablePrivilege(SE_CREATE_PERMANENT_NAME);
  EnablePrivilege(SE_BACKUP_NAME);
  EnablePrivilege(SE_RESTORE_NAME);
  EnablePrivilege(SE_SHUTDOWN_NAME);
  EnablePrivilege(SE_DEBUG_NAME);
  EnablePrivilege(SE_AUDIT_NAME);
  EnablePrivilege(SE_SYSTEM_ENVIRONMENT_NAME);
  EnablePrivilege(SE_CHANGE_NOTIFY_NAME);
  EnablePrivilege(SE_REMOTE_SHUTDOWN_NAME);

  init_current_user();
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
  if(!IsOldFAR)
  {
    Info->StructSize=sizeof(*Info);
    Info->Flags=0;

    static const TCHAR *DisksMenuStrings[1];
    DisksMenuStrings[0]=GetMsg(mNameDisk);
    Info->DiskMenu.Guids=&DiskGuid;
    Info->DiskMenu.Strings=DisksMenuStrings;
    Info->DiskMenu.Count=Opt.AddToDisksMenu?1:0;

    static const TCHAR *PluginMenuStrings[1];
    PluginMenuStrings[0]=GetMsg(mName);
    Info->PluginMenu.Guids=&MenuGuid;
    Info->PluginMenu.Strings=PluginMenuStrings;
    Info->PluginMenu.Count=Opt.AddToPluginsMenu?(sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0])):0;
    Info->PluginConfig.Guids=&MenuGuid;
    Info->PluginConfig.Strings=PluginMenuStrings;
    Info->PluginConfig.Count=Opt.AddToConfigMenu?(sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0])):0;
    Info->CommandPrefix=Opt.Prefix;
  }
}

HANDLE WINAPI OpenW(const struct OpenInfo *anInfo)
{
  if(IsOldFAR) return NULL;
  HANDLE res=NULL;
  CFarPanel pInfo(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO);
  if(pInfo.IsOk())
  {
    UserManager *panel=NULL;
    panel=(UserManager *)malloc(sizeof(UserManager));
    if(panel)
    {
      panel->level=-1;
      if(anInfo->OpenFrom==OPEN_PLUGINSMENU)
      {
        int Msgs[]={mMenuProp,mMenuApply,mMenuSep1,mMenuUsers,mMenuRights};
        FarMenuItem MenuItems[mMenuRights-mMenuProp+1];
        TCHAR ItemText[mMenuRights-mMenuProp+1][128];
        for(unsigned int i=0;i<(mMenuRights-mMenuProp+1);i++)
        {
          MenuItems[i].Flags=0ULL;
          _tcscpy(ItemText[i],GetMsg(Msgs[i]));
          MenuItems[i].Text=ItemText[i];
          if(MenuItems[i].Text[0]=='-')
          {
            ItemText[i][0]=0;
            MenuItems[i].Flags|=MIF_SEPARATOR;
          }
        };
        MenuItems[0].Flags|=MIF_SELECTED;
        int MenuCode=Info.Menu(&MainGuid,&MainMenuGuid,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,NULL,NULL,_T("Contents"),NULL,NULL,MenuItems,sizeof(MenuItems)/sizeof(MenuItems[0]));
        switch(MenuCode)
        {
          case 0:
            if(pInfo.PanelType()==PTYPE_FILEPANEL)
            {
              if(pInfo.SelectedItemsNumber()>0)
              {
                if(pInfo.Plugin())
                  panel->level=parse_dir(pInfo.CurDir(),pInfo[pInfo.CurrentItem()].FileName,NULL,pathtypePlugin,&(panel->param),panel->hostfile,panel->hostfile_oem);
                else
                {
                  panel->level=parse_dir(pInfo.CurDir(),pInfo[pInfo.CurrentItem()].FileName,pInfo[pInfo.CurrentItem()].FileName,pathtypeReal,&(panel->param),panel->hostfile,panel->hostfile_oem);
                }
              }
            }
            else if(pInfo.PanelType()==PTYPE_TREEPANEL)
              panel->level=parse_dir(pInfo.CurDir(),NULL,NULL,pathtypeTree,&(panel->param),panel->hostfile,panel->hostfile_oem);
            break;
          case 1:
            if(pInfo.PanelType()==PTYPE_FILEPANEL)
              ProcessChilds(pInfo);
            break;
          case 3:
            panel->level=levelGroups;
            break;
          case 4:
            panel->level=levelRights;
            break;
        }
      }
      else if(anInfo->OpenFrom==OPEN_LEFTDISKMENU||anInfo->OpenFrom==OPEN_RIGHTDISKMENU)
      {
        panel->level=levelGroups;
      }
      else if(anInfo->OpenFrom==OPEN_COMMANDLINE)
      {
        TCHAR *cmd=(TCHAR *)anInfo->Data;
        TCHAR name[MAX_PATH];
        if(_tcslen(cmd))
        {
          FSF.Unquote(FSF.Trim(_tcscpy(name,cmd)));
          panel->level=parse_dir(name,NULL,NULL,pathtypeUnknown,&(panel->param),panel->hostfile,panel->hostfile_oem);
        }
        else panel->level=levelGroups;
      }
      else if(anInfo->OpenFrom==OPEN_SHORTCUT)
      {
        if(0==_tcscmp(groups_shortcut,(const wchar_t*)anInfo->Data))
        {
          panel->level=levelGroups;
        }
        else if(0==_tcscmp(rights_shortcut,(const wchar_t*)anInfo->Data))
        {
          panel->level=levelRights;
        }
      }
      if(panel->level>-1)
      {
        panel->computer_ptr=NULL;
        panel->flags=0;
        wcscpy(panel->domain,L"");
        if(panel->level==levelRoot)
        {
          DWORD attr=GetFileAttributesW(panel->hostfile);
          if((attr!=0xFFFFFFFF)&&(attr&FILE_ATTRIBUTE_DIRECTORY))
            panel->flags|=FLAG_FOLDER;
          //network support.
          if(!wcsncmp(panel->hostfile,L"\\\\",2))
          {
            panel->flags|=FLAG_NETPATH;
            wchar_t *ptr=wcschr(panel->hostfile+2,'\\');
            if(ptr)
            {
              wcsncpy(panel->computer,panel->hostfile+2,ptr-(panel->hostfile+2));
              panel->computer[ptr-(panel->hostfile+2)]=0;
              ptr++;
              wchar_t *file_ptr=wcschr(ptr,'\\');
//              if(file_ptr)
              {
                wchar_t share[MAX_PATH];
                if(file_ptr)
                {
                  wcsncpy(share,ptr,file_ptr-ptr);
                  share[file_ptr-ptr]=0;
                  file_ptr++;
                }
                else wcscpy(share,ptr);
                NET_API_STATUS res;
                SHARE_INFO_502 *shares;
                DWORD entriesread,totalentries,resumehandle=0;
                res=NetShareEnum(panel->computer,502,(LPBYTE *)&shares,MAX_PREFERRED_LENGTH,&entriesread,&totalentries,&resumehandle);
                if((res==ERROR_SUCCESS)/*||(res==ERROR_MORE_DATA)*/)
                {
                  for(unsigned long i=0;i<entriesread;i++)
                  {
                    if(!wcscmp(share,(wchar_t *)shares[i].shi502_netname))
                    {
                      panel->computer_ptr=panel->computer;
                      wcscpy(panel->domain,(wchar_t *)shares[i].shi502_path);
                      if(file_ptr)
                      {
                        wcsaddendslash(panel->domain);
                        wcscat(panel->domain,file_ptr);
                      }
                      break;
                    }
                  }
                  NetApiBufferFree(shares);
                }
              }
            }
          }
        }
        res=(HANDLE)panel;
      }
      if(res==NULL) free(panel);
    }
  }
  return res;
}

void WINAPI ClosePanelW(const struct ClosePanelInfo *Info)
{
  UserManager *panel=(UserManager *)Info->hPanel;
  free(panel);
}

static void NormalizeCustomColumns(PluginPanelItem *tmpItem)
{
  for(int i=0;i<CUSTOM_COLUMN_COUNT;i++)
    if(!tmpItem->CustomColumnData[i])
      ((TCHAR**)tmpItem->CustomColumnData)[i]=(TCHAR*)default_column_data; //FIXME
  tmpItem->CustomColumnNumber=CUSTOM_COLUMN_COUNT;
}

static void SetDescription(PluginPanelItem *tmpItem,wchar_t *Description)
{
  tmpItem->Description=(TCHAR *)malloc((wcslen(Description)+1)*sizeof(TCHAR));
  if(tmpItem->Description)
  {
    _tcscpy((TCHAR*)tmpItem->Description,Description); //FIXME
  }
}

intptr_t WINAPI GetFindDataW(struct GetFindDataInfo *anInfo)
{
  if(!IsOldFAR)
  {
    UserManager *panel=(UserManager *)anInfo->hPanel;
    anInfo->PanelItem=NULL; anInfo->ItemsNumber=0;
    panel->error=false;
    if(plain_dirs_dir[panel->level])
    {
      int ItemCount=(panel->flags&FLAG_FOLDER)?plain_dirs_dir[panel->level][0]:plain_dirs_dir[panel->level][1];
      int OwnerCount=plain_dirs_owners[panel->level]?1:0;
      anInfo->PanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem)*(ItemCount+OwnerCount));
      if(anInfo->PanelItem)
      {
        anInfo->ItemsNumber=ItemCount+OwnerCount;
        for(int i=0;i<ItemCount;i++)
        {
          (anInfo->PanelItem)[i].FileAttributes=FILE_ATTRIBUTE_DIRECTORY;
          AddDefaultUserdata((anInfo->PanelItem+i),plain_dirs_dir[panel->level][i*2+3],i,0,NULL,GetMsg(plain_dirs_dir[panel->level][i*2+2]),GetMsg(plain_dirs_dir[panel->level][i*2+2]));
        }
        if(OwnerCount)
        {
          wchar_t *owner;
          PSID sid;
          if(plain_dirs_owners[panel->level](panel,&sid,&owner))
          {
            AddDefaultUserdata((anInfo->PanelItem)+ItemCount,-1,ItemCount,0,sid,owner,owner);
          }
        }
      }
    }
    else if(perm_dirs_dir[panel->level])
    {
      AclData *data=NULL;
      panel->error=!GetAcl(panel,panel->level,&data);
      if(!(panel->error))
      {
        int item_count=0;
        item_count=data->Count;
        if(perm_dirs_dir[panel->level]==PERM_KEY)
        { item_count++; }
        else if((perm_dirs_dir[panel->level]==PERM_FF)&&(panel->flags&FLAG_FOLDER))
        { item_count++; item_count++; }
        else if(perm_dirs_dir[panel->level]==PERM_PRINT)
        { item_count++; item_count++; }
        anInfo->PanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem)*item_count);
        if(anInfo->PanelItem)
        {
          anInfo->ItemsNumber=item_count;
          AceData *tmpAce=data->Aces;
          PluginPanelItem *tmpItem=anInfo->PanelItem;
          wchar_t *username;
          int cur_sortorder=0;
          while(tmpAce)
          {
            TCHAR new_filename[MAX_PATH];
            GetUserNameEx(panel->computer_ptr,tmpAce->user,Opt.FullUserNames,&username);
            switch(GetAclState(panel->level,tmpAce->ace_type,tmpAce->ace_flags))
            {
              case UM_ITEM_EMPTY:
                _tcscpy(new_filename,_T(""));
                break;
              case UM_ITEM_SUCCESS:
              case UM_ITEM_ALLOW:
                _tcscpy(new_filename,_T("+."));
                break;
              case UM_ITEM_FAIL:
              case UM_ITEM_DENY:
                _tcscpy(new_filename,_T("-."));
                break;
              case UM_ITEM_SUCCESS_FAIL:
                _tcscpy(new_filename,_T("*."));
                break;
            }
            _tcscat(new_filename,username);
            TCHAR** CustomColumnData=(TCHAR**)malloc(sizeof(TCHAR *)*CUSTOM_COLUMN_COUNT);
            tmpItem->CustomColumnData=CustomColumnData;
            if(CustomColumnData)
            {
              CustomColumnData[0]=(TCHAR *)malloc(7*sizeof(TCHAR));
              if(CustomColumnData[0])
                _tcscpy(CustomColumnData[0],get_access_string(panel->level,tmpAce->ace_mask));
              CustomColumnData[1]=get_sid_string(tmpAce->user);
              NormalizeCustomColumns(tmpItem);
            }
            AddDefaultUserdata(tmpItem,tmpAce->ace_mask,cur_sortorder++,GetAclState(panel->level,tmpAce->ace_type,tmpAce->ace_flags),tmpAce->user,username,new_filename);
            tmpAce=tmpAce->next;
            tmpItem++;
          }
          if(perm_dirs_dir[panel->level]==PERM_KEY)
          {
            tmpItem->FileAttributes=FILE_ATTRIBUTE_DIRECTORY;
            AddDefaultUserdata(tmpItem,panel->level+1,cur_sortorder++,0,NULL,GetMsg(mDirNewKey),GetMsg(mDirNewKey));
            tmpItem++;
          }
          else if((perm_dirs_dir[panel->level]==PERM_FF)&&(panel->flags&FLAG_FOLDER))
          {
            tmpItem->FileAttributes=FILE_ATTRIBUTE_DIRECTORY;
            AddDefaultUserdata(tmpItem,panel->level+1,cur_sortorder++,0,NULL,GetMsg(mDirNewFolder),GetMsg(mDirNewFolder));
            tmpItem++;
            tmpItem->FileAttributes=FILE_ATTRIBUTE_DIRECTORY;
            AddDefaultUserdata(tmpItem,panel->level+2,cur_sortorder++,0,NULL,GetMsg(mDirNewFile),GetMsg(mDirNewFile));
            tmpItem++;
          }
          else if(perm_dirs_dir[panel->level]==PERM_PRINT)
          {
            tmpItem->FileAttributes=FILE_ATTRIBUTE_DIRECTORY;
            AddDefaultUserdata(tmpItem,panel->level+1,cur_sortorder++,0,NULL,GetMsg(mDirNewContainer),GetMsg(mDirNewContainer));
            tmpItem++;
            tmpItem->FileAttributes=FILE_ATTRIBUTE_DIRECTORY;
            AddDefaultUserdata(tmpItem,panel->level+2,cur_sortorder++,0,NULL,GetMsg(mDirNewJob),GetMsg(mDirNewJob));
            tmpItem++;
          }
        }
        else
        {
          panel->error=true;
          if(!(anInfo->OpMode&OPM_FIND)) ShowError(0);
        }
        FreeAcl(data);
      }
      else if(!(anInfo->OpMode&OPM_FIND)) ShowCustomError(mErrorCantGetACL);
    }
    else if(panel->level==levelShared)
    {
      SHARE_INFO_502 *shares;
      DWORD entriesread,totalentries,resumehandle=0;
      NET_API_STATUS err=NetShareEnum(panel->computer_ptr,502,(LPBYTE *)&shares,MAX_PREFERRED_LENGTH,&entriesread,&totalentries,&resumehandle);
      if(err==NERR_Success)
      {
        int count=0;
        for(unsigned long i=0;i<entriesread;i++)
          if(!_wcsicmp((wchar_t *)shares[i].shi502_path,(panel->computer_ptr)?panel->domain:panel->hostfile))
            count++;
        anInfo->PanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem)*count);
        if(anInfo->PanelItem)
        {
          anInfo->ItemsNumber=count;
          for(unsigned long i=0,j=0;i<entriesread;i++)
          {
            if(j>=(unsigned long)count) break;
            if(!_wcsicmp((wchar_t *)shares[i].shi502_path,(panel->computer_ptr)?panel->domain:panel->hostfile))
            {
              (anInfo->PanelItem)[j].FileAttributes=FILE_ATTRIBUTE_DIRECTORY;
              SetDescription((anInfo->PanelItem)+j,(wchar_t *)shares[i].shi502_remark);
              AddDefaultUserdata((anInfo->PanelItem+j),levelSharedIn,0,0,NULL,(wchar_t *)shares[i].shi502_netname,(wchar_t *)shares[i].shi502_netname);
              TCHAR** CustomColumnData=(TCHAR**)malloc(sizeof(TCHAR *)*CUSTOM_COLUMN_COUNT);
              (anInfo->PanelItem)[j].CustomColumnData=CustomColumnData;
              if(CustomColumnData)
              {
                CustomColumnData[0]=(TCHAR *)malloc(11*sizeof(TCHAR));
                if(CustomColumnData[0])
                {
                  if(shares[i].shi502_max_uses==0xffffffff)
                    _tcscpy(CustomColumnData[0],_T("maximum"));
                  else
                    FSF.sprintf(CustomColumnData[0],_T("%ld"),shares[i].shi502_max_uses);
                }
                CustomColumnData[1]=(TCHAR *)malloc(11*sizeof(TCHAR));
                if(CustomColumnData[1])
                  FSF.sprintf(CustomColumnData[1],_T("%ld"),shares[i].shi502_current_uses);
                CustomColumnData[2]=(TCHAR *)malloc((wcslen((wchar_t *)shares[i].shi502_path)+1)*sizeof(TCHAR));
                if(CustomColumnData[2])
                {
                  _tcscpy(CustomColumnData[2],shares[i].shi502_path);
                }
                NormalizeCustomColumns((anInfo->PanelItem)+j);
              }
              j++;
            }
          }
        }
        NetApiBufferFree(shares);
      }
    }
    else if(panel->level==levelPrinterShared)
    {
      HANDLE printer; PRINTER_DEFAULTSW defaults; PRINTER_INFO_2W *data=NULL;
      memset(&defaults,0,sizeof(defaults));
      defaults.DesiredAccess=PRINTER_ALL_ACCESS;
      if(OpenPrinterW(panel->hostfile,&printer,&defaults))
      {
        DWORD Needed;
        if(!GetPrinterW(printer,2,NULL,0,&Needed))
        {
          if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
          {
            data=(PRINTER_INFO_2W *)malloc(Needed);
            if(data)
            {
              if(GetPrinterW(printer,2,(PBYTE)data,Needed,&Needed))
              {
                if(data->Attributes&PRINTER_ATTRIBUTE_SHARED)
                {
                  anInfo->PanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem));
                  if(anInfo->PanelItem)
                  {
                    anInfo->ItemsNumber=1;
                    TCHAR* item_filename=(TCHAR*)malloc((_tcslen(data->pShareName)+1)*sizeof(TCHAR));
                    if(item_filename) _tcscpy(item_filename,data->pShareName);
                    (anInfo->PanelItem)[0].FileName=item_filename;
                  }
                }
              }
              free(data); data=NULL;
            }
          }
        }
        ClosePrinter(printer);
      }
    }
    else if(panel->level==levelGroups)
    {
      LPBYTE groups=NULL,users=NULL;
      DWORD entriesread1,totalentries1; DWORD_PTR resumehandle1=0; //groups
      DWORD entriesread2,totalentries2,resumehandle2=0; //users
      NET_API_STATUS err1,err2;
      if(panel->global)
        err1=NetGroupEnum(panel->domain,1,&groups,MAX_PREFERRED_LENGTH,&entriesread1,&totalentries1,&resumehandle1);
      else
        err1=NetLocalGroupEnum(panel->computer_ptr,1,&groups,MAX_PREFERRED_LENGTH,&entriesread1,&totalentries1,&resumehandle1);
      err2=NetUserEnum((panel->global)?(panel->domain):(panel->computer_ptr),10,FILTER_NORMAL_ACCOUNT,&users,MAX_PREFERRED_LENGTH,&entriesread2,&totalentries2,&resumehandle2);
      if((err1==NERR_Success)&&(err2==NERR_Success))
      {
        wchar_t *group_name;
        wchar_t *group_comment;
        anInfo->PanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem)*(entriesread1+entriesread2+sizeof(AddSID)/sizeof(AddSID[0])+1));
        if(anInfo->PanelItem)
        {
          anInfo->ItemsNumber=entriesread1+entriesread2+sizeof(AddSID)/sizeof(AddSID[0])+1;
          for(unsigned long i=0;i<entriesread1;i++)
          {
            if(panel->global)
            {
              group_name=((GROUP_INFO_1 *)groups)[i].grpi1_name;
              group_comment=((GROUP_INFO_1 *)groups)[i].grpi1_comment;
            }
            else
            {
              group_name=((LOCALGROUP_INFO_1 *)groups)[i].lgrpi1_name;
              group_comment=((LOCALGROUP_INFO_1 *)groups)[i].lgrpi1_comment;
            }
            (anInfo->PanelItem)[i].FileAttributes=FILE_ATTRIBUTE_DIRECTORY;
            bool sid_ok=false;
            DWORD sid_size=0,domain_size=0; PSID sid=NULL; wchar_t *domain=NULL; SID_NAME_USE type;
            if(!LookupAccountNameW((panel->global)?(panel->domain):(panel->computer_ptr),group_name,NULL,&sid_size,NULL,&domain_size,&type))
              if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
              {
                domain=(wchar_t *)malloc(domain_size*sizeof(wchar_t)+sid_size);
                sid=domain+domain_size;
                if(domain)
                  if(LookupAccountNameW((panel->global)?(panel->domain):(panel->computer_ptr),group_name,sid,&sid_size,domain,&domain_size,&type))
                    sid_ok=true;
              }
            if(sid_ok)
              AddDefaultUserdata((anInfo->PanelItem)+i,levelUsers,0,0,sid,group_name,group_name);
            SetDescription((anInfo->PanelItem)+i,group_comment);
            free(domain);
          }
          for(unsigned long i=0;i<entriesread2;i++)
          {
            bool sid_ok=false;
            DWORD sid_size=0,domain_size=0; PSID sid=NULL; wchar_t *domain=NULL; SID_NAME_USE type;
            if(!LookupAccountNameW((panel->global)?(panel->domain):(panel->computer_ptr),((USER_INFO_10 *)users)[i].usri10_name,NULL,&sid_size,NULL,&domain_size,&type))
              if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
              {
                domain=(wchar_t *)malloc(domain_size*sizeof(wchar_t)+sid_size);
                sid=domain+domain_size;
                if(domain)
                  if(LookupAccountNameW((panel->global)?(panel->domain):(panel->computer_ptr),((USER_INFO_10 *)users)[i].usri10_name,sid,&sid_size,domain,&domain_size,&type))
                    sid_ok=true;
              }
            if(sid_ok)
            {
              AddDefaultUserdata((anInfo->PanelItem)+i+entriesread1,levelUsers,0,0,sid,((USER_INFO_10 *)users)[i].usri10_name,((USER_INFO_10 *)users)[i].usri10_name);
              if(is_current_user(sid)) (anInfo->PanelItem)[i+entriesread1].FileAttributes=FILE_ATTRIBUTE_READONLY;
            }
            SetDescription((anInfo->PanelItem)+i+entriesread1,((USER_INFO_10 *)users)[i].usri10_comment);
            free(domain);
          }
          for(unsigned int i=0;i<sizeof(AddSID)/sizeof(AddSID[0]);i++)
          {
            PSID sid=NULL;
            if(AllocateAndInitializeSid(AddSID[i].sid_id,1,AddSID[i].SubAuthority,0,0,0,0,0,0,0,&sid))
            {
              wchar_t *username;
              GetUserNameEx(panel->computer_ptr,sid,false,&username);
              AddDefaultUserdata((anInfo->PanelItem)+i+entriesread1+entriesread2,0,0,0,sid,username,username);
              if(is_current_user(sid)) (anInfo->PanelItem)[i+entriesread1+entriesread2].FileAttributes=FILE_ATTRIBUTE_READONLY;
            }
          }
          for(unsigned int i=0;i<(entriesread1+entriesread2+sizeof(AddSID)/sizeof(AddSID[0]));i++)
          {
            TCHAR** CustomColumnData=(TCHAR**)malloc(sizeof(TCHAR *)*CUSTOM_COLUMN_COUNT);
            (anInfo->PanelItem)[i].CustomColumnData=CustomColumnData;
            if(CustomColumnData)
            {
              if((anInfo->PanelItem)[i].UserData.FreeData)
                CustomColumnData[1]=get_sid_string(GetSidFromUserData((anInfo->PanelItem)[i].UserData.Data));
              NormalizeCustomColumns((anInfo->PanelItem)+i);
            }
          }
          TCHAR* item_filename=(TCHAR*)malloc(6);
          (anInfo->PanelItem)[entriesread1+entriesread2+sizeof(AddSID)/sizeof(AddSID[0])].FileName=item_filename;
          if(item_filename)
            _tcscpy(item_filename,_T(".."));
          (anInfo->PanelItem)[entriesread1+entriesread2+sizeof(AddSID)/sizeof(AddSID[0])].FileAttributes=0;
        }
      }
      if(groups) NetApiBufferFree(groups);
      if(users) NetApiBufferFree(users);
    }
    else if(panel->level==levelUsers)
    {
      LPBYTE members;
      LOCALGROUP_MEMBERS_INFO_1* localMembers=NULL;
      GROUP_USERS_INFO_0* globalMembers=NULL;
      DWORD entriesread,totalentries; DWORD_PTR resumehandle=0;
      NET_API_STATUS err;
      if(panel->global)
      {
        err=NetGroupGetUsers(panel->domain,panel->nonfixed,0,&members,MAX_PREFERRED_LENGTH,&entriesread,&totalentries,&resumehandle);
        globalMembers=(GROUP_USERS_INFO_0*)members;
      }
      else
      {
        err=NetLocalGroupGetMembers(panel->computer_ptr,panel->nonfixed,1,&members,MAX_PREFERRED_LENGTH,&entriesread,&totalentries,&resumehandle);
        localMembers=(LOCALGROUP_MEMBERS_INFO_1*)members;
      }
      if(err==NERR_Success)
      {
        anInfo->PanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem)*(entriesread+1));
        if(anInfo->PanelItem)
        {
          anInfo->ItemsNumber=entriesread+1;
          for(unsigned long i=0;i<entriesread;i++)
          {
            if(panel->global)
            {
              AddDefaultUserdata((anInfo->PanelItem)+i,-1,0,0,NULL,globalMembers[i].grui0_name,globalMembers[i].grui0_name);
            }
            else
            {
              AddDefaultUserdata((anInfo->PanelItem)+i,-1,0,0,localMembers[i].lgrmi1_sid,localMembers[i].lgrmi1_name,localMembers[i].lgrmi1_name);
              if(is_current_user(localMembers[i].lgrmi1_sid)) (anInfo->PanelItem)[i].FileAttributes=FILE_ATTRIBUTE_READONLY;
              TCHAR** CustomColumnData=(TCHAR**)malloc(sizeof(TCHAR *)*CUSTOM_COLUMN_COUNT);
              (anInfo->PanelItem)[i].CustomColumnData=CustomColumnData;
              if(CustomColumnData)
              {
                CustomColumnData[1]=get_sid_string(localMembers[i].lgrmi1_sid);
                NormalizeCustomColumns((anInfo->PanelItem)+i);
              }
            }
          }
          TCHAR* item_filename=(TCHAR*)malloc(6);
          (anInfo->PanelItem)[entriesread].FileName=item_filename;
          if(item_filename)
            _tcscpy(item_filename,_T(".."));
          (anInfo->PanelItem)[entriesread].FileAttributes=0;
        }
        NetApiBufferFree(members);
      }
    }
    else if(panel->level==levelRights)
    {
      anInfo->PanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem)*(sizeof(AddPriveleges)/sizeof(AddPriveleges[0])));
      if(anInfo->PanelItem)
        for(unsigned int i=0;i<sizeof(AddPriveleges)/sizeof(AddPriveleges[0]);i++)
        {
          (anInfo->PanelItem)[i].FileAttributes=FILE_ATTRIBUTE_DIRECTORY;
          AddDefaultUserdata((anInfo->PanelItem)+(anInfo->ItemsNumber),levelRightUsers,0,0,NULL,(wchar_t*)AddPriveleges[i],AddPrivelegeNames[i]);
          (anInfo->ItemsNumber)++;
        }
    }
    else if(panel->level==levelRightUsers)
    {
      LSA_HANDLE PolicyHandle;
      PolicyHandle=GetPolicyHandle(panel->computer);
      if(PolicyHandle)
      {
        LSA_UNICODE_STRING RightName;
        LSA_ENUMERATION_INFORMATION *info; ULONG count=0;
        RightName.Buffer=panel->nonfixed;
        RightName.Length=wcslen(RightName.Buffer)*sizeof(wchar_t);
        RightName.MaximumLength=RightName.Length+sizeof(wchar_t);
        if(LsaEnumerateAccountsWithUserRight(PolicyHandle,&RightName,(PVOID *)&info,&count)==STATUS_SUCCESS)
        {
          anInfo->PanelItem=(PluginPanelItem *)malloc(sizeof(PluginPanelItem)*count);
          if(anInfo->PanelItem)
          {
            wchar_t *username;
            PluginPanelItem *tmpItem=anInfo->PanelItem;
            (anInfo->ItemsNumber)=count;
            for(unsigned long i=0;i<count;i++,tmpItem++)
            {
              GetUserNameEx(panel->computer_ptr,info[i].Sid,Opt.FullUserNames,&username);
              AddDefaultUserdata(tmpItem,-1,0,0,info[i].Sid,username,username);
            }
          }
          LsaFreeMemory((PVOID)info);
        }
        LsaClose(PolicyHandle);
      }
    }
    return TRUE;
  }
  return FALSE;
}

void WINAPI FreeFindDataW(const struct FreeFindDataInfo *anInfo)
{
  for(size_t i=0;i<anInfo->ItemsNumber;i++)
  {
    free((void*)anInfo->PanelItem[i].Description);
    free((void*)anInfo->PanelItem[i].Owner);
    if(anInfo->PanelItem[i].CustomColumnData)
      for(int j=0;j<CUSTOM_COLUMN_COUNT;j++)
        if(anInfo->PanelItem[i].CustomColumnData[j]!=default_column_data)
          free((void*)anInfo->PanelItem[i].CustomColumnData[j]);
    free((void*)anInfo->PanelItem[i].CustomColumnData);
    free((void*)anInfo->PanelItem[i].FileName);
  }
  free(anInfo->PanelItem);
  UserManager *panel=(UserManager *)anInfo->hPanel;
  if(panel->error)
  {
    panel->error=false;
    SetDirectoryInfo info={sizeof(info),anInfo->hPanel,_T(".."),0,0,{0,0}};
    SetDirectoryW(&info);
    Info.PanelControl(anInfo->hPanel,FCTL_UPDATEPANEL,1,0);
    Info.PanelControl(anInfo->hPanel,FCTL_REDRAWPANEL,0,0);
  }
}

intptr_t WINAPI SetDirectoryW(const struct SetDirectoryInfo *Info)
{
  if(!IsOldFAR)
  {
    if(Info->OpMode&OPM_FIND) return FALSE;
    int res=TRUE;
    UserManager *panel=(UserManager *)Info->hPanel;
    TCHAR buff[MAX_PATH]; wchar_t buffW[MAX_PATH];
    if(!_tcscmp(Info->Dir,_T("\\")))
    {
      panel->level=root_dirs[panel->level];
    }
    else if(!_tcscmp(Info->Dir,_T("..")))
    {
      if(up_dirs[panel->level]==panel->level) res=FALSE;
      panel->level=up_dirs[panel->level];
    }
    else if(CheckChDir(Info->hPanel,Info->Dir,buff,buffW,&panel->level))
    {
      if(nonfixed_dirs[panel->level])
      {
        _tcscpy(panel->nonfixed_oem,buff);
        wcscpy(panel->nonfixed,buffW);
      }
    }
    if(!has_nonfixed_dirs[panel->level])
    {
      _tcscpy(panel->nonfixed_oem,_T(""));
      wcscpy(panel->nonfixed,L"");
    }
    GetCurrentPath(panel->level,panel->nonfixed_oem,panel->path);
    return res;
  }
  return FALSE;
}

void WINAPI GetOpenPanelInfoW(struct OpenPanelInfo *Info)
{
  if(!IsOldFAR)
  {
    UserManager *panel=(UserManager *)Info->hPanel;
    Info->StructSize=sizeof(*Info);
    Info->Flags=OPIF_ADDDOTS|OPIF_SHOWNAMESONLY/*|OPIF_FINDFOLDERS*/;
    if((panel->level==levelUsers)||(panel->level==levelGroups))
      Info->Flags|=OPIF_USEATTRHIGHLIGHTING|OPIF_DISABLEHIGHLIGHTING;
    Info->HostFile=NULL;
    if(has_hostfile[panel->level])
      Info->HostFile=panel->hostfile_oem;
    Info->CurDir=panel->path;

    static TCHAR Title[TINY_BUFFER];
    switch(title_type[panel->level])
    {
      case 0:
        FSF.sprintf(Title,_T(" %s:%s\\%s "),GetMsg(title_type_string[panel->level]),FSF.PointToName(panel->hostfile_oem),panel->path);
        break;
      case 1:
        FSF.sprintf(Title,_T(" %s\\%s "),GetMsg(title_type_string[panel->level]),panel->path);
        break;
    }
    Info->PanelTitle=Title;

    static struct PanelMode PanelModesArray[10];
    static TCHAR *ColumnTitles[10][32];
    int msg_start=panel_modes[panel->level];
    const TCHAR *tmp_msg;

    memset(PanelModesArray,0,sizeof(PanelModesArray));
    memset(ColumnTitles,0,sizeof(ColumnTitles));
    for(int i=0;i<10;i++)
    {
      for(int j=0;j<5;j++)
      {
        tmp_msg=GetMsg(msg_start+i*5+j);
        if(tmp_msg[0])
          switch(j)
          {
            case 0:
              PanelModesArray[i].ColumnTypes=(TCHAR*)tmp_msg;
              break;
            case 1:
              PanelModesArray[i].ColumnWidths=(TCHAR*)tmp_msg;
              break;
            case 2:
              PanelModesArray[i].StatusColumnTypes=(TCHAR*)tmp_msg;
              break;
            case 3:
              PanelModesArray[i].StatusColumnWidths=(TCHAR*)tmp_msg;
              break;
            case 4:
              if(tmp_msg[0]-'0') PanelModesArray[i].Flags|=PMFLAGS_FULLSCREEN;
              break;
          }
      }
      int j=0; const TCHAR *scan=PanelModesArray[i].ColumnTypes;
      if(scan)
      {
        while(TRUE)
        {
          if(j==32) break;
          ColumnTitles[i][j]=get_panel_titles[panel->level](scan);
          scan=_tcschr(scan,',');
          if(!scan) break;
          scan++;
          if(!scan[0]) break;
          j++;
        }
        PanelModesArray[i].ColumnTitles=ColumnTitles[i];
      }
    }

    Info->PanelModesArray=PanelModesArray;
    Info->PanelModesNumber=sizeof(PanelModesArray)/sizeof(PanelModesArray[0]);
    Info->StartPanelMode='3';
    static struct KeyBarLabel Labels[]=
    {
      {{VK_F3,0},NULL,NULL}, //0
      {{VK_F4,0},NULL,NULL}, //1
      {{VK_F5,0},NULL,NULL}, //2
      {{VK_F6,0},NULL,NULL}, //3
      {{VK_F7,0},NULL,NULL}, //4
      {{VK_F8,0},NULL,NULL}, //5
      {{VK_F1,SHIFT_PRESSED},NULL,NULL}, //6
      {{VK_F2,SHIFT_PRESSED},NULL,NULL}, //7
      {{VK_F3,SHIFT_PRESSED},NULL,NULL}, //8
      {{VK_F4,SHIFT_PRESSED},NULL,NULL}, //9
      {{VK_F5,SHIFT_PRESSED},NULL,NULL}, //10
      {{VK_F6,SHIFT_PRESSED},NULL,NULL}, //11
      {{VK_F8,SHIFT_PRESSED},NULL,NULL}, //12
      {{VK_F3,LEFT_ALT_PRESSED},NULL,NULL}, //13
      {{VK_F4,LEFT_ALT_PRESSED},NULL,NULL}, //14
      {{VK_F5,LEFT_ALT_PRESSED},NULL,NULL}, //15
      {{VK_F6,LEFT_ALT_PRESSED},NULL,NULL}, //16
      {{VK_F3,LEFT_CTRL_PRESSED|SHIFT_PRESSED},NULL,NULL}, //17
      {{VK_F4,LEFT_CTRL_PRESSED|SHIFT_PRESSED},NULL,NULL}, //18
    };
    static struct KeyBarTitles KeyBar={ArraySize(Labels),Labels};
    //keys
    Labels[0].Text=(TCHAR*)_T("");
    if(panel->level==levelGroups)
    {
      if(panel->global)
        Labels[0].Text=GetMsg(mKeyLocal);
      else
        Labels[0].Text=GetMsg(mKeyGlobal);
    }
    else if(perm_dirs_dir[panel->level]!=PERM_NO)
    {
      Labels[0].Text=GetMsg(mKeyChange);
    }
    Labels[1].Text=GetMsg(label_f4[panel->level]);
    Labels[2].Text=press_f5_from[panel->level]?NULL:_T("");
    Labels[3].Text=GetMsg(label_f6[panel->level]);
    Labels[4].Text=GetMsg(label_f7[panel->level]);
    Labels[5].Text=press_f8[panel->level]?NULL:_T("");
    if(press_f8[panel->level]==TakeOwnership) Labels[5].Text=GetMsg(mKeyTakeOwnership);
    //shift-keys
    Labels[6].Text=_T("");
    Labels[7].Text=_T("");
    Labels[8].Text=_T("");
    Labels[9].Text=GetMsg(label_shift_f4[panel->level]);
    Labels[10].Text=press_f5_from[panel->level]?NULL:_T("");
    Labels[11].Text=GetMsg(label_shift_f6[panel->level]);
    Labels[12].Text=press_f8[panel->level]?NULL:_T("");
    //alt-keys
    Labels[13].Text=_T("");
    Labels[14].Text=GetMsg(label_alt_f4[panel->level]);
    Labels[15].Text=_T("");
    Labels[16].Text=_T("");
    //ctrl+shift-keys
    Labels[17].Text=_T("");
    Labels[18].Text=_T("");
    Info->KeyBar=&KeyBar;

    if(panel->level==levelGroups||panel->level==levelUsers)
    {
      Info->ShortcutData=groups_shortcut;
    }
    else if(panel->level==levelRights||panel->level==levelRightUsers)
    {
      Info->ShortcutData=rights_shortcut;
    }
  }
}

intptr_t WINAPI ProcessPanelInputW(const struct ProcessPanelInputInfo *anInfo)
{
  if(anInfo->Rec.EventType!=KEY_EVENT||!anInfo->Rec.Event.KeyEvent.bKeyDown) return FALSE;
  UserManager *panel=(UserManager *)anInfo->hPanel;
  WORD Key=anInfo->Rec.Event.KeyEvent.wVirtualKeyCode;
  if((panel->error)&&(Key!=VK_RETURN)) return TRUE;
  //F4
  if(IsNone(&anInfo->Rec)&&(Key==VK_F4))
  {
    if(press_f4[panel->level])
      if(press_f4[panel->level](panel))
      {
        Info.PanelControl(anInfo->hPanel,FCTL_UPDATEPANEL,1,0);
        Info.PanelControl(anInfo->hPanel,FCTL_REDRAWPANEL,0,0);
      }
    return TRUE;
  }
  //Alt-F4
  if(IsAlt(&anInfo->Rec)&&(Key==VK_F4))
  {
    if(press_alt_f4[panel->level])
      if(press_alt_f4[panel->level](panel))
      {
        Info.PanelControl(anInfo->hPanel,FCTL_UPDATEPANEL,1,0);
        Info.PanelControl(anInfo->hPanel,FCTL_REDRAWPANEL,0,0);
      }
    return TRUE;
  }
  //Shift-F4
  if(IsShift(&anInfo->Rec)&&(Key==VK_F4))
  {
    if(press_shift_f4[panel->level])
      if(press_shift_f4[panel->level](panel))
      {
        Info.PanelControl(anInfo->hPanel,FCTL_UPDATEPANEL,1,0);
        Info.PanelControl(anInfo->hPanel,FCTL_REDRAWPANEL,0,0);
      }
    return TRUE;
  }
  //[Shift-]F5
  if((IsShift(&anInfo->Rec)||IsNone(&anInfo->Rec))&&(Key==VK_F5))
  {
    if(press_f5_from[panel->level])
    {
      UserManager *anotherpanel=NULL;
      PanelInfo PInfo;
      PInfo.StructSize=sizeof(PanelInfo);
      if(Info.PanelControl(PANEL_PASSIVE,FCTL_GETPANELINFO,0,&PInfo))
      {
        if(IsEqualGUID(PInfo.OwnerGuid,MainGuid))
        {
          anotherpanel=(UserManager*)PInfo.PluginHandle;
          if(anotherpanel&&press_f5[anotherpanel->level])
          {
            if(press_f5[anotherpanel->level](panel,anotherpanel,IsNone(&anInfo->Rec)))
            {
              Info.PanelControl(PANEL_ACTIVE,FCTL_UPDATEPANEL,0,0);
              Info.PanelControl(PANEL_ACTIVE,FCTL_REDRAWPANEL,0,0);
              Info.PanelControl(PANEL_PASSIVE,FCTL_UPDATEPANEL,1,0);
              Info.PanelControl(PANEL_PASSIVE,FCTL_REDRAWPANEL,0,0);
            }
          }
        }
      }
    }
    return TRUE;
  }
  //[Shift-]F8
  if((IsShift(&anInfo->Rec)||IsNone(&anInfo->Rec))&&(Key==VK_F8))
  {
    if(press_f8[panel->level])
      if(press_f8[panel->level](panel,IsNone(&anInfo->Rec)))
      {
        Info.PanelControl(anInfo->hPanel,FCTL_UPDATEPANEL,1,0);
        Info.PanelControl(anInfo->hPanel,FCTL_REDRAWPANEL,0,0);
      }
    return TRUE;
  }
  //[Shift-]F7
  if((IsShift(&anInfo->Rec)||IsNone(&anInfo->Rec))&&(Key==VK_F7))
  {
    if(press_f7[panel->level])
      if(press_f7[panel->level](panel))
      {
        Info.PanelControl(anInfo->hPanel,FCTL_UPDATEPANEL,1,0);
        Info.PanelControl(anInfo->hPanel,FCTL_REDRAWPANEL,0,0);
      }
    return TRUE;
  }
  //[Shift-]F6
  if((IsShift(&anInfo->Rec)||IsNone(&anInfo->Rec))&&(Key==VK_F6))
  {
    if(press_f6[panel->level])
      if(press_f6[panel->level](panel,IsNone(&anInfo->Rec)))
      {
        Info.PanelControl(anInfo->hPanel,FCTL_UPDATEPANEL,1,0);
        Info.PanelControl(anInfo->hPanel,FCTL_REDRAWPANEL,0,0);
      }
    return TRUE;
  }
  //F3
  if(IsNone(&anInfo->Rec)&&(Key==VK_F3))
  {
    if(panel->level==levelGroups)
    {
      panel->global=!panel->global;
      if(panel->global)
      {
        wchar_t *buffer;
        HANDLE hSScr=Info.SaveScreen(0,0,-1,-1);
        const TCHAR *MsgItems[]={_T(""),GetMsg(mOtherDomainSearch)};
        Info.Message(&MainGuid,&MessageGuid,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
        NET_API_STATUS err=NetGetAnyDCName(panel->computer_ptr,NULL,(LPBYTE *)&buffer);
        if(err==ERROR_NO_SUCH_DOMAIN)
        {
          err=NetGetDCName(panel->computer_ptr,NULL,(LPBYTE *)&buffer);
        }
        if(err==NERR_Success)
        {
          wcscpy(panel->domain,buffer);
          NetApiBufferFree(buffer);
        }
        else
        {
          panel->global=false;
        }
        Info.RestoreScreen(hSScr);
      }
      else
        wcscpy(panel->domain,L"");
      Info.PanelControl(anInfo->hPanel,FCTL_UPDATEPANEL,0,0);
      Info.PanelControl(anInfo->hPanel,FCTL_REDRAWPANEL,0,0);
    }
    else if(perm_dirs_dir[panel->level]!=PERM_NO)
    {
      CFarPanel pInfo((HANDLE)panel,FCTL_GETPANELINFO);
      if(pInfo.IsOk())
      {
        if((pInfo.ItemsNumber()>0)&&(!(pInfo[pInfo.CurrentItem()].FileAttributes&FILE_ATTRIBUTE_DIRECTORY)))
        {
          if(pInfo[pInfo.CurrentItem()].UserData.FreeData)
          {
            UpdateAcl(panel,panel->level,GetSidFromUserData(pInfo[pInfo.CurrentItem()].UserData.Data),GetItemTypeFromUserData(pInfo[pInfo.CurrentItem()].UserData.Data),0,actionChangeType);
          }
        }
        Info.PanelControl(anInfo->hPanel,FCTL_UPDATEPANEL,0,0);
        Info.PanelControl(anInfo->hPanel,FCTL_REDRAWPANEL,0,0);
      }
    }
    return TRUE;
  }
  //AltUp,AltDown
  if(IsAlt(&anInfo->Rec)&&(Key==VK_UP||Key==VK_DOWN)&&perm_dirs_dir[panel->level]!=PERM_NO)
  {
    CFarPanel pInfo((HANDLE)panel,FCTL_GETPANELINFO);
    if(pInfo.IsOk())
    {
      if((pInfo.ItemsNumber()>0)&&(!(pInfo[pInfo.CurrentItem()].FileAttributes&FILE_ATTRIBUTE_DIRECTORY)))
      {
        if(pInfo[pInfo.CurrentItem()].UserData.FreeData)
        {
          UpdateAcl(panel,panel->level,GetSidFromUserData(pInfo[pInfo.CurrentItem()].UserData.Data),GetItemTypeFromUserData(pInfo[pInfo.CurrentItem()].UserData.Data),0,(Key==VK_UP)?actionMoveUp:actionMoveDown);
        }
      }
      Info.PanelControl(anInfo->hPanel,FCTL_UPDATEPANEL,0,0);
      Info.PanelControl(anInfo->hPanel,FCTL_REDRAWPANEL,0,0);
    }
    return TRUE;
  }
  return FALSE;
}

intptr_t WINAPI ConfigureW(const struct ConfigureInfo* Info)
{
  (void)Info;
  return Config();
}

void WINAPI ExitFARW(const struct ExitInfo *Info)
{
  (void)Info;
  free_sid_cache();
  free_current_user();
}

intptr_t WINAPI CompareW(const struct CompareInfo *Info)
{
  UserManager *panel=(UserManager *)Info->hPanel;
  if(sort[panel->level])
  {
    if((Info->Item1->UserData.Data)&&(Info->Item2->UserData.Data))
    {
      int res;
      res=GetSortOrderFromUserData(Info->Item1->UserData.Data)-GetSortOrderFromUserData(Info->Item2->UserData.Data);
      if(res) res=res/abs(res);
      return res;
    }
  }
  return -2;
}

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void)hDll;
  (void)dwReason;
  (void)lpReserved;
  return TRUE;
}
