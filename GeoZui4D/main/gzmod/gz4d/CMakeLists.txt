install(FILES __init__.py DESTINATION lib/GeoZui4D/gzmod/gz4d)
install(DIRECTORY base DESTINATION lib/GeoZui4D/gzmod FILES_MATCHING PATTERN "*.py" PATTERN ".svn" EXCLUDE)
install(DIRECTORY MovieMaker DESTINATION lib/GeoZui4D/gzmod/gz4d FILES_MATCHING PATTERN "*.py" PATTERN ".svn" EXCLUDE)
