# -- CMake build rules

install(FILES delfos.json
	DESTINATION ~/.lnav/formats/installed
	PERMISSIONS
	  OWNER_READ OWNER_WRITE
	  GROUP_READ GROUP_WRITE
	  WORLD_READ
)

install(FILES delfosEnzo.conf
	DESTINATION "/${CMAKE_INSTALL_SYSCONFDIR}/delfos"
)

install(FILES 40-delfos.conf
	DESTINATION "/${CMAKE_INSTALL_SYSCONFDIR}/rsyslog.d"
)

# -- eof
