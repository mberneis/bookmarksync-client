# Microsoft Developer Studio Project File - Name="SyncLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=SyncLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SyncLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SyncLib.mak" CFG="SyncLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SyncLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "SyncLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SyncLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I ".." /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "SyncLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I ".." /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "SyncLib - Win32 Release"
# Name "SyncLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Base64.cxx
# End Source File
# Begin Source File

SOURCE=.\BinarySearch.cxx
# End Source File
# Begin Source File

SOURCE=.\BitmapFileImage.cxx
# End Source File
# Begin Source File

SOURCE=.\BitmapImage.cxx
# End Source File
# Begin Source File

SOURCE=.\BufferedInputStream.cxx
# End Source File
# Begin Source File

SOURCE=.\BufferedOutputStream.cxx
# End Source File
# Begin Source File

SOURCE=.\ByteArrayOutputStream.cxx
# End Source File
# Begin Source File

SOURCE=.\Character.cxx
# End Source File
# Begin Source File

SOURCE=.\ConnectSettings.cxx
# End Source File
# Begin Source File

SOURCE=.\CsvParser.cxx
# End Source File
# Begin Source File

SOURCE=.\DateTime.cxx
# End Source File
# Begin Source File

SOURCE=.\Errors.cxx
# End Source File
# Begin Source File

SOURCE=.\FileInputStream.cxx
# End Source File
# Begin Source File

SOURCE=.\FileOutputStream.cxx
# End Source File
# Begin Source File

SOURCE=.\HTML.cxx
# End Source File
# Begin Source File

SOURCE=.\HttpRequest.cxx
# End Source File
# Begin Source File

SOURCE=.\IconImage.cxx
# End Source File
# Begin Source File

SOURCE=.\Image.cxx
# End Source File
# Begin Source File

SOURCE=.\Log.cxx
# End Source File
# Begin Source File

SOURCE=.\NetscapeInfo.cxx
# End Source File
# Begin Source File

SOURCE=.\PostOutputStream.cxx
# End Source File
# Begin Source File

SOURCE=.\PrintWriter.cxx
# End Source File
# Begin Source File

SOURCE=.\RegKey.cxx
# End Source File
# Begin Source File

SOURCE=.\Socket.cxx
# End Source File
# Begin Source File

SOURCE=.\SocketError.cxx
# End Source File
# Begin Source File

SOURCE=.\SocksSocket.cxx
# End Source File
# Begin Source File

SOURCE=.\text.cxx
# End Source File
# Begin Source File

SOURCE=.\URL.cxx
# End Source File
# Begin Source File

SOURCE=.\UTF8.cxx
# End Source File
# Begin Source File

SOURCE=.\Util.cxx
# End Source File
# Begin Source File

SOURCE=.\WinInetHttpRequest.cxx
# End Source File
# Begin Source File

SOURCE=.\WWWFormDataReader.cxx
# End Source File
# Begin Source File

SOURCE=.\XML.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Base64.h
# End Source File
# Begin Source File

SOURCE=.\BinarySearch.h
# End Source File
# Begin Source File

SOURCE=.\BitmapFileImage.h
# End Source File
# Begin Source File

SOURCE=.\BitmapImage.h
# End Source File
# Begin Source File

SOURCE=.\BufferedInputStream.h
# End Source File
# Begin Source File

SOURCE=.\BufferedOutputStream.h
# End Source File
# Begin Source File

SOURCE=.\ByteArrayOutputStream.h
# End Source File
# Begin Source File

SOURCE=.\Character.h
# End Source File
# Begin Source File

SOURCE=.\ConnectSettings.h
# End Source File
# Begin Source File

SOURCE=.\CriticalSection.h
# End Source File
# Begin Source File

SOURCE=.\CsvParser.h
# End Source File
# Begin Source File

SOURCE=.\DateTime.h
# End Source File
# Begin Source File

SOURCE=.\Errors.h
# End Source File
# Begin Source File

SOURCE=.\FileInputStream.h
# End Source File
# Begin Source File

SOURCE=.\FileOutputStream.h
# End Source File
# Begin Source File

SOURCE=.\HTML.h
# End Source File
# Begin Source File

SOURCE=.\HttpRequest.h
# End Source File
# Begin Source File

SOURCE=.\IconImage.h
# End Source File
# Begin Source File

SOURCE=.\Image.h
# End Source File
# Begin Source File

SOURCE=.\InputStream.h
# End Source File
# Begin Source File

SOURCE=.\Log.h
# End Source File
# Begin Source File

SOURCE=.\NetscapeInfo.h
# End Source File
# Begin Source File

SOURCE=.\OutputStream.h
# End Source File
# Begin Source File

SOURCE=.\PostOutputStream.h
# End Source File
# Begin Source File

SOURCE=.\PrintWriter.h
# End Source File
# Begin Source File

SOURCE=.\Reader.h
# End Source File
# Begin Source File

SOURCE=.\RegKey.h
# End Source File
# Begin Source File

SOURCE=.\Socket.h
# End Source File
# Begin Source File

SOURCE=.\SocketError.h
# End Source File
# Begin Source File

SOURCE=.\SocksSocket.h
# End Source File
# Begin Source File

SOURCE=.\StringBuffer.h
# End Source File
# Begin Source File

SOURCE=.\SyncITMsg.h
# End Source File
# Begin Source File

SOURCE=.\text.h
# End Source File
# Begin Source File

SOURCE=.\Timer.h
# End Source File
# Begin Source File

SOURCE=.\URL.h
# End Source File
# Begin Source File

SOURCE=.\UTF8.h
# End Source File
# Begin Source File

SOURCE=.\Util.h
# End Source File
# Begin Source File

SOURCE=.\WinInetHttpRequest.h
# End Source File
# Begin Source File

SOURCE=.\WWWFormDataReader.h
# End Source File
# Begin Source File

SOURCE=.\XML.h
# End Source File
# End Group
# End Target
# End Project
