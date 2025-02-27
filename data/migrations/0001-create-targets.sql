create table if not exists target (
    target_id integer primary key autoincrement,
    uuid varchar(36) unique
);

create table if not exists target_position (
    id integer primary key autoincrement,
    timestamp datetime default current_timestamp,
    target_id integer references target(id),
    target_distance float,
    target_bearing float,
    target_bearing_unit text,
    target_speed float,
    target_course float,
    target_course_unit text,
    target_distance_unit text,
    target_name text,
    target_status text,
    latitude float,
    longitude float,
    target_latitude float,
    target_longitude float,
    sent integer default false
);
