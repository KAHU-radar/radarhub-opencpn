/******************************************************************************
 * updated: 4-5-2012
 * Project:  OpenCPN
 * Purpose:  test Plugin
 * Author:   Jon Gough
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */


#include "wx/wxprec.h"
#include <regex>

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers
#include <wx/stdpaths.h>
#include <wx/timer.h>
#include <wx/event.h>
#include <wx/sysopt.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <wx/filefn.h>
#include <wx/msgdlg.h>
#include <wx/listbook.h>
#include <wx/panel.h>
#include <wx/ffile.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/mstream.h>

#include <wx/aui/aui.h>

#include "radarhub_pi.h"
#include "version.h"
#include "wxWTranslateCatalog.h"

#include "ODAPI.h"

#include "wx/jsonwriter.h"
#include <wx/socket.h>


#ifndef DECL_EXP
#ifdef __WXMSW__
#define DECL_EXP     __declspec(dllexport)
#else
#define DECL_EXP
#endif
#endif

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif

void                    *g_ppimgr;
radarhub_pi          *g_radarhub_pi;
wxBitmap                *m_pdeficon;

// Needed for ocpndc.cpp to compile. Normally would be in glChartCanvas.cpp
float g_GLMinSymbolLineWidth;


// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr)
{
    return new radarhub_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p)
{
    delete p;
}

