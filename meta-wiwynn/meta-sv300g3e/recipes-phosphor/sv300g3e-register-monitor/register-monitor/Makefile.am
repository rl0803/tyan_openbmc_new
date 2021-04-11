sbin_PROGRAMS = system-restart-monitor

system_restart_monitor_SOURCES = system-restart-monitor.cpp
system_restart_monitor_LDFLAGS = $(SYSTEMD_LIBS) \
                         $(SDBUSPLUS_LIBS) \
                         -pthread \
                         $(PTHREAD_LIBS) \
                         $(PHOSPHOR_DBUS_INTERFACES_LIBS) \
                         $(BOOST_CPPFLAGS)
system_restart_monitor_CFLAGS = $(SYSTEMD_CFLAGS) \
                        $(SDBUSPLUS_CFLAGS) \
                        -pthread \
                        $(PTHREAD_CFLAGS) \
                        $(PHOSPHOR_DBUS_INTERFACES_CFLAGS) \
                        $(BOOST_CPPFLAGS)
