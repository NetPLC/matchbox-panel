pngs = \
broken-0.png  minisys.png        no-link-0.png     signal-61-80.png  \
mbmenu.png    minitime.png       signal-1-40.png   signal-81-100.png \
mbterm.png    miniwave-icon.png  signal-41-60.png  term.png          

xpms = \
battery.xpm   minisys.xpm        no-link-0.xpm    signal-41-60.xpm   \
broken-0.xpm  minitimestrip.xpm  oval_filled.xpm  signal-61-80.xpm   \
mbmenu.xpm    miniwave.xpm       signal-1-40.xpm  signal-81-100.xpm  

EXTRA_DIST = $(xpms) $(pngs)

if WANT_PNGS
imgs = $(pngs)
else
imgs = $(xpms)
endif

pixmapsdir = $(datadir)/pixmaps/

if WANT_SMALL_ICONS
pixmaps_DATA =
else
pixmaps_DATA = $(imgs)
endif