radarhub_pi::radarhub_pi(void *ppimgr)
:opencpn_plugin_118(ppimgr)
{
    g_ppimgr = ppimgr;
    g_radarhub_pi = this;
    
    l_pDir = new wxString(*GetpPrivateApplicationDataLocation());

    cache = nullptr;
    connector = nullptr;
    preferences_window = nullptr;
    
//    m_shareLocn = GetPluginDataDir("radar_pi") + wxFileName::GetPathSeparator() + _T("data") + wxFileName::GetPathSeparator();
        
    // {l_pDir}/plugins/radarhub_pi/data is the datadir for this plugin
    avro_schema_t person_schema;
    const char  PERSON_SCHEMA[] =
    "{\"type\":\"record\",\
      \"name\":\"Person\",\
      \"fields\":[\
         {\"name\": \"ID\", \"type\": \"long\"},\
         {\"name\": \"First\", \"type\": \"string\"},\
         {\"name\": \"Last\", \"type\": \"string\"},\
         {\"name\": \"Phone\", \"type\": \"string\"},\
         {\"name\": \"Age\", \"type\": \"int\"}]}";
    
    if (ppimgr == nullptr) {
      avro_schema_from_json_literal(PERSON_SCHEMA, &person_schema);
    }
  
    wxMemoryInputStream sm(
        "\211PNG\r\n\032\n\000\000\000\rIHDR\000\000\000 \000\000\000 "
        "\b\006\000\000\000szz\364\000\000\000\004sBIT\b\b\b\b|"
        "\bd\210\000\000\000\011pHYs\000\000\r\327\000\000\r\327\001B("
        "\233x\000\000\000\031tEXtSoftware\000www.inkscape.org\233\356<\032\000\000\004("
        "IDATX\205\355\327]\210]"
        "\325\025\a\360\337:\223\304\304 "
        ">h5\305I\250J\321Z\025\035\343G\374(\001!:\3274\264\005\261\004%\212\017*R\205@#"
        "\021\212\004A\321B\020?"
        "b\373Vh\bC\"h\2339\367\232\001\351\230\207\306\230\351D\215\204\210Z\252\031!"
        "h\255H\035G0sW\037\346\344:"
        "\367\316GF\252\304\a\027\234\207\265\367Z\377\365\337k\357\263\327\332\221\231N\244\024'4:"
        "\346}"
        "\035\343\030\210\245\276\364\013\205\233\244\263q\026\232\322\373\302\277\244]\346{!"
        "W\345\3419c\316e\013b0\026\372\334#\270\017\257H;\025\336\222F&\014tk:"
        "O\3709\256\302\223N\366`\256\314/"
        "\376o\002Q\217K\244\255\3028n\315Z\0368\216\375E\330*u\011\267f-_\233\315~"
        "\3263\020\215\330\200=B\335\250\313]c$"
        "\376\023\253\342\223\270v&\237\254\345\001\243.\027\352\330Sa\314\034c\246\014D="
        "\326b\013\326d-w\307\307q="
        "\302i\006\r\232\3473\027\353r\031\272\214\373\2071\257\345\3159\326\201\3613<\217\337d-"
        "\267\315\231@4\342\a8$"
        "\315\027\356q\225\367\025>\260\327g\306=\255\251W8$"
        "\355\257\266\346R\351\247\302\240\246\273su~0\211\304m\330,"
        "\\\220\275\371\321\334\b\324\243O:E\227\215B\303\251\372|\354\025\205'"
        "\244g\235\351\261\274,\277l\363\331\021]"
        "\026\273\017\033\2601k\371\247\326\\\031\245\360\337\254\345\257\247\004\313\314\266O\2775"
        "J\237\252\353\316LvZ\246tX"
        "\277O\354\324\323i?\305\177\247\363\225^\327\357\316\326X]"
        "\267\322\247\372\255\351\264\237\356\020\336\205\315\331\233#\225\376\271T("
        "\374S\330\030\203\261p\272\275l-hu\036\022~"
        "%<\034\003\261\024*\254\315\025v\233L%\020z\204=\223\364'\024\376`\221k*:"
        "\003\361\32783\032\261)\352\261/\352\261/"
        "\032\261)v\304\202\026\211\336|Wz\324Q\317N\302\331#\364\314J "
        "v\305\017\261\304\270a\210MQ\b5\213\374>W\346\027\206\334\202\375\346y\027?"
        "\021\326\011\353\244e\026{1D\264\300\206<"
        "\203\253cG,"
        "\202\nsI\324c\311\314\0318\252\a\207su\376\033\\\351\307\0309v\243\345C\331\024\376."
        "\034\221V\b\363\2627\017f-\357\020B\351\226V\026\036\312&"
        "\336\264\330\305Pa\036V\264g\241\235\300\304\344pKO\313\305$}"
        "b\354:M\217\342\001M/"
        "\035\333gM[\205\316\013j\030\313\333\364\346l\004\322\361\013CJ!\262\226\333tY\321*<!"
        "\346\344\337!"
        "\235\031\030f\022\3030$\247\034\234\335\322\272\020\2217\344;U\354\220\326)"
        "\274\334a\333\203\2416\275h\317h;\201\246a,"
        "\215\3768\035\354\3656\272'\377zyS\356P\030\325\260-\372\243'"
        "\372\243G\3036\205\321\354\315\347Z\3347E\201\013\215z\003*\314\245U\214\351\011d-"
        "\217\340\210\256\211UW\207n\2271\277m#\272\310j\034P\330\242\260\005\a\252\261\257d\271{"
        "\261\267U\037&0\217T1Z2\265!"
        "I\303X\201\201\312\361~"
        "\343\366\307\213\361\227\2741\337\200\\\231G\361H\365M\221h\304\271\302\203&z\203c\270+*"
        "\3546\231\356&\374#\326G#\272!W\345\207\270_S_4\342\202\351\002\266\005/"
        "\343\034l\307\357\262\226\357U\204\272\261\276\302\356X\360t\367y\251O\277\262\243F\334\\"
        "\325\204\a\324\2354\305g\310|"
        "u\367(\215(\335\336\341[*\365M\027k\346r\234\016b}"
        "\326\362\317\255\361\2018\303\270\247\245\032\016\342U\214\343\n\\\204\277}#"
        "\345\270r\\\213\247\360\313\254\345\356\266\271\301X\370M5$"
        "\263\227\326\272\rJcJ\217\333n\301\361Jq\313o\273\005J\217+"
        "\215\251\3330\233\355\011oJ\277\373my\233\361\261\207I\270A\372\221\260\014\276\365\207"
        "\311\267)'\374m\370="
        "\201\377\001\312\351\235\226\272B\301\307\000\000\000\000IEND\256B`\202",
        1195);
    m_pdeficon = new wxBitmap(wxImage(sm));

    delete l_pDir;
}

