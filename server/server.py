import os
import socketserver
import avro.schema
import avro.io
import json
import shapely.geometry
import geopandas
import threading
import io
import datetime
import traceback
import uuid as uuidmod

with open("../data/proto_avro.json") as f:
    schema = avro.schema.parse(f.read())
    
class MyTCPHandler(socketserver.BaseRequestHandler):
    def handle(self):
        try:
            self.writer = avro.io.DatumWriter(schema)
            self.encoder = avro.io.BinaryEncoder(self.request.makefile('wb', buffering=0))            
            self.decoder = avro.io.BinaryDecoder(self.request.makefile('rb', buffering=0))
            self.reader = avro.io.DatumReader(schema)
            while True:
                try:
                    msg = self.reader.read(self.decoder)
                except avro.errors.InvalidAvroBinaryEncoding as e:
                    break
                print("Message", msg)
                msg = msg["Message"]
                if "Call" in msg:
                    call_id = msg["Call"]["id"]
                    fn, args = next(iter(msg["Call"]["Call"].items()))
                    try:
                        response = getattr(self, "call_" + fn)(**args)
                    except Exception as e:
                        self.write_response(call_id, {"Error": {"exception": str(e)}})
                    else:
                        self.write_response(call_id, {fn: response})
        except Exception as e:
            print(e)
            traceback.print_exc()

    def write_response(self, call_id, msg):
        resp = {"Message": {"Response": {"id": call_id, "Response": msg}}}
        print("response", resp)
        self.writer.write(
            resp,
            self.encoder)

    def call_Login(self, apikey):
        self.apikey = apikey
        return {}
        
    def call_Submit(self, start, route, nmea=None, uuid=None):
        start = datetime.datetime.fromtimestamp(
            start / 1000, tz=datetime.timezone.utc)

        useduuid = uuid or uuidmod.uuid4()
        name = "%s.geojson" % (useduuid,)

        with open(name, "w") as f:
            f.write(
                geopandas.GeoDataFrame(
                    [{"apikey": self.apikey, "nmea": nmea, "start": start.isoformat()}],
                    geometry=[shapely.geometry.LineString(
                        [[point["lon"], point["lat"], point["timestamp"]]
                         for point in route])], crs=4326).to_json())

        if uuid is not None:
            return {}
        return {"uuid": str(useduuid)}
    
server = socketserver.ThreadingTCPServer(("0.0.0.0", 9900), MyTCPHandler)
print("Socket server running on port 9900")
server.serve_forever()
