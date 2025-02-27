CREATE INDEX idx_sent ON target_position(sent);
CREATE INDEX idx_target_id ON target_position(target_id);
CREATE INDEX idx_sent_target_id_timestamp ON target_position(sent, target_id, timestamp);