radarhub_pi::~radarhub_pi()
{
    if (connector) { connector->Delete(); delete connector; }
    connector = nullptr;
    if (preferences_window) delete preferences_window;
    preferences_window = nullptr;
    // Delete the cache after the connector, or the connector thread could get stuck hanging on a dead mutex!
    if (cache) delete cache;
    cache = nullptr;
}
int radarhub_pi::Init(void)
{
    wxSocketBase::Initialize();
    AddLocaleCatalog( PLUGIN_CATALOG_NAME );
    m_parent_window = GetOCPNCanvasWindow();
    config = GetOCPNConfigObject();

    std::string slash = wxString(wxFileName::GetPathSeparator()).ToStdString();
    std::string migrations = (GetPluginDataDir("radarhub_pi").ToStdString()
                              + slash + "data"
                              + slash + "protocol"
                              + slash + "migrations");

    std::string db_name =(GetpPrivateApplicationDataLocation()->ToStdString()
                          + slash + "routecache.sqlite3");

    try {
        cache = new Routecache(migrations, db_name);
        connector = new Connector(
             cache,
             GetPluginDataDir("radarhub_pi").ToStdString(),
             config);
    } catch (const std::exception& e) {
        std::cerr << e.what() << " Disabling plugin\n";
        return 0;
    }

    connector->Run();
    
    return (
        WANTS_CURSOR_LATLON       |
//        WANTS_TOOLBAR_CALLBACK    |
//        INSTALLS_TOOLBAR_TOOL     |
        WANTS_CONFIG              |
        INSTALLS_TOOLBOX_PAGE     |
        INSTALLS_CONTEXTMENU_ITEMS  |
        WANTS_NMEA_EVENTS         |
        WANTS_NMEA_SENTENCES        |
        //    USES_AUI_MANAGER            |
        WANTS_PREFERENCES         |
        //    WANTS_ONPAINT_VIEWPORT      |
        WANTS_PLUGIN_MESSAGING    |
        WANTS_LATE_INIT           |
        WANTS_MOUSE_EVENTS        |
        WANTS_KEYBOARD_EVENTS
    );
}

void radarhub_pi::LateInit(void)
{
    SendPluginMessage(wxS("CROWDSOURCE_PI_READY_FOR_REQUESTS"), wxS("TRUE"));
    return;
}

bool radarhub_pi::DeInit(void)
{
    wxSocketBase::Shutdown();
    return true;
}

int radarhub_pi::GetAPIVersionMajor()
{
      return OCPN_API_VERSION_MAJOR;
}

int radarhub_pi::GetAPIVersionMinor()
{
      return OCPN_API_VERSION_MINOR;
}

int radarhub_pi::GetPlugInVersionMajor()
{
      return PLUGIN_VERSION_MAJOR;
}

int radarhub_pi::GetPlugInVersionMinor()
{
      return PLUGIN_VERSION_MINOR;
}

int radarhub_pi::GetPlugInVersionPatch()
{
    return PLUGIN_VERSION_PATCH;
}

int radarhub_pi::GetPlugInVersionPost()
{
    return PLUGIN_VERSION_TWEAK;
}

wxString radarhub_pi::GetCommonName()
{
    return _T(PLUGIN_COMMON_NAME);
}

wxString radarhub_pi::GetShortDescription()
{
    return _(PLUGIN_SHORT_DESCRIPTION);
}

wxString radarhub_pi::GetLongDescription()
{
    return _(PLUGIN_LONG_DESCRIPTION);

}

wxBitmap *radarhub_pi::GetPlugInBitmap()
{
    return m_pdeficon;
}

