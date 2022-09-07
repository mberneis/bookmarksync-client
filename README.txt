This is the first open-source release of SyncIT.com's BookmarkSync
Windows client.  The current release is version 1.4.

This software is released under the GPL (General Public License).  See
the file COPYING for more details.  In summary: you get the source
code, don't prevent anyone else from getting the source code, and
there is NO WARRANTY.

The client requires Microsoft Visual Studio version 6.0 or later to
build.

To build:
1. Open the BookmarkSync.dsw project file.
2. Select the SyncIT project as the active project.
3. Check the build target (Debug or Release)
4. Build!

The finished client will appear in SyncIT\Release or SyncIT\Debug,
depending on the build settings.

The release version should be exactly 290,816 bytes.


No installation or setup is required, just run the client.
Configuration information is kept in the Windows Registry, under
  HKEY_CURRENT_USER\Software\SyncIT\BookmarkSync

There are two keys of particular interest:
1. Root: a string of the form "http://www.syncit.com/"
   Please leave the trailing slash.  This is the web site to sync
   against.

2. File3: a string of the form "client/syncit3.php?"
   This is the client script on the web site that handles sync
   requests.

If you're moving the client to a different web site, simply change
these two variables.  All other actions, like web help, registration,
will point to the correct pages based on "Root".

Have fun!
Terence Way
CTO, SyncIT.com
tway@syncit.com
