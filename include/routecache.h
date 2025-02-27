#ifndef _ROUTECACHE_H_
#define _ROUTECACHE_H_

#include <string>
#include <wx/string.h>
#include <wx/datetime.h>
#include <sqlite3.h>
#include <mutex>
#include "avroutils.h"

class Routecache {
public:
    Routecache(std::string migrations_dir, std::string db_name);
    ~Routecache();

    sqlite3* db = nullptr;
    std::mutex lock;

    // 100 = the number of targets supported by NMEA...
    int route_ids[100]; // DB ids corresponding to NMEA targets
    wxDateTime route_updates[100];
 
    void Insert(
        int target_id,
        double target_distance,
        double target_bearing,
        std::string target_bearing_unit,
        double target_speed,
        double target_course,
        std::string target_course_unit,
        std::string target_distance_unit,
        std::string target_name,
        std::string target_status,
        double latitude,
        double longitude,
        double target_latitude,
        double target_longitude);
    void ConnectionStats(
        long& unsent_datapoints,
        long& unsent_tracks);
    bool Retrieve(AvroValue& route_message);
    void MarkAsSent(AvroValue& route_message);

private:
    std::string migrations_dir;
    std::string db_name;

    void OpenDB();
    void CloseDB();
    void CreateEmpty();
    void Migrate();
    void RunMigration(int i, wxString name);
};

#endif
