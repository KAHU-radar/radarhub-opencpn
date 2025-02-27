#include <iostream>
#include <vector>
#include <set>
#include <fstream>
#include <wx/dir.h>
#include <wx/string.h>
#include <wx/filename.h>
#include <wx/utils.h>
#include <thread>
#include <mutex>
#include <random>
#include <sstream>
#include <iomanip>
#include <string>
#include "routecache.h"
#include "sqlutils.h"

bool DoesTableExist(sqlite3* db, const std::string& tableName) {
    Query query = Query(db, "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='" + tableName + "';");

    if (!query.step()) return false; // Shouldn't happen
    
    return query.get_int(0) > 0;
}

std::string generateUUID() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

    char uuid[37];
    std::snprintf(
        uuid, sizeof(uuid),
        "%08x-%04x-%04x-%04x-%012x",
        dist(gen),
        dist(gen) & 0xFFFF,
        (dist(gen) & 0x0FFF) | 0x4000,
        (dist(gen) & 0x3FFF) | 0x8000,
        dist(gen)
    );
    return std::string(uuid);
}

Routecache::Routecache(std::string migrations_dir, std::string db_name) {
    db = NULL;

    this->migrations_dir = migrations_dir;
    this->db_name = db_name;

    for (int idx = 0; idx < 100; idx++) {
        route_updates[idx] = wxDateTime::UNow();
        route_updates[idx].Set(1, wxDateTime::Jan, 1970, 0, 0, 0, 0);
    }
    
    try {
        OpenDB();
        CreateEmpty();
        Migrate();
        return;
    } catch (const std::exception& e) {
        std::cerr << e.what() << ". Deleting route cache.\n";
        if (std::remove(db_name.c_str()) != 0) {
            std::cerr << "Unable to delete route cache: " << db_name << "\n";
        }
        CloseDB();
        throw e;
    }
}

Routecache::~Routecache() {
    if (db != NULL) sqlite3_close(db);
    db = NULL;
}

void Routecache::OpenDB() {
    std::lock_guard<std::mutex> guard(lock);
    int res = sqlite3_open(db_name.c_str(), &db);
    if (res == SQLITE_OK) return;
    db = NULL;
    throw QueryException(db, "open " + db_name);
}

void Routecache::CloseDB() {
    std::lock_guard<std::mutex> guard(lock);
    if (db == NULL) return;
    sqlite3_close(db);
    db = NULL;
}

void Routecache::CreateEmpty() {
    std::lock_guard<std::mutex> guard(lock);
    if (DoesTableExist(db, "migrations")) return;
    Query(db, R"(
        CREATE TABLE IF NOT EXISTS migrations (
            id integer,
            name text,
            applied datetime default current_timestamp
        );
    )").step();
}

void Routecache::Migrate() {
    int max_id;
    {
        std::lock_guard<std::mutex> guard(lock);
        Query query(db, "SELECT max(id) FROM migrations;");
        query.step();
        max_id = query.get_int(0);
    }

    wxString path(migrations_dir);
    wxDir dir(path);
    if (!dir.IsOpened()) {
        throw std::runtime_error("Unable to open migrations directory: " + migrations_dir);
    }

    std::set<wxString> sqlFiles;
    wxString filename;
    for (bool cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES | wxDIR_DIRS);
         cont;
         cont = dir.GetNext(&filename)) {
        sqlFiles.insert(filename);
    }

    wxString slash(wxFileName::GetPathSeparator());
    
    for (const auto& filename : sqlFiles) {
     int migration_id = std::stoi(std::string(filename.ToUTF8()));
        if (migration_id > max_id) {
            wxString pth = path + slash + filename;
            RunMigration(migration_id, pth);
        }
    }
}

void Routecache::RunMigration(int i, wxString name) {
    std::lock_guard<std::mutex> guard(lock);
    std::cerr << "Running migration " << i << ": " << name.ToStdString() << "\n";

    wxFile file(name);
    wxString sql;
    if (!file.IsOpened()) {
        throw std::runtime_error("Unable to open file: " + name.ToStdString());
    }    
    file.ReadAll(&sql, wxConvUTF8);

    std::string std_sql(sql.ToUTF8());
     
    // std::cerr << "<MigrationSQL>\n" << std_sql << "\n</MigrationSQL>\n\n";

    Query query(db, std_sql);
    do {
        query.step();
    } while (query.next());


    std::string strname(name.ToUTF8());
    
    Query(db, R"(
      insert into migrations (id, name) values (?, ?);
    )")
     .bind(1, i)
     .bind(2, strname)
     .step();
}

