install(PROGRAMS
    BeamPattern.py
    BeamPatternFrame.py
    console.py
    deltaT.py
    gps_tcp.py
    http.py
    lighting.py
    movieMaker.py
    nosb.py
    sendAIS.py
    serial_gps.py
    udp_gps.py
DESTINATION lib/GeoZui4D/gzmod)

install(DIRECTORY ais deltaT lighting DESTINATION lib/GeoZui4D/gzmod FILES_MATCHING PATTERN "*.py" PATTERN ".svn" EXCLUDE)
add_subdirectory(gz4d)
