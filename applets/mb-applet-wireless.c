/*
 *  miniwave - Tiny 820.11 wireless 
 *
 *  Note: you can use themes from http://www.eskil.org/wavelan-applet/  
 *
 *  originally based on wmwave
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#include <libmb/mb.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(text) gettext(text)
#else
# define _(text) (text)
#endif

#define HAVE_LINUX_WIRELESS_H 1

#ifdef HAVE_LINUX_WIRELESS_H

#include <sys/types.h>
#include <sys/ioctl.h>

#include <netdb.h>      /* gethostbyname, getnetbyname */
#include <linux/if_arp.h>   /* For ARPHRD_ETHER */
#include <linux/socket.h>   /* For AF_INET & struct sockaddr */
#include <sys/socket.h>       /* For struct sockaddr_in */
#include <linux/wireless.h>

#endif

#ifdef MB_HAVE_PNG
#define IMG_EXT "png"
#else
#define IMG_EXT "xpm"
#endif

static int LastImg = -1;

enum {
  MW_BROKE = 0,
  MW_NO_LINK,
  MW_SIG_1_40,
  MW_SIG_41_60,
  MW_SIG_61_80,
  MW_SIG_80_100,
};

#define  MW_BROKE_IMG      "broken-0."      IMG_EXT
#define  MW_NO_LINK_IMG    "no-link-0."     IMG_EXT
#define  MW_SIG_1_40_IMG   "signal-1-40."   IMG_EXT
#define  MW_SIG_41_60_IMG  "signal-41-60."  IMG_EXT
#define  MW_SIG_61_80_IMG  "signal-61-80."  IMG_EXT
#define  MW_SIG_80_100_IMG "signal-81-100." IMG_EXT

static char *ImgLookup[64] = {
  MW_BROKE_IMG,      
  MW_NO_LINK_IMG,    
  MW_SIG_1_40_IMG,   
  MW_SIG_41_60_IMG,  
  MW_SIG_61_80_IMG,  
  MW_SIG_80_100_IMG, 

};

static char *ThemeName = NULL;
static MBPixbuf *pb;
static MBPixbufImage *Imgs[6] = { 0,0,0,0,0,0 }, 
  *ImgsScaled[6] = { 0,0,0,0,0,0 };
static int CurImg = MW_BROKE;

struct {

   char  iface[5];
   char  status[3];
   float link;
   float level;
   float noise;
   int   nwid;
   int   crypt;
   int   misc;
   int   mode;   

} Mwd;

#define HAVE_LINUX_WIRELESS_H 1

#ifdef HAVE_LINUX_WIRELESS_H

typedef struct iw_range     iwrange;
typedef struct iw_param     iwparam;
typedef struct iw_freq      iwfreq;
typedef struct iw_priv_args iwprivargs;
typedef struct sockaddr     sockaddr;

typedef struct iw_info
{

  	char      name[IFNAMSIZ];     /* Wireless/protocol name */
  	int       has_nwid;
  	iwparam   nwid;           /* Network ID */
  	int       has_freq;
  	float     freq;           /* Frequency/channel */
  	int       has_sens;
  	iwparam   sens;           /* sensitivity */
  	int       has_key;
  	unsigned char key[IW_ENCODING_TOKEN_MAX]; /* Encoding key used */
  	int       key_size;       /* Number of bytes */
  	int       key_flags;      /* Various flags */
  	int       has_essid;
  	int       essid_on;
  	char      essid[IW_ESSID_MAX_SIZE + 1];   /* ESSID */
  	int       has_nickname;
  	char      nickname[IW_ESSID_MAX_SIZE + 1]; /* NickName */
  	int       has_ap_addr;
  	sockaddr  ap_addr;        /* Access point address */
  	int       has_bitrate;
  	iwparam   bitrate;        /* Bit rate in bps */
  	int       has_rts;
  	iwparam   rts;            /* RTS threshold in bytes */
  	int       has_frag;
  	iwparam   frag;           /* Fragmentation threshold in bytes */
  	int       has_mode;
  	int       mode;           /* Operation mode */
  	int       has_power;
  	iwparam   power;          /* Power management parameters */
#if 0	
  	/* Stats */
  	iwstats   stats;
  	int       has_stats;
  	iwrange   range;
  	int       has_range;
#endif
} InterfaceInfo;

InterfaceInfo ExtendedIWInfo;

