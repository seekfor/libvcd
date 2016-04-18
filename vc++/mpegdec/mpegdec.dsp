# Microsoft Developer Studio Project File - Name="mpegdec" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=mpegdec - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mpegdec.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mpegdec.mak" CFG="mpegdec - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mpegdec - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "mpegdec - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mpegdec - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "mpegdec - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "CFG_MPEGDEC_SUPPORT" /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "mpegdec - Win32 Release"
# Name "mpegdec - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "libmpeg"

# PROP Default_Filter ""
# Begin Group "mpgdec"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\source\libmpeg\mpgdec\idct.c
# End Source File
# Begin Source File

SOURCE=..\..\source\libmpeg\mpgdec\mpgdec.c
# End Source File
# Begin Source File

SOURCE=..\..\source\libmpeg\mpgdec\vld.c
# End Source File
# End Group
# Begin Group "mpgenc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\source\libmpeg\mpgenc\fdct.c
# End Source File
# Begin Source File

SOURCE=..\..\source\libmpeg\mpgenc\me.c
# End Source File
# Begin Source File

SOURCE=..\..\source\libmpeg\mpgenc\mpgenc.c
# End Source File
# Begin Source File

SOURCE=..\..\source\libmpeg\mpgenc\vlc.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\source\libmpeg\bs.c
# End Source File
# Begin Source File

SOURCE=..\..\source\libmpeg\img.c
# End Source File
# Begin Source File

SOURCE=..\..\source\libmpeg\tbl.c
# End Source File
# End Group
# Begin Group "libdemux"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\source\libdemux\datdmx.c
# End Source File
# Begin Source File

SOURCE=..\..\source\libdemux\m1vdmx.c
# End Source File
# Begin Source File

SOURCE=..\..\source\libdemux\mpgdmx.c
# End Source File
# End Group
# Begin Group "libos"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\source\libos\os.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\source\main.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\bs.h
# End Source File
# Begin Source File

SOURCE=..\include\common.h
# End Source File
# Begin Source File

SOURCE=..\include\datdmx.h
# End Source File
# Begin Source File

SOURCE=..\include\idct.h
# End Source File
# Begin Source File

SOURCE=..\include\img.h
# End Source File
# Begin Source File

SOURCE=..\include\m1vdmx.h
# End Source File
# Begin Source File

SOURCE=..\include\mp3dec.h
# End Source File
# Begin Source File

SOURCE=..\include\mpgdec.h
# End Source File
# Begin Source File

SOURCE=..\include\mpgdmx.h
# End Source File
# Begin Source File

SOURCE=..\include\os.h
# End Source File
# Begin Source File

SOURCE=..\include\pcm.h
# End Source File
# Begin Source File

SOURCE=..\include\std.h
# End Source File
# Begin Source File

SOURCE=..\include\vld.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
