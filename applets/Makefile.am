SUBDIRS = dotdesktop icons small-icons

EXTRA_DIST = mb-applet-battery.c

PREFIXDIR  = $(prefix)
PKGDATADIR = $(datadir)/matchbox
DATADIR    = $(datadir)

INCLUDES = -DDATADIR=\"$(DATADIR)\" -DPKGDATADIR=\"$(PKGDATADIR)\" -DPREFIX=\"$(PREFIXDIR)\" $(LIBMB_CFLAGS)

if WANT_APM
bin_PROGRAMS =                                                    \
  mb-applet-clock    mb-applet-menu-launcher  mb-applet-wireless  \
  mb-applet-launcher mb-applet-system-monitor mb-applet-battery
else
bin_PROGRAMS =                                                    \
  mb-applet-clock    mb-applet-menu-launcher  mb-applet-wireless  \
  mb-applet-launcher mb-applet-system-monitor
endif

LDADD = $(LIBMB_LIBS)

mb_applet_clock_SOURCES =          mb-applet-clock.c
mb_applet_menu_launcher_SOURCES =  mb-applet-menu-launcher.c
mb_applet_wireless_SOURCES =       mb-applet-wireless.c
mb_applet_launcher_SOURCES =       mb-applet-launcher.c  
mb_applet_system_monitor_SOURCES = mb-applet-system-monitor.c 
mb_applet_battery_SOURCES =        mb-applet-battery.c
mb_applet_battery_LDADD   =        $(LIBMB_LIBS) -lapm  