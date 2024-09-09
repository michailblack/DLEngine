#pragma once

// Support Windows 10 and later
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00

#include <sdkddkver.h>

#ifdef EXCLUDE_COMMON_WINDOWS_HEADERS
#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NOMCX
#endif

// For ImGUI compilation
#undef NOCLIPBOARD
#undef NOCTLMGR
#undef NONLS

#define NOMINMAX

#define STRICT

#include <Windows.h>
