include(`um_ver.m4')dnl
dnl
`#define VER_MAJOR' MAJOR
`#define VER_MINOR' MINOR
`#define VER_BUILD' BUILD
`#define VER_ALLSTR' "MAJOR.MINOR build BUILD TYPE"
`#define COPYRIGHT "Copyright � 2001-2003, ZG"'

`#ifdef UNICODE'
`#define PLUG_DISPLAYNAME "User Manager Plugin for FAR Manager 2.0."'
`#else'
`#define PLUG_DISPLAYNAME "User Manager Plugin for FAR Manager 1.7+."'
`#endif'
`#define PLUG_INTERNELNAME "userman.dll"'
