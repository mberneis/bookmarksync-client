/*
 * SyncLib/ConnectSettings.h
 * Copyright(c) 1999, SyncIt.com  Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * -----------------
 * This program is GPL'd.  If you distribute this program or a derivative of
 * this program publicly you must include the source code.  It is easy
 * enough to drop me an email requesting a different license, if necessary.
 *
 * Author:            Terence Way
 * Last Modification: 6 Oct 1999
 *
 * Description:
 *    A set of routines for extracting a browser's connection settings (which
 *    proxy to use).
 */
#ifndef ConnectSettings
#define ConnectSettings

namespace syncit {

   enum ConnectSetting {
      ConnectAutomatic = 0,

      ConnectDirect = 1,
      ConnectHttp,
      ConnectSocks,
      ConnectWinInet,
      ConnectExplorer,
      ConnectNetscape,

      ConnectEnd
   };

   enum ConnectResult {
      ConnectDisabled = 0,

      ConnectEnabled,
      ConnectHttpUrl,
      ConnectSocksUrl,
      ConnectConfigUrl,

      ConnectResultMask = 0x7F,
      ConnectEditable = 0x80
   };

   ConnectSetting GetCurrentConnectSetting();

   /**
    * @param f  one of ConnectSetting  ConnectDirect <= f < ConnectEnd
    * @param pach  pointer to buffer to store URL
    * @param cch   size of buffer pach
    *
    * @return  ConnectDisabled  if setting isn't enabled;
    *          ConnectEnabled   if setting is enabled, but no further settings;
    *          ConnectProxyUrl  if setting indicates a hostname/port;
    *          ConnectCustomUrl if setting indicates an editable hostname/port;
    *          ConnectConfigUrl if setting indicates a browser auto-config URL
    */
   ConnectResult GetConnectSettings(ConnectSetting f,
                                    char *pach,
                                    size_t cch);

   void SetConnectSettings(ConnectSetting f,
                           const char *psz = NULL);


   /**
    * Return true if we're directly connected to the Internet, or if we have
    * autodial enabled AND we're connected to some network.  Return false if
    * autodial is enabled but we're not connected.
    */
   bool IsDialInActive();
}

#endif /* ConnectSettings_H */
