/******************************************************************************
 * $Id: radarhub_pi.h,v 1.0 2015/01/28 01:54:37 jongough Exp $
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN General Drawing Plugin
 * Author:   Jon Gough
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   $EMAIL$   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 */
#ifndef _CROWDSOURCEPI_H_
#define _CROWDSOURCEPI_H_

#ifdef __WXMSW__
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <sstream>
#  define DEBUGSL(x) do { \
std::ostringstream oss; \
oss << x; \
time_t now = time(0); \
tm* localtm = localtime(&now); \
char *stime = asctime(localtm); \
stime[strlen(stime) - 1 ] = 0; \
std::string s1(oss.str()); \
std::string s = stime; \
s += " :: "; \
s += s1; \
s += "\n"; \
std::wstring stemp = std::wstring(s.begin(), s.end()); \
LPCWSTR sw = stemp.c_str(); \
OutputDebugString(sw); } while (0)

#  define DEBUGST(x) do { \
std::string s(""); \
std::ostringstream oss; \
oss << x; \
time_t now = time(0); \
tm* localtm = localtime(&now); \
char *stime = asctime(localtm); \
stime[strlen(stime) - 1 ] = 0; \
do { \
std::string s1(oss.str()); \
s += stime; \
s += " :: "; \
s += s1; } while (0);

#  define DEBUGCONT(x) do { \
std::ostringstream oss; \
oss << x; \
std::string s1(oss.str()); \
s += s1 ; } while (0);

#  define DEBUGEND(x) do { \
std::string s1(""); \
std::ostringstream oss; \
oss << x; \
s1 = oss.str(); \
s += s1; } while (0); \
s += "\n" ; \
std::wstring stemp = std::wstring(s.begin(), s.end()); \
LPCWSTR sw = stemp.c_str(); \
OutputDebugString(sw); } while (0)
#else
#  define DEBUGSL(x) do {} while (0)
#  define DEBUGST(x) do {} while (0)
#  define DEBUGCONT(x) do {} while (0)
#  define DEBUGEND(x) do {} while (0)
#endif
#else
#ifdef DEBUG_BUILD
#  define DEBUGSL(x) do { \
time_t now = time(0); \
tm* localtm = localtime(&now); \
char *stime = asctime(localtm); \
stime[strlen(stime) - 1 ] = 0; \
std::cout << stime << " :: "; \
std::cout << x << std::endl ;} while (0)

#  define DEBUGST(x) do { \
time_t now = time(0); \
tm* localtm = localtime(&now); \
char *stime = asctime(localtm); \
stime[strlen(stime) - 1 ] = 0; \
std::cout << stime << " :: " ; \
std::cout << x; } while (0)

#  define DEBUGCONT(x) do { \
std::cout << x ; } while (0)

#  define DEBUGEND(x) do { \
std::cout << x  << std::endl ; } while (0)
#else
#  define DEBUGSL(x) do {} while (0)
#  define DEBUGST(x) do {} while (0)
#  define DEBUGCONT(x) do {} while (0)
#  define DEBUGEND(x) do {} while (0)
#endif
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#ifdef __OCPN__ANDROID__
#include "qopengl.h"                  // this gives us the qt runtime gles2.h
#endif

#include "wxWTranslateCatalog.h"

#include "ocpn_plugin.h"

#include <wx/string.h>
#include <wx/settings.h>
#include <wx/statline.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/dialog.h>
#include <wx/splitter.h>
#include <wx/fileconf.h>
#include <wx/dynarray.h>

#include "ODAPI.h"
#include <avro.h>
#include <iostream>
#include <sqlite3.h>
#include "routecache.h"
#include "connector.h"
#include "preferences.h"

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------

#ifndef PI
#define PI (3.1415926535897931160E0)
#endif
#ifndef deg2rad
#define deg2rad(x) ((x)*2 * PI / 360.0)
#endif
#ifndef rad2deg
#define rad2deg(x) ((x)*360.0 / (2 * PI))
#endif

class radarhub_pi;

class CrowdsourcePreferencesWindow : public PreferencesWindow {
public:
    CrowdsourcePreferencesWindow(
        radarhub_pi& plugin,
        wxWindow* parent, wxWindowID id, const wxString& title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE)
     : PreferencesWindow(parent, id, title, pos, size, style),
       plugin(plugin),
       timer(this) {

        Bind(wxEVT_TIMER, &CrowdsourcePreferencesWindow::OnTimer, this);

        timer.Start(500);
    };
    ~CrowdsourcePreferencesWindow() {
        timer.Stop();
    };

    void OnTimer(wxTimerEvent& event);

    radarhub_pi& plugin;
    wxTimer timer;
protected:
    friend class radarhub_pi;
};

class radarhub_pi : public opencpn_plugin_118
{
public:

    radarhub_pi(void *ppimgr);
    ~radarhub_pi();

    wxWindow            *m_parent_window;
    wxFileConfig        *config;
    wxString *l_pDir;
 
    //    The required PlugIn Methods
    int Init(void);
    bool DeInit(void);

    int GetAPIVersionMajor();
    int GetAPIVersionMinor();
    int GetPlugInVersionMajor();
    int GetPlugInVersionMinor();
    wxBitmap *GetPlugInBitmap();

    wxString GetCommonName();
    wxString GetShortDescription();
    wxString GetLongDescription();
 
    void SetNMEASentence(wxString &sentence);
    void ShowPreferencesDialog(wxWindow* parent);

    void SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix); // 18

    void LateInit(void); // 110
 
    int GetPlugInVersionPatch(); // 117
    int GetPlugInVersionPost(); // 117  

    void Polar2Pos(double, double, double&, double&);
    double NMEADegMinToDecimal(const std::string& val, const std::string& dir);

    double latitude;
    double longitude;
    double cog;
    double sog;

    Routecache *cache;
    Connector *connector;
    CrowdsourcePreferencesWindow *preferences_window;
};

#endif



