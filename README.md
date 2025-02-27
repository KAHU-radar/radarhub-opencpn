# KAHU Radar Hub
*A crowdsourcing [OpenCPN](https://opencpn.org/) plugin*

Contribute AIS and ARPA targets from your vessel to crowdsourcing for marine safety!

This plugin lets you upload AIS and radar ARPA targets (or any NMEA) to an internet server. Upload can be continuous, over intermittent internet, or scheduled, and tracks can be downsampled to fit your bandwidth.
The communication protocol is based on [Apache Avro](https://avro.apache.org/) and batches track points so that the overhead for each point above timestamp and lat/lon is low, meaning it is designed to be as bandwidth conservative as possible.

This plugin is designed to be used in conjunction with the [radar_pi](https://github.com/opencpn-radar-pi/radar_pi) radar plugin. Alternatively, it can use [`$RATTM`](https://gpsd.gitlab.io/gpsd/NMEA.html#_ttm_tracked_target_message) and [`$RATTL`](https://gpsd.gitlab.io/gpsd/NMEA.html#_tll_target_latitude_and_longitude) NMEA messages originating from an external radar system.

![Screenshot](screenshot.png)

Some limitations of the current beta version:
* Only supports #RATTM (not $RATTL) NMEA sentences
* Does not collect AIS data, only radar $RATTM sentences
* Only compiled for Linux / debian on intel/amd PCs
* Not published to the plugin app store - you have to load a file using the "Import plugin" button
* The protocol is NOT encrypted
* The protocol is NOT cryptographically signed

Working features:
* Reconnects if it loses internet connectivity
* Uses exponential back off for reconnect, so as to not flood the local network with connection attempts
* Caches received data until it has internet connectibity
* Reports amount of unsent cached data in the preferences dialog
* Reports connectivity status in the preferences dialog
* Uses a basic authentication scheme using an API key

## Download location

Plugin files for various OSes can be downloaded from [Cloudsmith](https://cloudsmith.io/~kahu/repos/radarhub_pi-beta/packages/).

## Installation instructions
 
* Connect a laptop to the same ethernet segment as your radar. Follow the instructions here for any network setup: https://github.com/opencpn-radar-pi/radar_pi/wiki
* Connect the laptop to the internet on board (this might be the same ethernet network, or wifi or a 3g modem)
* Connect the laptop to your AIS / NMEA bus
* Install `OpenCPN` on the laptop
  - Make sure it reports AIS / GPS as online and shows current vessel position, bearing etc. 
* Install the `radar` plugin and configure your radar in it.
  - Make sure you can see the radar picture.
  - Turn on ARPA.
* Make an account on https://crowdsource.kahu.earth and make an API key there
* Install the `radarhub` plugin
  - In its preferences dialog, configure the server address `crowdsource.kahu.earth` and the API key from the previous step
  - Check that it reports that it managed to connect ("last connection" should say a few seconds ago)
* Install the VDR plugin

## Comprehensive test instructions

These steps are in no way necessary for use, but is our checklist for validating the functionality of the plugin,
but it also highlights some of the features and so might be interesting to others:

### While in the port

The following steps are to ensure the plugin is working as intended

* Check that other vessels going in/out of the port are reported as ARPA targets (and a subset as AIS targets of course)
* Check on  https://crowdsource.kahu.earth that these vessels shows up as tracks you have contributed
* Check that they are in the right place on the map
* In the `radarhub` plugin preferences dialog, check that it reports that the last connection was very recently (seconds ago)
* Turn off your internet connection / turn off the internet connection for the opencpn laptop, but not its connection to the radar and AIS/NMEA
* Wait a while, observing ARPA targets of vessels going in and out of the port
* In the `radarhub` plugin preferences dialog, check that it reports that the last connection was roughly when you turned of the internet connection, and that the number of "queued tracks" is not 0 (should match numbers of targets you've seen).
* Turn on the internet connection again
*  In the `radarhub` plugin preferences dialog, check that it reports that the last connection recently (seconds ago), and that the number of "queued tracks" is now 0 again.
* Check on https://crowdsource.kahu.earth that these vessels seen while you where offline now shows up as tracks you have contributed

### On voyages

These steps are to test the capabilities and accuracy of the plugin

* Record your voyage using the VDR plugin
* If you lose internet connection due to beeing outside of 3g coverage, the `radarhub` plugin report that tracks are queued (in the plugin preferences dialog).
* When you get back to shore
  - send us the VDR recording (vdr.txt) to compare to data we received from the plugin
  - send us the date & time of the start and end of your voyage
* When back at shore, check out https://crowdsource.kahu.earth to see the tracks you collected

## Server

An example server written in Python is provided
[here](https://github.com/KAHU-radar/radarhub-server). This server
implements the full protocol, but just dumps all received tracks to
disk in geojson format. It can be used as a simple shore based VDR,
but mostly serves as an example base for anyone wanting to build a
more elaborate server side setup.


## Build instructions

```
apt install build-essential gcc g++ cmake libwxgtk3.2-dev gettext libbz2-dev zlib1g-dev libsqlite3-dev
```

```
mkdir build; cd build; cmake ..; make package; bash cloudsmith-upload.sh
```

(Note that cloudsmith-upload.sh actually modifies the files, and is
necessary to generate the final package. On a local build, it won't
actually upload anything)