void Routecache::Insert(
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
    double target_longitude) {
    std::lock_guard<std::mutex> guard(lock);

    wxDateTime now = wxDateTime::Now();
 
    if (now
        .Subtract(
                  route_updates[target_id])
        .GetSeconds()
        .GetValue() > 60) {
        std::string uuid = generateUUID();
        Query(db, R"(
          insert into target (uuid) values (?);
        )").bind(1, uuid).step();
        Query query(db, R"(
          select target_id from target where uuid = ?;
        )");
        query.bind(1, uuid);
        query.step();
        route_ids[target_id] = query.get_int(0);
    }
    route_updates[target_id] = now;
    
    Query(db, R"(
        insert into target_position (
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
            target_longitude
        ) values (
          ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
      )")
     .bind(1, route_ids[target_id])
     .bind(2, target_distance)
     .bind(3, target_bearing)
     .bind(4, target_bearing_unit)
     .bind(5, target_speed)
     .bind(6, target_course)
     .bind(7, target_course_unit)
     .bind(8, target_distance_unit)
     .bind(9, target_name)
     .bind(10, target_status)
     .bind(11, latitude)
     .bind(12, longitude)
     .bind(13, target_latitude)
     .bind(14, target_longitude)
     .step();
}

void Routecache::ConnectionStats(
    long& unsent_datapoints,
    long& unsent_tracks) {
    std::lock_guard<std::mutex> guard(lock);

    Query query1(db, R"(
      select
        count(*)
      from
        target_position
      where       
        not sent;
      )");
    query1.step();
    unsent_datapoints = query1.get_int64(0);
    
    Query query2(db, R"(
      select
        count(*)
      from
        (select distinct
           target_id
         from
           target_position
         where
           not sent
        );
    )");
    query2.step();
    unsent_tracks = query2.get_int64(0);
}

bool Routecache::Retrieve(AvroValue& route_message) {
    std::lock_guard<std::mutex> guard(lock);

    Query query(db, R"(
      select
        target.uuid,
        target_position.timestamp,
        (strftime('%s', timestamp) +  strftime('%f', timestamp) - strftime('%S', timestamp)) * 1000
          as timestamp_epoch,
        target_position.target_latitude,
        target_position.target_longitude
      from
        target_position,
        target
      where
        target_position.target_id = (
          select
            target_id
          from
            target_position
          where
            not sent
            and target_id in (
              select
                target_id
              from
                target_position
              group by 
                target_id
              having 
                count(*) > 1
            )
          order by
            timestamp ASC
          limit 1)
        and target.target_id = target_position.target_id
        and not target_position.sent
        order by timestamp asc
        limit 100;
      )");

    std::string uuid;
    std::string timestamp;

    AvroValue linestring = route_message.Get("route");
    bool isfirst = true;
    double start;
    route_message.Get("nmea").SetCurrentBranch(0);
    
    while (query.step()) {
        if (isfirst) {
            route_message.Get("uuid").SetCurrentBranch(1).Set(query.get_string(0));
            start = query.get_double(2);
            route_message.Get("start").Set((int64_t) start);
            isfirst = false;
        }
        AvroValue position = linestring.Append();
        position.Get("lat").Set((float) query.get_double(3));
        position.Get("lon").Set((float) query.get_double(4));
        position.Get("timestamp").Set(
            (float) (query.get_double(2) - start));
    }
    return !isfirst;
}

void Routecache::MarkAsSent(AvroValue& route_message) {
    std::lock_guard<std::mutex> guard(lock);

    double end =
     ((long) route_message
      .Get("route")
      .Get(-1)
      .Get("timestamp")
      .GetFloat())
     + route_message
     .Get("start")
     .GetLong();

    std::string uuid = route_message.Get("uuid").Get().GetString();
    
    Query query(db, R"(
      update
        target_position
      set
        sent = 1
      where
        target_id = (select target_id from target where uuid = ?)
        and timestamp <= datetime(? / 1000, 'unixepoch');
    )");
    
    query
     .bind(1, uuid)
     .bind(2, end)
     .step();
    std::cerr
     << "Updated "
     << std::to_string(query.changes())
     << " rows for "
     << uuid
     << " @ "
     << end
     << "." << std::endl;
}