int 
get_extented_iw_info(InterfaceInfo *indata)
{
  InterfaceInfo *wdata = indata;
  struct iwreq   request;
  int            netsock_fd = -1;


  strcpy( wdata->name, Mwd.iface );
  
  /* open socket */

  netsock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	
  if (netsock_fd == -1) return -1;

  /* network name */

  strcpy ( request.ifr_name, wdata->name);
  request.u.essid.pointer = (caddr_t) wdata->essid;
  request.u.essid.length  = IW_ESSID_MAX_SIZE + 1;
  request.u.essid.flags   = 0;
  
  if ( ioctl (netsock_fd, SIOCGIWESSID, &request) >= 0)
    {
      wdata->has_essid = 1;
      wdata->essid_on = request.u.data.flags;
    }
  else wdata->has_essid = 0;

#if 0 	    /* Stuff possibly breaks on some cards  */

  /* station name */

  strcpy ( request.ifr_name, wdata->name);
  request.u.essid.pointer = (caddr_t) wdata->nickname;
  request.u.essid.length=0;
  request.u.essid.flags=0;
	
  if ( ioctl (netsock_fd, SIOCGIWNICKN, &request) < 0)
    {
      wdata->nickname[0] = '\0';
    }

  /* mode */

  strcpy ( request.ifr_name, wdata->name);
  
  if ( ioctl (netsock_fd, SIOCGIWMODE, &request) >= 0)
    {
      if ((request.u.mode < 6) && (request.u.mode >= 0))
	wdata->mode = request.u.mode;
    }

  /* enc settings (on/off and key) */
	
  wdata->has_key = 0;
  strcpy ( request.ifr_name, wdata->name);
  request.u.data.pointer = (caddr_t) wdata->key;
  request.u.data.length=0;
  request.u.data.flags=0;

  if ( ioctl (netsock_fd, SIOCGIWENCODE, &request) >= 0)
    {
      wdata->has_key = 1;
      wdata->key_size = request.u.data.length;
      wdata->key_flags = request.u.data.flags;
    }

  /* power savings */
	
  strcpy(request.ifr_name, wdata->name);
  request.u.power.flags = 0;
  
  if(ioctl(netsock_fd, SIOCGIWPOWER, &request) >= 0)
    {
      wdata->has_power = 1;
      memcpy(&(wdata->power), &(request.u.power), sizeof(iwparam));
    }

  /* access point mac address */
	
  strcpy(request.ifr_name, wdata->name);
  
  if(ioctl(netsock_fd, SIOCGIWAP, &request) >= 0)
    {
      wdata->has_ap_addr = 1;
      memcpy(&(wdata->ap_addr), &(request.u.ap_addr), sizeof (sockaddr));
    }

#endif

  /* pack it up and spit it out */

  close (netsock_fd);

  return 0;
}

#endif

Bool
update_wireless(void) {
  FILE *wireless;   // File handle for /proc/net/wireless
  int count;					      
  char line[255];

  Mwd.link = 0;
  Mwd.level = 0;
  Mwd.noise = 0;
  Mwd.nwid = 0;
  Mwd.crypt = 0;
  Mwd.misc = 0;

  if ((wireless = fopen ("/proc/net/wireless", "r")) != NULL)
  {
     fgets(line,sizeof(line),wireless);
     fgets(line,sizeof(line),wireless);
     if (fgets(line,sizeof(line),wireless) == NULL) {
	Mwd.mode = 0;
	Mwd.iface[0]=0;
     } else {
	sscanf(line,"%s %s %f %f %f %d %d %d",
	       Mwd.iface, Mwd.status, &Mwd.link, &Mwd.level,
	       &Mwd.noise, &Mwd.nwid, &Mwd.crypt, &Mwd.misc);
	for(count=0;(count<strlen(line)) && (line[count]==0x20);count++);
        strncpy(Mwd.iface,&line[count],5);
        Mwd.iface[4]=0;
	Mwd.mode = 1;
     }
     fclose(wireless);
  }
  else
  {
     fprintf (stderr, "miniwave: Wirless device /proc/net/wireless not found\nEnable radio networking and recompile your kernel\n");
     return False;
  }
  return True;
}

void
paint_callback (MBTrayApp *app, Drawable drw )
{

  MBPixbufImage *img_backing = NULL;

  if (update_wireless())
    {
      if (Mwd.mode)
	{
	  /* res->percent = (int)rint ((log (link) / log (92)) * 100.0); ? */
	  if (Mwd.link > 0 && Mwd.link < 41)
	    CurImg = MW_SIG_1_40;
	  else if (Mwd.link > 40 && Mwd.link < 61)
	    CurImg = MW_SIG_41_60;
	  else if (Mwd.link > 60 && Mwd.link < 81)
	    CurImg = MW_SIG_61_80;
	  else if (Mwd.link > 80)
	    CurImg = MW_SIG_80_100;
	  else
	    CurImg = MW_NO_LINK;
	}
      else CurImg = MW_NO_LINK;
    } 
  else CurImg = MW_BROKE;

  if (LastImg == CurImg) return;
  
  img_backing = mb_tray_app_get_background (app, pb);

  mb_pixbuf_img_copy_composite(pb, img_backing, 
			       ImgsScaled[CurImg], 0, 0,
			       mb_pixbuf_img_get_width(ImgsScaled[0]),
			       mb_pixbuf_img_get_height(ImgsScaled[0]),
			       mb_tray_app_tray_is_vertical(app) ? 
			       (mb_pixbuf_img_get_width(img_backing)-mb_pixbuf_img_get_width(ImgsScaled[0]))/2 : 0,
			       mb_tray_app_tray_is_vertical(app) ? 0 : 
			       (mb_pixbuf_img_get_height(img_backing)-mb_pixbuf_img_get_height(ImgsScaled[0]))/2 );

  mb_pixbuf_img_render_to_drawable(pb, img_backing, drw, 0, 0);

  mb_pixbuf_img_free( pb, img_backing );

  LastImg = CurImg;
}


