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

#include <sys/types.h>
#include <sys/ioctl.h>

#include <netdb.h>      /* gethostbyname, getnetbyname */
#if 0
#include <linux/if_arp.h>   /* For ARPHRD_ETHER */
#include <linux/socket.h>   /* For AF_INET & struct sockaddr */
#endif
#include <sys/socket.h>       /* For struct sockaddr_in */
#if 0
#include <linux/wireless.h>
#else
#include <iwlib.h>

#endif

#ifdef MB_HAVE_PNG
#define IMG_EXT "png"
#else
#define IMG_EXT "xpm"
#endif

/* also see http://www.snorp.net/files/patches/wireless-applet.c */

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
   char  *iface;
   char  status[3];
   struct wireless_info info;
} Mwd;

int skfd; /* file descriptor for socket */

Bool
update_wireless(void) 
{

  if (skfd == -1) {
    fprintf (stderr, "Unable to open a socket for wireless extensions.\nPlease ensure your kernel has wireless extensions support.");
    return False;
  }

  if (Mwd.iface == NULL)
      return False;

  if (iw_get_basic_config(skfd, Mwd.iface, &Mwd.info.b) < 0)
    {
      printf("iw_get_basic_config failed %s\n", Mwd.iface); 
      return False;
    }


  if(iw_get_range_info(skfd, Mwd.iface, &(Mwd.info.range)) >= 0)
    Mwd.info.has_range = 1;  

  if (iw_get_stats(skfd, Mwd.iface, &(Mwd.info.stats),
                   &(Mwd.info.range), Mwd.info.has_range) >= 0)
    Mwd.info.has_stats = 1;

  return True;
}

void
paint_callback (MBTrayApp *app, Drawable drw )
{

  MBPixbufImage *img_backing = NULL;

  if (update_wireless())
    {
      if (Mwd.info.has_range && (Mwd.info.stats.qual.level != 0))
	{
	  /* res->percent = (int)rint ((log (link) / log (92)) * 100.0); ? */
	  if (Mwd.info.stats.qual.qual > 0 && Mwd.info.stats.qual.qual < 41)
	    CurImg = MW_SIG_1_40;
	  else if (Mwd.info.stats.qual.qual > 40 && Mwd.info.stats.qual.qual < 61)
	    CurImg = MW_SIG_41_60;
	  else if (Mwd.info.stats.qual.qual > 60 && Mwd.info.stats.qual.qual < 81)
	    CurImg = MW_SIG_61_80;
	  else if (Mwd.info.stats.qual.qual > 80)
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

  update_wireless() ;

  /*if (Mwd.info.mode)*/
    {
      /*if (get_extented_iw_info(&Mwd.info) == 0)*/
	{
	  sprintf(tray_msg,
		  "%s:\n" 
		  "  Network: %s\n"
		  "  Link %.1f\n  Level %.1f\n  Noise %.1f\n",
		  Mwd.iface, 
		  Mwd.info.has_nickname ? Mwd.info.nickname  : "Unknown",
		  Mwd.info.stats.qual.qual, 
		  Mwd.info.stats.qual.level, 
		  Mwd.info.stats.qual.noise );
	}
      /*
      else

	{
	  sprintf(tray_msg, _("%s:\n Link %.1f\n Level %.1f\n Noise %.1f\n"),
		  Mwd.iface, 
		  Mwd.info.stats.qual.qual, 
		  Mwd.info.stats.qual.level, 
		  Mwd.info.stats.qual.noise );
	}
      */
    }
    /*
  else
    sprintf(tray_msg, _("No wireless cards detected\n"));
    */

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

int get_iface(int skfd, char *ifname, char *args[], int count)
{

 if (iw_get_basic_config(skfd, ifname, &Mwd.info.b) < 0)
   return;

  if (Mwd.iface != NULL)
    return 0;

  Mwd.iface = strdup(ifname);
  return 0;
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

  memset(&Mwd.info, 0, sizeof(struct wireless_info));
  skfd = iw_sockets_open();
  if (skfd != -1)
    {
      iw_enum_devices(skfd, get_iface, NULL, 0);
    }

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

   if (Mwd.iface != NULL)
      free(Mwd.iface);

   return 1;
}



