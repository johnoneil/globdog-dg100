# globdog-dg100
QT based GUI manager for the Globalsat DG-100 GPS data logger

Project imported from Google Code at shutdown. Unsure of current status.

#Description

This is a Qt based Linux manager application for the Globalsat DG-100 GPS datalogger. It allows the user to control DG-100 settings, download track files off the DG-100, engage live GPS 'mouse' mode, and export GPX files to google earth. It also allows basic viewing of trackfiles via Google Maps.

##Status

I imported this old project from Google Code when it was shutdown. Project should build if the proper tools are available but it is quite old.

##Summary

Use the GlobDoG to connect to your DG-100, and adjust settings (as outlined in the DG-100 manual.)
![Alt text](/img/image3.png?raw=true "Basic UI")


Use the GlobDoG to display real-time NMEA data from your DG-100 via GPS "mouse mode."

![Alt text](/img/image1.png?raw=true "Mousemode image")

Use the GlobDoG to download your DG-100 recorded trackfiles. View the tracks in Google Maps, and export selected files as standard GPX files. GPX can be imported to Google Earth.

![Alt text](/img/image2.png?raw=true "Map Image")

##Requirements

* A GlobalSat DG-100 GPS data logger device.
* QT4 support
* Standard Prolific USB to Serial drivers installed