void
load_icons(MBTrayApp *app)
{
 int   i;
 char *icon_path;

  for (i=0; i<6; i++)
    {
      if (Imgs[i] != NULL) mb_pixbuf_img_free(pb, Imgs[i]);
      icon_path = mb_dot_desktop_icon_get_full_path (ThemeName, 
						     32, 
						     ImgLookup[i]);
      
      if (icon_path == NULL 
	  || !(Imgs[i] = mb_pixbuf_img_new_from_file(pb, icon_path)))
	{
	  fprintf(stderr, "minivol: failed to load icon\n" );
	  exit(1);
	}

      free(icon_path);
    }
  
}

void
resize_callback (MBTrayApp *app, int w, int h )
{
  int i;
  int base_width  = mb_pixbuf_img_get_width(Imgs[0]);
  int base_height = mb_pixbuf_img_get_height(Imgs[0]);
  int  scale_width = base_width, scale_height = base_height;
  Bool want_resize = True;

  if (mb_tray_app_tray_is_vertical(app) && w < base_width)
    {

      scale_width = w;
      scale_height = ( base_height * w ) / base_width;

      want_resize = False;
    }
  else if (!mb_tray_app_tray_is_vertical(app) && h < base_height)
    {
      scale_height = h;
      scale_width = ( base_width * h ) / base_height;
      want_resize = False;
    }

  if (w < base_width && h < base_height
      && ( scale_height > h || scale_width > w))
    {
       /* Something is really wrong to get here  */
      scale_height = h; scale_width = w;
      want_resize = False;
    }

  if (want_resize)  /* we only request a resize is absolutely needed */
    {
      LastImg = -1;
      mb_tray_app_request_size (app, scale_width, scale_height);
    }

  for (i=0; i<6; i++)
    {
      if (ImgsScaled[i] != NULL) mb_pixbuf_img_free(pb, ImgsScaled[i]);
      ImgsScaled[i] = mb_pixbuf_img_scale(pb, Imgs[i], 
					  scale_width, scale_height);
    }

}

void
button_callback (MBTrayApp *app, int x, int y, Bool is_released )
{
  char tray_msg[256];

  if (!is_released) return;

  if (Mwd.mode)
    {
      if (get_extented_iw_info(&ExtendedIWInfo) == 0)
	{
	  sprintf(tray_msg,
		  "%s:\n" 
		  "  Network: %s\n"
		  "  Link %.1f\n  Level %.1f\n  Noise %.1f\n",
		  Mwd.iface, 
		  ExtendedIWInfo.has_essid ? ExtendedIWInfo.essid  : "Unkown",
		  Mwd.link, Mwd.level, Mwd.noise );
	}
      else
	{
	  sprintf(tray_msg, _("%s:\n Link %.1f\n Level %.1f\n Noise %.1f\n"),
		  Mwd.iface, Mwd.link, Mwd.level, Mwd.noise );
	}
    }
  else
    sprintf(tray_msg, _("No wireless cards detected\n"));

  mb_tray_app_tray_send_message(app, tray_msg, 5000);

}

void 
theme_callback (MBTrayApp *app, char *theme_name)
{
  if (!theme_name) return;
  if (ThemeName) free(ThemeName);

  LastImg = -1; 			/* Make sure paint gets updated */

  ThemeName = strdup(theme_name);
  load_icons(app);
  resize_callback (app, mb_tray_app_width(app), mb_tray_app_width(app) );
}

void
timeout_callback ( MBTrayApp *app )
{
  mb_tray_app_repaint (app);
}

int
main( int argc, char *argv[])
{
  MBTrayApp *app = NULL;
  struct timeval tv;

#if ENABLE_NLS
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, DATADIR "/locale");
  bind_textdomain_codeset (PACKAGE, "UTF-8"); 
  textdomain (PACKAGE);
#endif

  memset(&ExtendedIWInfo, 0, sizeof(InterfaceInfo));

  app = mb_tray_app_new ( _("Wireless Monitor"),
			  resize_callback,
			  paint_callback,
			  &argc,
			  &argv );  
  
   pb = mb_pixbuf_new(mb_tray_app_xdisplay(app), 
		      mb_tray_app_xscreen(app));
   
   memset(&tv,0,sizeof(struct timeval));
   tv.tv_sec = 2;

   load_icons(app);

   mb_tray_app_set_timeout_callback (app, timeout_callback, &tv); 
   
   mb_tray_app_set_button_callback (app, button_callback );
   
   mb_tray_app_set_theme_change_callback (app, theme_callback );

   mb_tray_app_set_icon(app, pb, Imgs[3]);
   
   mb_tray_app_main (app);
   
   return 1;
}