void CrowdsourcePreferencesWindow::OnTimer(wxTimerEvent& event) {
    try {
        if (plugin.cache && plugin.connector) {
            long unsent_datapoints_nr;
            long unsent_tracks_nr;
            plugin.cache->ConnectionStats(
                unsent_datapoints_nr,
                unsent_tracks_nr);

            unsent_datapoints->SetLabel(
                std::to_string(unsent_datapoints_nr));
            unsent_tracks->SetLabel(
                std::to_string(unsent_tracks_nr));

            last_connection->SetLabel(
                plugin.connector->last_connection.Format("%Y-%m-%d %H:%M:%S"));
            transferred_data->SetLabel("<Not yet implemented>");
            // Thread safety: We do not destroy the connector while
            // inside the dialog, and so the socket is not destroyed either
            // once created.
            if (plugin.connector->socket) {
                connection_status->SetLabel(plugin.connector->socket->status);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << " while updating preferences dialog.\n";
    }
}

void radarhub_pi::ShowPreferencesDialog(wxWindow *parent) {
    try {
        if (!preferences_window) {
            preferences_window = new CrowdsourcePreferencesWindow(
                *this, parent, wxID_ANY, wxString("Crowdsource preferences"));
        }

        wxString server;
        long port;
        wxString api_key;
        float min_reconnect_time;
        float max_reconnect_time;
        config->Read("/Server/server", &server, "crowdsource.kahu.earth");
        config->Read("/Server/port", &port, 9900);
        config->Read("/Server/api_key", &api_key, "");
        config->Read("/Connection/min_reconnect_time", &min_reconnect_time, 100.0);
        config->Read("/Connection/max_reconnect_time", &max_reconnect_time, 600.0);

        preferences_window->server->SetValue(server.ToUTF8());
        preferences_window->port->SetValue(std::to_string(port));
        preferences_window->api_key->SetValue(api_key);
        preferences_window->min_reconnect_time->SetValue(min_reconnect_time);
        preferences_window->max_reconnect_time->SetValue(max_reconnect_time);

        if (preferences_window->ShowModal() == wxID_SAVE) {
            config->Write("/Server/server", preferences_window->server->GetValue());
            config->Write("/Server/port", preferences_window->port->GetValue());
            config->Write("/Server/api_key", preferences_window->api_key->GetValue());
            config->Write("/Connection/min_reconnect_time", preferences_window->min_reconnect_time->GetValue());
            config->Write("/Connection/max_reconnect_time", preferences_window->max_reconnect_time->GetValue());
            config->Flush();

            if (connector) { connector->Delete(); delete connector; }
            connector = new Connector(
                 cache,
                 GetPluginDataDir("radarhub_pi").ToStdString(),
                 config);
             connector->Run();
        }
        preferences_window->Destroy();
        delete preferences_window;
        preferences_window = nullptr;
    } catch (const std::exception& e) {
        std::cerr << e.what() << " while updating preferences dialog.\n";
    }
}

void radarhub_pi::SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix)
{
    latitude = pfix.Lat;
    longitude = pfix.Lon;
    sog = pfix.Sog; // Speed over ground
    cog = pfix.Cog; // Course over ground
}

static std::regex nmeaRattmRegex(R"(\$RATTM,(\d{2}),([\d\.\-]+),([\d\.\-]+),([^,]*),([\d\.\-]+),([\d\.\-]+),([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),(..*)\*([A-Fa-f0-9]{2})\s*)");



void radarhub_pi::Polar2Pos(double bearing, double distance, double& lat, double& lon) {
    // latitude & longitude are of own ship, all other variables pertain to a target
    lat = latitude + distance * cos(deg2rad(bearing)) / 60. / 1852.;
    lon = longitude + distance * sin(deg2rad(bearing)) / cos(deg2rad(latitude)) / 60. / 1852.;
}


void radarhub_pi::SetNMEASentence(wxString &sentence)
{
    std::cout << "Crowdsource: Received NMEA" << sentence <<
     " at lat=" << latitude << " lon=" << longitude <<
     " course=" << cog <<
     std::endl;
    if (sentence.StartsWith("$RATTL")) {
    } else if (sentence.StartsWith("$RATTM")) {
        std::smatch match;
        std::string sentence_str = sentence.ToStdString();
        if (!std::regex_match(sentence_str, match, nmeaRattmRegex)) {
          std::cerr << "Failed to parse RATTM NMEA sentence: [" << sentence << "]\n";
          return;
        }
        if (match.size() != 15) {
          std::cerr << "Only parsed " << (match.size() - 1) << " fields of RATTM NMEA sentence: [" << sentence << "]\n";
          return;
        }

        int target_id = std::stoi(match[1]);
        double target_distance = std::stod(match[2]);
        double target_bearing = std::stod(match[3]);
        std::string target_bearing_unit = match[4];
        double target_speed = std::stod(match[5]);
        double target_course = std::stod(match[6]);
        std::string target_course_unit = match[7];
        // target_distance_closes_point_of_approac = std::stod(match[8]);
        // target_time_closes_point_of_approac = std::stod(match[9]);
        std::string target_distance_unit = match[10];
        std::string target_name = match[11];
        std::string target_status = match[12];

        double target_latitude;
        double target_longitude;
        Polar2Pos(target_bearing, target_distance, target_latitude, target_longitude);

        std::cout << "Target ID: " << target_id << std::endl;
        std::cout << "Target Distance: " << target_distance << std::endl;
        std::cout << "Target Distance Unit: " << target_distance_unit << std::endl;
        std::cout << "Bearing from Own Ship: " << target_bearing << std::endl;
        std::cout << "Bearing Unit: " << target_bearing_unit << std::endl;
        std::cout << "Target Speed: " << target_speed << std::endl;
        std::cout << "Target Course: " << target_course << std::endl;
        std::cout << "Course unit: " << target_course_unit << std::endl;
        std::cout << "Target Name: " << target_name << std::endl;
        std::cout << "Target Status: " << target_status << std::endl;
        std::cout << "Target latitude: " << target_latitude << std::endl;
        std::cout << "Target longitude: " << target_longitude << std::endl;

        try {
            cache->Insert(
                target_id,
                target_distance,
                target_bearing,
                target_bearing_unit,
                target_speed,
                target_course,
                target_course_unit,
                target_distance_unit,
                target_name,
                target_status,
                latitude,
                longitude,
                target_latitude,
                target_longitude);
        } catch (const std::exception& e) {
            std::cerr << "Failed to insert target point in cache: " << e.what() << "\n";
        }
    }
}

