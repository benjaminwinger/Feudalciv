/********************************************************************** 
 Freeciv - Copyright (C) 1996 - A Kjeldberg, L Gregersen, P Unold
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
***********************************************************************/

/***************************************************************************
                          dialogs.c  -  description
                             -------------------
    begin                : Wed Jul 24 2002
    copyright            : (C) 2002 by Rafał Bursig
    email                : Rafał Bursig <bursig@poczta.fm>
***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <SDL/SDL.h>

/* utility */
#include "fcintl.h"
#include "log.h"
#include "rand.h"

/* common */
#include "combat.h"
#include "government.h"
#include "movement.h"
#include "unitlist.h"

/* client */
#include "civclient.h"
#include "climap.h" /* for client_tile_get_known() */
#include "clinet.h"
#include "goto.h"
#include "packhand.h"
#include "text.h"

/* gui-sdl */
#include "chatline.h"
#include "citydlg.h"
#include "cma_fe.h"
#include "colors.h"
#include "finddlg.h"
#include "gotodlg.h"
#include "graphics.h"
#include "gui_iconv.h"
#include "gui_id.h"
#include "gui_main.h"
#include "gui_stuff.h"
#include "gui_tilespec.h"
#include "gui_zoom.h"
#include "helpdlg.h"
#include "inteldlg.h"
#include "mapctrl.h"
#include "mapview.h"
#include "messagewin.h"
#include "optiondlg.h"
#include "plrdlg.h"
#include "repodlgs.h"
#include "themecolors.h"
#include "wldlg.h"

#include "dialogs.h"

struct player *races_player;

extern bool is_unit_move_blocked;
extern void popdown_diplomat_dialog(void);
extern void popdown_incite_dialog(void);
extern void popdown_bribe_dialog(void);

extern void popdown_caravan_dialog(void);

void popdown_advanced_terrain_dialog(void);
int advanced_terrain_window_dlg_callback(struct GUI *pWindow);
int exit_advanced_terrain_dlg_callback(struct GUI *pWidget);

static char *pLeaderName = NULL;

static void popdown_unit_select_dialog(void);
static void popdown_terrain_info_dialog(void);
static void popdown_pillage_dialog(void);
static void popdown_connect_dialog(void);
static void popdown_revolution_dialog(void);
static void popdown_unit_upgrade_dlg(void);

/********************************************************************** 
  ...
***********************************************************************/
void put_window_near_map_tile(struct GUI *pWindow,
  		int window_width, int window_height, struct tile *ptile)
{
  int canvas_x, canvas_y;
  
  if (tile_to_canvas_pos(&canvas_x, &canvas_y, ptile)) {
    if (canvas_x + tileset_tile_width(tileset) + window_width >= pWindow->dst->w)
    {
      if (canvas_x - window_width < 0) {
	pWindow->size.x = (pWindow->dst->w - window_width) / 2;
      } else {
	pWindow->size.x = canvas_x - window_width;
      }
    } else {
      pWindow->size.x = canvas_x + tileset_tile_width(tileset);
    }
    
    canvas_y += (tileset_tile_height(tileset) - window_height) / 2;
    if (canvas_y + window_height >= pWindow->dst->h)
    {
      pWindow->size.y = pWindow->dst->h - window_height - 1;
    } else {
      if (canvas_y < 0)
      {
	pWindow->size.y = 0;
      } else {
        pWindow->size.y = canvas_y;
      }
    }
  } else {
    pWindow->size.x = (pWindow->dst->w - window_width) / 2;
    pWindow->size.y = (pWindow->dst->h - window_height) / 2;
  }
  
}


/********************************************************************** 
  This function is called when the client disconnects or the game is
  over.  It should close all dialog windows for that game.
***********************************************************************/
void popdown_all_game_dialogs(void)
{
  popdown_caravan_dialog();  
  popdown_unit_select_dialog();
  popdown_advanced_terrain_dialog();
  popdown_terrain_info_dialog();
  popdown_newcity_dialog();
  popdown_optiondlg();
  popdown_diplomat_dialog();
  popdown_pillage_dialog();
  popdown_incite_dialog();
  popdown_connect_dialog();
  popdown_bribe_dialog();
  popdown_find_dialog();
  popdown_revolution_dialog();
  popdown_all_science_dialogs();
  popdown_meswin_dialog();
  popdown_worklist_editor();
  popdown_economy_report_dialog();
  popdown_activeunits_report_dialog();
  popdown_intel_dialogs();
  popdown_players_nations_dialog();
  popdown_players_dialog();
  popdown_goto_airlift_dialog();
  popdown_unit_upgrade_dlg();
  popdown_help_dialog();
  
  /* clear gui buffer */
  if (get_client_state() == CLIENT_PRE_GAME_STATE) {
    SDL_FillRect(Main.gui, NULL, 0x0);
  }
}

/* ======================================================================= */

/**************************************************************************
  Find the my unit's (focus) chance of success at attacking/defending the
  given enemy unit.  Return FALSE if the values cannot be determined (e.g., no
  units given).
**************************************************************************/
static bool sdl_get_chance_to_win(int *att_chance, int *def_chance,
		       		struct unit *enemy_unit, struct unit *my_unit)
{
  
  if (!my_unit || !enemy_unit) {
    return FALSE;
  }

  /* chance to win when active unit is attacking the selected unit */
  *att_chance = unit_win_chance(my_unit, enemy_unit) * 100;

  /* chance to win when selected unit is attacking the active unit */
  *def_chance = (1.0 - unit_win_chance(enemy_unit, my_unit)) * 100;

  return TRUE;
}

/**************************************************************************
  Popup a dialog to display information about an event that has a
  specific location.  The user should be given the option to goto that
  location.
**************************************************************************/
void popup_notify_goto_dialog(const char *headline, const char *lines,
			      struct tile *ptile)
{
  freelog(LOG_NORMAL, "popup_notify_goto_dialog : PORT ME\n \
  			a: %s\nb: %s",headline, lines );
}

/* ----------------------------------------------------------------------- */
struct ADVANCED_DLG *pNotifyDlg = NULL;

/**************************************************************************
...
**************************************************************************/
static int notify_dialog_window_callback(struct GUI *pWindow)
{
  return std_move_window_group_callback(pNotifyDlg->pBeginWidgetList,
  								pWindow);
}

/**************************************************************************
...
**************************************************************************/
static int exit_notify_dialog_callback(struct GUI *pWidget)
{
  if(pNotifyDlg) {
    popdown_window_group_dialog(pNotifyDlg->pBeginWidgetList,
					    pNotifyDlg->pEndWidgetList);
    FC_FREE(pNotifyDlg->pScroll);
    FC_FREE(pNotifyDlg);
    flush_dirty();
  }
  return -1;
}

/**************************************************************************
  Popup a generic dialog to display some generic information.
**************************************************************************/
void popup_notify_dialog(const char *caption, const char *headline,
			 const char *lines)
{
  struct GUI *pBuf, *pWindow;
  SDL_String16 *pStr;
  SDL_Surface *pHeadline, *pLines;
  SDL_Rect dst;
  int w = 0, h;
  
  if (pNotifyDlg) {
    return;
  }
  
  pNotifyDlg = fc_calloc(1, sizeof(struct ADVANCED_DLG));
   
  pStr = create_str16_from_char(caption, adj_font(12));
  pStr->style |= TTF_STYLE_BOLD;
  
  pWindow = create_window(NULL, pStr, adj_size(10), adj_size(10), 0);
  
  pWindow->action = notify_dialog_window_callback;
  set_wstate(pWindow, FC_WS_NORMAL);
  w = MAX(w, pWindow->size.w);
  
  add_to_gui_list(ID_WINDOW, pWindow);
  pNotifyDlg->pEndWidgetList = pWindow;
  
  /* ---------- */
  /* create exit button */
  pBuf = create_themeicon(pTheme->Small_CANCEL_Icon, pWindow->dst,
  			  			WF_DRAW_THEME_TRANSPARENT);
  pBuf->action = exit_notify_dialog_callback;
  set_wstate(pBuf, FC_WS_NORMAL);
  pBuf->key = SDLK_ESCAPE;
  w += (pBuf->size.w + adj_size(10));
  
  add_to_gui_list(ID_BUTTON, pBuf);
  pNotifyDlg->pBeginWidgetList = pBuf;
    
  pStr = create_str16_from_char(headline, adj_font(16));
  pStr->style |= TTF_STYLE_BOLD;
  
  pHeadline = create_text_surf_from_str16(pStr);
    
  if(lines && *lines != '\0') {
    change_ptsize16(pStr, adj_font(12));
    pStr->style &= ~TTF_STYLE_BOLD;
    copy_chars_to_string16(pStr, lines);
    pLines = create_text_surf_from_str16(pStr);
  } else {
    pLines = NULL;
  }
  
  FREESTRING16(pStr);
  
  w = MAX(w, pHeadline->w);
  if(pLines) {
    w = MAX(w, pLines->w);
  }
  w += adj_size(60);
  h = WINDOW_TILE_HIGH + 1 + FRAME_WH + adj_size(10) + pHeadline->h + adj_size(10);
  if(pLines) {
    h += pLines->h + adj_size(10);
  }
  pWindow->size.x = (Main.screen->w - w) / 2;
  pWindow->size.y = (Main.screen->h - h) / 2;
  
  resize_window(pWindow, NULL,
  	get_game_colorRGB(COLOR_THEME_BACKGROUND_BROWN), w, h);
	
  dst.x = (pWindow->size.w - pHeadline->w) / 2;
  dst.y = WINDOW_TILE_HIGH + adj_size(11);
  
  SDL_BlitSurface(pHeadline, NULL, pWindow->theme, &dst);
  if(pLines) {
    dst.y += pHeadline->h + adj_size(10);
    if(pHeadline->w < pLines->w) {
      dst.x = (pWindow->size.w - pLines->w) / 2;
    }
     
    SDL_BlitSurface(pLines, NULL, pWindow->theme, &dst);
  }
  
  FREESURFACE(pHeadline);
  FREESURFACE(pLines);
  
  /* exit button */
  pBuf = pWindow->prev; 
  pBuf->size.x = pWindow->size.x + pWindow->size.w - pBuf->size.w - FRAME_WH - 1;
  pBuf->size.y = pWindow->size.y + 1 + (WINDOW_TILE_HIGH - pBuf->size.h) / 2;
    
  /* redraw */
  redraw_group(pNotifyDlg->pBeginWidgetList, pWindow, 0);
  flush_rect(pWindow->size, FALSE);
}

/* =======================================================================*/
/* ======================== UNIT UPGRADE DIALOG ========================*/
/* =======================================================================*/
static struct SMALL_DLG *pUnit_Upgrade_Dlg = NULL;

/****************************************************************
...
*****************************************************************/
static int upgrade_unit_window_callback(struct GUI *pWindow)
{
  return std_move_window_group_callback(
  			pUnit_Upgrade_Dlg->pBeginWidgetList, pWindow);
}

/****************************************************************
...
*****************************************************************/
static int cancel_upgrade_unit_callback(struct GUI *pWidget)
{
  popdown_unit_upgrade_dlg();
  /* enable city dlg */
  enable_city_dlg_widgets();
  flush_dirty();
  return -1;
}

/****************************************************************
...
*****************************************************************/
static int ok_upgrade_unit_window_callback(struct GUI *pWidget)
{
  struct unit *pUnit = pWidget->data.unit;
  popdown_unit_upgrade_dlg();
  /* enable city dlg */
  enable_city_dlg_widgets();
  free_city_units_lists();
  request_unit_upgrade(pUnit);
  flush_dirty();
  return -1;
}

/****************************************************************
...
*****************************************************************/
void popup_unit_upgrade_dlg(struct unit *pUnit, bool city)
{
  struct unit_type *ut1, *ut2;
  int value = 9999, hh, ww = 0;
  char cBuf[128];
  struct GUI *pBuf = NULL, *pWindow;
  SDL_String16 *pStr;
  SDL_Surface *pText;
  SDL_Rect dst;
  
  ut1 = pUnit->type;
  
  if (pUnit_Upgrade_Dlg) {
    /* just in case */
    flush_dirty();
    return;
  }
  CHECK_UNIT_TYPE(ut1);
    
  pUnit_Upgrade_Dlg = fc_calloc(1, sizeof(struct SMALL_DLG));

  ut2 = can_upgrade_unittype(game.player_ptr, ut1);
  
  if (ut2) {
    value = unit_upgrade_price(game.player_ptr, ut1, ut2);
  
    if (game.player_ptr->economic.gold >= value) {
      my_snprintf(cBuf, sizeof(cBuf),
    	      _("Upgrade %s to %s for %d gold?\n"
                "Treasury contains %d gold."),
	  get_unit_type(ut1->index)->name, get_unit_type(ut2->index)->name,
	  value, game.player_ptr->economic.gold);
    } else {
      my_snprintf(cBuf, sizeof(cBuf),
          _("Upgrading %s to %s costs %d gold.\n"
            "Treasury contains %d gold."),
          get_unit_type(ut1->index)->name, get_unit_type(ut2->index)->name,
          value, game.player_ptr->economic.gold);
    }
  } else {
    my_snprintf(cBuf, sizeof(cBuf),
        _("Sorry: cannot upgrade %s."), get_unit_type(ut1->index)->name);
  }
  
  hh = WINDOW_TILE_HIGH + 1;
  pStr = create_str16_from_char(_("Upgrade Obsolete Units"), adj_font(12));
  pStr->style |= TTF_STYLE_BOLD;

  pWindow = create_window(NULL, pStr, adj_size(100), adj_size(100), 0);

  pWindow->action = upgrade_unit_window_callback;
  set_wstate(pWindow, FC_WS_NORMAL);

  pUnit_Upgrade_Dlg->pEndWidgetList = pWindow;

  add_to_gui_list(ID_WINDOW, pWindow);

  /* ============================================================= */
  
  /* create text label */
  pStr = create_str16_from_char(cBuf, adj_font(10));
  pStr->style |= (TTF_STYLE_BOLD|SF_CENTER);
  pStr->fgcol.r = 255;
  pStr->fgcol.g = 255;
  /*pStr->forecol.b = 255; */
  
  pText = create_text_surf_from_str16(pStr);
  FREESTRING16(pStr);
  
  hh += (pText->h + adj_size(10));
  ww = MAX(ww, pText->w + adj_size(20));
  
  /* cancel button */
  pBuf = create_themeicon_button_from_chars(pTheme->CANCEL_Icon,
			    pWindow->dst, _("Cancel"), adj_font(12), 0);

  clear_wflag(pBuf, WF_DRAW_FRAME_AROUND_WIDGET);
  pBuf->action = cancel_upgrade_unit_callback;
  set_wstate(pBuf, FC_WS_NORMAL);

  hh += (pBuf->size.h + adj_size(20));
  
  add_to_gui_list(ID_BUTTON, pBuf);
  
  if ((ut2) && (game.player_ptr->economic.gold >= value)) {
    pBuf = create_themeicon_button_from_chars(pTheme->OK_Icon, pWindow->dst,
					      _("Upgrade"), adj_font(12), 0);
        
    clear_wflag(pBuf, WF_DRAW_FRAME_AROUND_WIDGET);
    pBuf->action = ok_upgrade_unit_window_callback;
    set_wstate(pBuf, FC_WS_NORMAL);
    pBuf->data.unit = pUnit;    
    add_to_gui_list(ID_BUTTON, pBuf);
    pBuf->size.w = MAX(pBuf->size.w, pBuf->next->size.w);
    pBuf->next->size.w = pBuf->size.w;
    ww = MAX(ww, adj_size(30) + pBuf->size.w * 2);
  } else {
    ww = MAX(ww, pBuf->size.w + adj_size(20));
  }
  /* ============================================ */
  
  pUnit_Upgrade_Dlg->pBeginWidgetList = pBuf;
  if(city) {
    pWindow->size.x = Main.event.motion.x;
    pWindow->size.y = Main.event.motion.y;
  } else {
    put_window_near_map_tile(pWindow,
  		ww + DOUBLE_FRAME_WH, hh, pUnit->tile);
  }
    
  resize_window(pWindow, NULL,
		get_game_colorRGB(COLOR_THEME_BACKGROUND_BROWN),
		ww + DOUBLE_FRAME_WH, hh);
  
  /* setup rest of widgets */
  /* label */
  dst.x = FRAME_WH + (ww - DOUBLE_FRAME_WH - pText->w) / 2;
  dst.y = WINDOW_TILE_HIGH + adj_size(11);
  SDL_BlitSurface(pText, NULL, pWindow->theme, &dst);
  FREESURFACE(pText);
   
  /* cancel button */
  pBuf = pWindow->prev;
  pBuf->size.y = pWindow->size.y + pWindow->size.h - pBuf->size.h - adj_size(10);
  
  if ((ut2) && (game.player_ptr->economic.gold >= value)) {
    /* sell button */
    pBuf = pBuf->prev;
    pBuf->size.x = pWindow->size.x + (ww - (2 * pBuf->size.w + adj_size(10))) / 2;
    pBuf->size.y = pBuf->next->size.y;
    
    /* cancel button */
    pBuf->next->size.x = pBuf->size.x + pBuf->size.w + adj_size(10);
  } else {
    /* x position of cancel button */
    pBuf->size.x = pWindow->size.x +
			    pWindow->size.w - FRAME_WH - pBuf->size.w - adj_size(10);
  }
  
  
  /* ================================================== */
  /* redraw */
  redraw_group(pUnit_Upgrade_Dlg->pBeginWidgetList, pWindow, 0);
    
  sdl_dirty_rect(pWindow->size);
  flush_dirty();
  
}

/****************************************************************
...
*****************************************************************/
static void popdown_unit_upgrade_dlg(void)
{
  if (pUnit_Upgrade_Dlg) {
    popdown_window_group_dialog(pUnit_Upgrade_Dlg->pBeginWidgetList,
			      pUnit_Upgrade_Dlg->pEndWidgetList);
    FC_FREE(pUnit_Upgrade_Dlg);
  }
}

/* =======================================================================*/
/* ======================== UNIT SELECTION DIALOG ========================*/
/* =======================================================================*/
static struct ADVANCED_DLG *pUnit_Select_Dlg = NULL;

/**************************************************************************
...
**************************************************************************/
static int unit_select_window_callback(struct GUI *pWindow)
{
  return std_move_window_group_callback(pUnit_Select_Dlg->pBeginWidgetList,
  								pWindow);
}

/**************************************************************************
...
**************************************************************************/
static int exit_unit_select_callback( struct GUI *pWidget )
{
  popdown_unit_select_dialog();
  is_unit_move_blocked = FALSE;
  return -1;
}

/**************************************************************************
...
**************************************************************************/
static int unit_select_callback( struct GUI *pWidget )
{
  struct unit *pUnit = player_find_unit_by_id(game.player_ptr,
                                   MAX_ID - pWidget->ID);

  popdown_unit_select_dialog();
  if (pUnit) {
    request_new_unit_activity(pUnit, ACTIVITY_IDLE);
    set_unit_focus(pUnit);
  }

  return -1;
}

/**************************************************************************
  Popdown a dialog window to select units on a particular tile.
**************************************************************************/
static void popdown_unit_select_dialog(void)
{
  if (pUnit_Select_Dlg) {
    is_unit_move_blocked = FALSE;
    popdown_window_group_dialog(pUnit_Select_Dlg->pBeginWidgetList,
			pUnit_Select_Dlg->pEndWidgetList);
				   
    FC_FREE(pUnit_Select_Dlg->pScroll);
    FC_FREE(pUnit_Select_Dlg);
    flush_dirty();
  }
}

/**************************************************************************
  Popup a dialog window to select units on a particular tile.
**************************************************************************/
void popup_unit_select_dialog(struct tile *ptile)
{
  struct GUI *pBuf = NULL, *pWindow;
  SDL_String16 *pStr;
  struct unit *pUnit = NULL, *pFocus = unit_list_get(get_units_in_focus(), 0);
  struct unit_type *pUnitType;
  char cBuf[255];  
  int i, w = 0, h, n;
  
  #define NUM_SEEN	20
  
  n = unit_list_size(ptile->units);
  
  if (!n || pUnit_Select_Dlg) {
    return;
  }
  
  is_unit_move_blocked = TRUE;  
  pUnit_Select_Dlg = fc_calloc(1, sizeof(struct ADVANCED_DLG));
    
  my_snprintf(cBuf , sizeof(cBuf),"%s (%d)", _("Unit selection") , n);
  pStr = create_str16_from_char(cBuf , adj_font(12));
  pStr->style |= TTF_STYLE_BOLD;
  
  pWindow = create_window(NULL, pStr, adj_size(10), adj_size(10), WF_DRAW_THEME_TRANSPARENT);
  
  pWindow->action = unit_select_window_callback;
  set_wstate(pWindow, FC_WS_NORMAL);
  w = MAX(w, pWindow->size.w);
  
  add_to_gui_list(ID_UNIT_SELLECT_DLG_WINDOW, pWindow);
  pUnit_Select_Dlg->pEndWidgetList = pWindow;
  /* ---------- */
  /* create exit button */
  pBuf = create_themeicon(pTheme->Small_CANCEL_Icon, pWindow->dst,
  			  			WF_DRAW_THEME_TRANSPARENT);
  pBuf->action = exit_unit_select_callback;
  set_wstate(pBuf, FC_WS_NORMAL);
  pBuf->key = SDLK_ESCAPE;
  w += (pBuf->size.w + adj_size(10));
  
  add_to_gui_list(ID_UNIT_SELLECT_DLG_EXIT_BUTTON, pBuf);
    
  /* ---------- */
  h = WINDOW_TILE_HIGH + 1 + FRAME_WH;
  
  for(i = 0; i < n; i++) {
    pUnit = unit_list_get(ptile->units, i);
    pUnitType = unit_type(pUnit);
        
    if(pUnit->owner == game.player_ptr) {
      my_snprintf(cBuf , sizeof(cBuf), _("Contact %s (%d / %d) %s(%d,%d,%d) %s"),
            pUnit->veteran ? _("Veteran") : "" ,
            pUnit->hp, pUnitType->hp,
            pUnitType->name,
            pUnitType->attack_strength,
            pUnitType->defense_strength,
            (pUnitType->move_rate / SINGLE_MOVE),
	    unit_activity_text(pUnit));
    } else {
      int att_chance, def_chance;
      
      my_snprintf(cBuf , sizeof(cBuf), _("%s %s %s(A:%d D:%d M:%d FP:%d) HP:%d%%"),
            get_nation_by_plr(unit_owner(pUnit))->name,
            (pUnit->veteran ? _("Veteran") : ""),
            pUnitType->name,
            pUnitType->attack_strength,
            pUnitType->defense_strength,
            (pUnitType->move_rate / SINGLE_MOVE),
      	    pUnitType->firepower,
	    (pUnit->hp * 100 / pUnitType->hp + 9) / 10);
      
      /* calculate chance to win */
      if (sdl_get_chance_to_win(&att_chance, &def_chance, pUnit, pFocus)) {
          cat_snprintf(cBuf, sizeof(cBuf), _(" CtW: Att:%d%% Def:%d%%"),
               att_chance, def_chance);
      }
    }
    
    create_active_iconlabel(pBuf, pWindow->dst,
    		pStr, cBuf, unit_select_callback);
            
    add_to_gui_list(MAX_ID - pUnit->id , pBuf);
    
    w = MAX(w, pBuf->size.w);
    h += pBuf->size.h;
    if(pUnit->owner == game.player_ptr) {
      set_wstate(pBuf, FC_WS_NORMAL);
    }
    
    if (i > NUM_SEEN - 1)
    {
      set_wflag(pBuf , WF_HIDDEN);
    }
    
  }
  pUnit_Select_Dlg->pBeginWidgetList = pBuf;
  pUnit_Select_Dlg->pBeginActiveWidgetList = pBuf;
  pUnit_Select_Dlg->pEndActiveWidgetList = pWindow->prev->prev;
  pUnit_Select_Dlg->pActiveWidgetList = pWindow->prev->prev;
  
  w += (DOUBLE_FRAME_WH + 2);
  if (n > NUM_SEEN)
  {
    n = create_vertical_scrollbar(pUnit_Select_Dlg, 1, NUM_SEEN, TRUE, TRUE);
    w += n;
    
    /* ------- window ------- */
    h = WINDOW_TILE_HIGH + 1 +
	    NUM_SEEN * pWindow->prev->prev->size.h + FRAME_WH;
  }
  
  put_window_near_map_tile(pWindow, w, h, pUnit->tile);
  resize_window(pWindow, NULL, NULL, w, h);
    
  if(pUnit_Select_Dlg->pScroll) {
    w -= n;
  }

  w -= (DOUBLE_FRAME_WH + 2);
  
  /* exit button */
  pBuf = pWindow->prev; 
  pBuf->size.x = pWindow->size.x + pWindow->size.w - pBuf->size.w - FRAME_WH - 1;
  pBuf->size.y = pWindow->size.y + 1;
  pBuf = pBuf->prev;
  
  setup_vertical_widgets_position(1, pWindow->size.x + FRAME_WH + 1,
		  pWindow->size.y + WINDOW_TILE_HIGH + 1, w, 0,
		  pUnit_Select_Dlg->pBeginActiveWidgetList, pBuf);
    
  if(pUnit_Select_Dlg->pScroll) {
    setup_vertical_scrollbar_area(pUnit_Select_Dlg->pScroll,
	pWindow->size.x + pWindow->size.w - FRAME_WH,
    	pWindow->size.y + WINDOW_TILE_HIGH + 1,
    	pWindow->size.h - (FRAME_WH + WINDOW_TILE_HIGH + 1), TRUE);
  }
  
  /* ==================================================== */
  /* redraw */
  redraw_group(pUnit_Select_Dlg->pBeginWidgetList, pWindow, 0);

  flush_rect(pWindow->size, FALSE);
}

/* ====================================================================== */
/* ============================ TERRAIN INFO ============================ */
/* ====================================================================== */
static struct SMALL_DLG *pTerrain_Info_Dlg = NULL;


/**************************************************************************
  Popdown terrain information dialog.
**************************************************************************/
static int terrain_info_window_dlg_callback(struct GUI *pWindow)
{
  return std_move_window_group_callback(pTerrain_Info_Dlg->pBeginWidgetList,
  								pWindow);
}

/**************************************************************************
  Popdown terrain information dialog.
**************************************************************************/
static void popdown_terrain_info_dialog(void)
{
  if (pTerrain_Info_Dlg) {
    popdown_window_group_dialog(pTerrain_Info_Dlg->pBeginWidgetList,
				pTerrain_Info_Dlg->pEndWidgetList);
    FC_FREE(pTerrain_Info_Dlg);
    flush_dirty();
  }
}

/**************************************************************************
  Popdown terrain information dialog.
**************************************************************************/
static int exit_terrain_info_dialog(struct GUI *pButton)
{
  popdown_terrain_info_dialog();
  return -1;
}

/***************************************************************
  Return a (static) string with terrain defense bonus;
***************************************************************/
const char *sdl_get_tile_defense_info_text(struct tile *ptile)
{
  static char buffer[64];
  int bonus = (ptile->terrain->defense_bonus - 10) * 10;    
  
  if(tile_has_special(ptile, S_RIVER)) {
    bonus += terrain_control.river_defense_bonus;
  }
  if(tile_has_special(ptile, S_FORTRESS)) {
    bonus += terrain_control.fortress_defense_bonus;
  }
  my_snprintf(buffer, sizeof(buffer), "Terrain Defense Bonus: +%d%% ", bonus);
  
  return buffer;
}

/**************************************************************************
  Popup terrain information dialog.
**************************************************************************/
static void popup_terrain_info_dialog(SDL_Surface *pDest, struct tile *ptile)
{
  SDL_Surface *pSurf;
  struct GUI *pBuf, *pWindow;
  SDL_String16 *pStr;  
  char cBuf[256];  

  if (pTerrain_Info_Dlg) {
    flush_dirty();
    return;
  }
      
  pSurf = get_terrain_surface(ptile);
  pTerrain_Info_Dlg = fc_calloc(1, sizeof(struct SMALL_DLG));
    
  /* ----------- */  
  my_snprintf(cBuf, sizeof(cBuf), "%s [%d,%d]", _("Terrain Info"), ptile->x , ptile->y);
  
  pWindow = create_window(pDest, create_str16_from_char(cBuf , adj_font(12)), adj_size(10), adj_size(10), 0);
  pWindow->string16->style |= TTF_STYLE_BOLD;
  
  pWindow->action = terrain_info_window_dlg_callback;
  set_wstate(pWindow, FC_WS_NORMAL);
  
  add_to_gui_list(ID_TERRAIN_INFO_DLG_WINDOW, pWindow);
  pTerrain_Info_Dlg->pEndWidgetList = pWindow;
  
  /* ---------- */
  pStr = create_str16_from_char(popup_info_text(ptile), adj_font(12));
  pStr->style |= SF_CENTER;
  pBuf = create_iconlabel(pSurf, pWindow->dst, pStr, 0);
  
  pBuf->size.h += tileset_tile_height(tileset) / 2;
  
  add_to_gui_list(ID_LABEL, pBuf);
  
  /* ------ window ---------- */
  pWindow->size.w = pBuf->size.w + adj_size(20);
  pWindow->size.h = pBuf->size.h + WINDOW_TILE_HIGH + 1 + FRAME_WH;

  put_window_near_map_tile(pWindow, pWindow->size.w, pWindow->size.h, ptile);
  resize_window(pWindow, NULL,
	  get_game_colorRGB(COLOR_THEME_BACKGROUND_BROWN),
				  pWindow->size.w, pWindow->size.h);
  
  /* ------------------------ */
  
  pBuf->size.x = pWindow->size.x + adj_size(10);
  pBuf->size.y = pWindow->size.y + WINDOW_TILE_HIGH + 1;
  
  pBuf = create_themeicon(pTheme->Small_CANCEL_Icon, pWindow->dst,
  			  			WF_DRAW_THEME_TRANSPARENT);
  pBuf->size.x = pWindow->size.x + pWindow->size.w - pBuf->size.w - FRAME_WH-1;
  pBuf->size.y = pWindow->size.y + 1;
  pBuf->action = exit_terrain_info_dialog;
  set_wstate(pBuf, FC_WS_NORMAL);
  pBuf->key = SDLK_ESCAPE;
  
  add_to_gui_list(ID_TERRAIN_INFO_DLG_EXIT_BUTTON, pBuf);
    
  pTerrain_Info_Dlg->pBeginWidgetList = pBuf;
  /* --------------------------------- */
  /* redraw */
  redraw_group(pTerrain_Info_Dlg->pBeginWidgetList, pWindow, 0);
  sdl_dirty_rect(pWindow->size);
  flush_dirty();
}
/* ====================================================================== */
/* ========================= ADVANCED_TERRAIN_MENU ====================== */
/* ====================================================================== */
struct ADVANCED_DLG  *pAdvanced_Terrain_Dlg = NULL;

/**************************************************************************
  Popdown a generic dialog to display some generic information about
  terrain : tile, units , cities, etc.
**************************************************************************/
void popdown_advanced_terrain_dialog(void)
{
  if (pAdvanced_Terrain_Dlg) {
    is_unit_move_blocked = FALSE;
    popdown_window_group_dialog(pAdvanced_Terrain_Dlg->pBeginWidgetList,
			pAdvanced_Terrain_Dlg->pEndWidgetList);
				   
    FC_FREE(pAdvanced_Terrain_Dlg->pScroll);
    FC_FREE(pAdvanced_Terrain_Dlg);
  }
}

/**************************************************************************
  ...
**************************************************************************/
int advanced_terrain_window_dlg_callback(struct GUI *pWindow)
{
  return std_move_window_group_callback(pAdvanced_Terrain_Dlg->pBeginWidgetList,
  								pWindow);
}

/**************************************************************************
  ...
**************************************************************************/
int exit_advanced_terrain_dlg_callback(struct GUI *pWidget)
{
  popdown_advanced_terrain_dialog();
  flush_dirty();
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int terrain_info_callback(struct GUI *pWidget)
{
  int x = pWidget->data.cont->id0;
  int y = pWidget->data.cont->id1;
    
  lock_buffer(pWidget->dst);  
  popdown_advanced_terrain_dialog();

  popup_terrain_info_dialog(get_locked_buffer(), map_pos_to_tile(x , y));
  unlock_buffer();
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int zoom_to_city_callback(struct GUI *pWidget)
{
  struct city *pCity = pWidget->data.city;
  
  lock_buffer(pWidget->dst);
  popdown_advanced_terrain_dialog();

  popup_city_dialog(pCity);
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int change_production_callback(struct GUI *pWidget)
{
  struct city *pCity = pWidget->data.city;
  popdown_advanced_terrain_dialog();
  popup_worklist_editor(pCity, &(pCity->worklist));
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int hurry_production_callback(struct GUI *pWidget)
{
  struct city *pCity = pWidget->data.city;
  
  lock_buffer(pWidget->dst);  
  popdown_advanced_terrain_dialog();

  popup_hurry_production_dialog(pCity, get_locked_buffer());
  unlock_buffer();
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int cma_callback(struct GUI *pWidget)
{
  struct city *pCity = pWidget->data.city;
  popdown_advanced_terrain_dialog();
  popup_city_cma_dialog(pCity);
  return -1;
}

/**************************************************************************
...
**************************************************************************/
static int adv_unit_select_callback(struct GUI *pWidget)
{
  struct unit *pUnit = pWidget->data.unit;

  popdown_advanced_terrain_dialog();
  
  if (pUnit) {
    request_new_unit_activity(pUnit, ACTIVITY_IDLE);
    set_unit_focus(pUnit);
  }

  return -1;
}

/**************************************************************************
...
**************************************************************************/
static int adv_unit_select_all_callback(struct GUI *pWidget)
{
  struct unit *pUnit = pWidget->data.unit;

  popdown_advanced_terrain_dialog();
  
  if (pUnit) {
    activate_all_units(pUnit->tile);
  }
  return -1;
}

/**************************************************************************
...
**************************************************************************/
static int adv_unit_sentry_idle_callback(struct GUI *pWidget)
{
  struct unit *pUnit = pWidget->data.unit;

  popdown_advanced_terrain_dialog();
  
  if (pUnit) {
    struct tile *ptile = pUnit->tile;
    unit_list_iterate(ptile->units, punit) {
      if (game.player_ptr == punit->owner && (punit->activity == ACTIVITY_IDLE)
	 && !punit->ai.control && can_unit_do_activity(punit, ACTIVITY_SENTRY)) {
        request_new_unit_activity(punit, ACTIVITY_SENTRY);
      }
    } unit_list_iterate_end;
  }
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int goto_here_callback(struct GUI *pWidget)
{
  int x = pWidget->data.cont->id0;
  int y = pWidget->data.cont->id1;
    
  popdown_advanced_terrain_dialog();
  
  /* may not work */
  send_goto_tile(unit_list_get(get_units_in_focus(), 0), map_pos_to_tile(x, y));
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int patrol_here_callback(struct GUI *pWidget)
{
/* FIXME */
#if 0    
  int x = pWidget->data.cont->id0;
  int y = pWidget->data.cont->id1;
  struct unit *pUnit = unit_list_get(get_units_in_focus(), 0);
#endif
    
  popdown_advanced_terrain_dialog();
  
#if 0  
  if(pUnit) {
    enter_goto_state(pUnit);
    /* may not work */
    do_unit_patrol_to(pUnit, map_pos_to_tile(x, y));
    exit_goto_state();
  }
#endif  
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int paradrop_here_callback(struct GUI *pWidget)
{
/* FIXME */    
#if 0    
  int x = pWidget->data.cont->id0;
  int y = pWidget->data.cont->id1;
#endif    
    
  popdown_advanced_terrain_dialog();

#if 0    
  /* may not work */
  do_unit_paradrop_to(get_unit_in_focus(), map_pos_to_tile(x, y));
#endif    
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int unit_help_callback(struct GUI *pWidget)
{
  Unit_type_id unit_id = MAX_ID - pWidget->ID;
    
  popdown_advanced_terrain_dialog();
  popup_unit_info(unit_id);
  return -1;
}


/**************************************************************************
  Popup a generic dialog to display some generic information about
  terrain : tile, units , cities, etc.
**************************************************************************/
void popup_advanced_terrain_dialog(struct tile *ptile, Uint16 pos_x, Uint16 pos_y)
{
  struct GUI *pWindow = NULL, *pBuf = NULL;
  struct city *pCity;
  struct unit *pFocus_Unit;
  SDL_String16 *pStr;
  SDL_Rect area;
  struct CONTAINER *pCont;
  char cBuf[255]; 
  int n, w = 0, h, units_h = 0;
    
  if (pAdvanced_Terrain_Dlg) {
    return;
  }
  
  pCity = ptile->city;
  n = unit_list_size(ptile->units);
  pFocus_Unit = unit_list_get(get_units_in_focus(), 0);
  
  if (!n && !pCity && !pFocus_Unit)
  {
    popup_terrain_info_dialog(NULL, ptile);
    return;
  }
    
  h = WINDOW_TILE_HIGH + adj_size(3) + FRAME_WH;
  is_unit_move_blocked = TRUE;
    
  pAdvanced_Terrain_Dlg = fc_calloc(1, sizeof(struct ADVANCED_DLG));
  
  pCont = fc_calloc(1, sizeof(struct CONTAINER));
  pCont->id0 = ptile->x;
  pCont->id1 = ptile->y;
  
  pStr = create_str16_from_char(_("Advanced Menu") , adj_font(12));
  pStr->style |= TTF_STYLE_BOLD;
  
  pWindow = create_window(NULL, pStr, adj_size(10), adj_size(10), WF_DRAW_THEME_TRANSPARENT);
    
  pWindow->action = advanced_terrain_window_dlg_callback;
  set_wstate(pWindow , FC_WS_NORMAL);
  w = MAX(w , pWindow->size.w);
  
  add_to_gui_list(ID_TERRAIN_ADV_DLG_WINDOW, pWindow);
  pAdvanced_Terrain_Dlg->pEndWidgetList = pWindow;
  /* ---------- */
  /* exit button */
  pBuf = create_themeicon(pTheme->Small_CANCEL_Icon, pWindow->dst,
  			  			WF_DRAW_THEME_TRANSPARENT);
  
  w += pBuf->size.w + adj_size(10);
  pBuf->action = exit_advanced_terrain_dlg_callback;
  set_wstate(pBuf, FC_WS_NORMAL);
  pBuf->key = SDLK_ESCAPE;
  
  add_to_gui_list(ID_TERRAIN_ADV_DLG_EXIT_BUTTON, pBuf);
  /* ---------- */
  
  pStr = create_str16_from_char(_("Terrain Info") , adj_font(10));
  pStr->style |= TTF_STYLE_BOLD;
   
  pBuf = create_iconlabel(NULL, pWindow->dst, pStr , 
    (WF_DRAW_THEME_TRANSPARENT|WF_DRAW_TEXT_LABEL_WITH_SPACE|WF_FREE_DATA));

  pBuf->string16->render = 3;
  pBuf->string16->bgcol.unused = 128;
    
  pBuf->data.cont = pCont;
  
  pBuf->action = terrain_info_callback;
  set_wstate(pBuf, FC_WS_NORMAL);
  
  add_to_gui_list(ID_LABEL, pBuf);
    
  w = MAX(w, pBuf->size.w);
  h += pBuf->size.h;

  /* ---------- */  
  if (pCity && pCity->owner == game.player_ptr)
  {
    /* separator */
    pBuf = create_iconlabel(NULL, pWindow->dst, NULL, WF_FREE_THEME);
    
    add_to_gui_list(ID_SEPARATOR, pBuf);
    h += pBuf->next->size.h;
    /* ------------------ */
    
    my_snprintf(cBuf, sizeof(cBuf), _("Zoom to : %s"), pCity->name );
    
    create_active_iconlabel(pBuf, pWindow->dst,
		    pStr, cBuf, zoom_to_city_callback);
    pBuf->data.city = pCity;
    set_wstate(pBuf, FC_WS_NORMAL);
  
    add_to_gui_list(ID_LABEL, pBuf);
    
    w = MAX(w, pBuf->size.w);
    h += pBuf->size.h;
    /* ----------- */
    
    create_active_iconlabel(pBuf, pWindow->dst, pStr,
	    _("Change Production"), change_production_callback);
	    
    pBuf->data.city = pCity;
    set_wstate(pBuf, FC_WS_NORMAL);
  
    add_to_gui_list(ID_LABEL, pBuf);
    
    w = MAX(w, pBuf->size.w);
    h += pBuf->size.h;
    /* -------------- */
    
    create_active_iconlabel(pBuf, pWindow->dst, pStr,
	    _("Hurry production"), hurry_production_callback);
	    
    pBuf->data.city = pCity;
    set_wstate(pBuf, FC_WS_NORMAL);
  
    add_to_gui_list(ID_LABEL, pBuf);
    
    w = MAX(w, pBuf->size.w);
    h += pBuf->size.h;
    /* ----------- */
  
    create_active_iconlabel(pBuf, pWindow->dst, pStr,
	    _("Change C.M.A Settings"), cma_callback);
	    
    pBuf->data.city = pCity;
    set_wstate(pBuf, FC_WS_NORMAL);
  
    add_to_gui_list(ID_LABEL, pBuf);
    
    w = MAX(w, pBuf->size.w);
    h += pBuf->size.h;
    
  }
  /* ---------- */
  
  if(pFocus_Unit && (pFocus_Unit->tile->x != ptile->x || pFocus_Unit->tile->y != ptile->y)) {
    /* separator */
    pBuf = create_iconlabel(NULL, pWindow->dst, NULL, WF_FREE_THEME);
    
    add_to_gui_list(ID_SEPARATOR, pBuf);
    h += pBuf->next->size.h;
    /* ------------------ */
        
    create_active_iconlabel(pBuf, pWindow->dst, pStr, _("Goto here"),
						    goto_here_callback);
    pBuf->data.cont = pCont;
    set_wstate(pBuf, FC_WS_NORMAL);
        
    add_to_gui_list(MAX_ID - 1000 - pFocus_Unit->id, pBuf);
    
    w = MAX(w, pBuf->size.w);
    h += pBuf->size.h;
    /* ----------- */
    
    create_active_iconlabel(pBuf, pWindow->dst, pStr, _("Patrol here"),
						    patrol_here_callback);
    pBuf->data.cont = pCont;
    set_wstate(pBuf, FC_WS_NORMAL);
        
    add_to_gui_list(MAX_ID - 1000 - pFocus_Unit->id, pBuf);
    
    w = MAX(w, pBuf->size.w);
    h += pBuf->size.h;
    /* ----------- */

#if 0 /* FIXME: specific connect buttons */
    if(unit_flag(pFocus_Unit, F_SETTLERS)) {
      create_active_iconlabel(pBuf, pWindow->dst, pStr, _("Connect here"),
						    connect_here_callback);
      pBuf->data.cont = pCont;
      set_wstate(pBuf, FC_WS_NORMAL);
  
      add_to_gui_list(ID_LABEL, pBuf);
    
      w = MAX(w, pBuf->size.w);
      h += pBuf->size.h;
      
    }
#endif

    if(can_unit_paradrop(pFocus_Unit) && client_tile_get_known(ptile) &&
      !(is_ocean(ptile->terrain) && is_ground_unit(pFocus_Unit)) &&
      !(is_sailing_unit(pFocus_Unit) && (!is_ocean(ptile->terrain) || !pCity)) &&
      !(((pCity && pplayers_non_attack(game.player_ptr, city_owner(pCity))) 
      || is_non_attack_unit_tile(ptile, game.player_ptr))) &&
      (unit_type(pFocus_Unit)->paratroopers_range >=
	    real_map_distance(pFocus_Unit->tile, ptile))) {
	      
      create_active_iconlabel(pBuf, pWindow->dst, pStr, _("Paradrop here"),
						    paradrop_here_callback);
      pBuf->data.cont = pCont;
      set_wstate(pBuf, FC_WS_NORMAL);
  
      add_to_gui_list(ID_LABEL, pBuf);
    
      w = MAX(w, pBuf->size.w);
      h += pBuf->size.h;
      
    }

  }
  pAdvanced_Terrain_Dlg->pBeginWidgetList = pBuf;
  
  /* ---------- */
  if (n)
  {
    int i;
    struct unit *pUnit;
    struct unit_type *pUnitType = NULL;
    units_h = 0;  
    /* separator */
    pBuf = create_iconlabel(NULL, pWindow->dst, NULL, WF_FREE_THEME);
    
    add_to_gui_list(ID_SEPARATOR, pBuf);
    h += pBuf->next->size.h;
    /* ---------- */
    if (n > 1)
    {
      struct unit *pDefender, *pAttacker;
      struct GUI *pLast = pBuf;
      SDL_Color BLACK = {0, 0, 0, 255};
      bool reset = FALSE;
      int my_units = 0;
      
      #define ADV_NUM_SEEN  15
      
      pDefender = (pFocus_Unit ? get_defender(pFocus_Unit, ptile) : NULL);
      pAttacker = (pFocus_Unit ? get_attacker(pFocus_Unit, ptile) : NULL);
      for(i=0; i<n; i++) {
        pUnit = unit_list_get(ptile->units, i);
	if (pUnit == pFocus_Unit) {
	  continue;
	}
        pUnitType = unit_type(pUnit);
        if(pUnit->owner == game.player_ptr) {
          my_snprintf(cBuf, sizeof(cBuf),
            _("Activate %s (%d / %d) %s (%d,%d,%d) %s"),
            pUnit->veteran ? _("Veteran") : "" ,
            pUnit->hp, pUnitType->hp,
            pUnitType->name,
            pUnitType->attack_strength,
            pUnitType->defense_strength,
            (pUnitType->move_rate / SINGLE_MOVE),
	    unit_activity_text(pUnit));
    
	  create_active_iconlabel(pBuf, pWindow->dst, pStr,
	       cBuf, adv_unit_select_callback);
          pBuf->data.unit = pUnit;
          set_wstate(pBuf, FC_WS_NORMAL);
	  add_to_gui_list(ID_LABEL, pBuf);
	  my_units++;
	} else {
	  int att_chance, def_chance;
	  
          my_snprintf(cBuf, sizeof(cBuf), _("%s %s %s (A:%d D:%d M:%d FP:%d) HP:%d%%"),
            get_nation_by_plr(unit_owner(pUnit))->name,
            (pUnit->veteran ? _("Veteran") : ""),
            pUnitType->name,
            pUnitType->attack_strength,
            pUnitType->defense_strength,
            (pUnitType->move_rate / SINGLE_MOVE),
      	    pUnitType->firepower,
	    ((pUnit->hp * 100) / pUnitType->hp));
    
          /* calculate chance to win */
          if (sdl_get_chance_to_win(&att_chance, &def_chance, pUnit, pFocus_Unit)) {
            cat_snprintf(cBuf, sizeof(cBuf), _(" CtW: Att:%d%% Def:%d%%"),
               att_chance, def_chance);
	  }
	  
	  if (pAttacker && pAttacker == pUnit) {
	    pStr->fgcol = *(get_game_colorRGB(COLOR_OVERVIEW_ENEMY_UNIT));		  
	    reset = TRUE;
	  } else {
	    if (pDefender && pDefender == pUnit) {
	      pStr->fgcol = *(get_game_colorRGB(COLOR_OVERVIEW_MY_UNIT));			
	      reset = TRUE;
	    }
	  }
	  
	  create_active_iconlabel(pBuf, pWindow->dst, pStr, cBuf, NULL);
          
	  if (reset) {
	    pStr->fgcol = BLACK;
	    reset = FALSE;
	  }
	  
	  add_to_gui_list(ID_LABEL, pBuf);
	}
	    
        w = MAX(w, pBuf->size.w);
        units_h += pBuf->size.h;
	
        if (i > ADV_NUM_SEEN - 1)
        {
          set_wflag(pBuf, WF_HIDDEN);
        }
        
      }
      
      pAdvanced_Terrain_Dlg->pEndActiveWidgetList = pLast->prev;
      pAdvanced_Terrain_Dlg->pActiveWidgetList = pLast->prev;
      pAdvanced_Terrain_Dlg->pBeginWidgetList = pBuf;
      pAdvanced_Terrain_Dlg->pBeginActiveWidgetList = pBuf;
            
      if(n > ADV_NUM_SEEN)
      {
        units_h = ADV_NUM_SEEN * pBuf->size.h;
	n = create_vertical_scrollbar(pAdvanced_Terrain_Dlg,
					1, ADV_NUM_SEEN, TRUE, TRUE);
	w += n;
      }

      if (my_units > 1) {
	
	my_snprintf(cBuf, sizeof(cBuf), "%s (%d)", _("Ready all"), my_units);
	create_active_iconlabel(pBuf, pWindow->dst, pStr,
	       cBuf, adv_unit_select_all_callback);
        pBuf->data.unit = pAdvanced_Terrain_Dlg->pEndActiveWidgetList->data.unit;
        set_wstate(pBuf, FC_WS_NORMAL);
	pBuf->ID = ID_LABEL;
	DownAdd(pBuf, pLast);
	h += pBuf->size.h;
	
	my_snprintf(cBuf, sizeof(cBuf), "%s (%d)", _("Sentry idle"), my_units);
	create_active_iconlabel(pBuf, pWindow->dst, pStr,
	       cBuf, adv_unit_sentry_idle_callback);
        pBuf->data.unit = pAdvanced_Terrain_Dlg->pEndActiveWidgetList->data.unit;
        set_wstate(pBuf, FC_WS_NORMAL);
	pBuf->ID = ID_LABEL;
	DownAdd(pBuf, pLast->prev);
	h += pBuf->size.h;
	
	/* separator */
        pBuf = create_iconlabel(NULL, pWindow->dst, NULL, WF_FREE_THEME);
        pBuf->ID = ID_SEPARATOR;
	DownAdd(pBuf, pLast->prev->prev);
        h += pBuf->next->size.h;
	  
      }
      #undef ADV_NUM_SEEN
    }
    else
    { /* n == 1 */
      /* one unit - give orders */
      pUnit = unit_list_get(ptile->units, 0);
      pUnitType = unit_type(pUnit);
      if (pUnit != pFocus_Unit) {
        if ((pCity && pCity->owner == game.player_ptr) ||
	   (pUnit->owner == game.player_ptr))
        {
          my_snprintf(cBuf, sizeof(cBuf),
            _("Activate %s (%d / %d) %s (%d,%d,%d) %s"),
            pUnit->veteran ? _("Veteran") : "" ,
            pUnit->hp, pUnitType->hp,
            pUnitType->name,
            pUnitType->attack_strength,
            pUnitType->defense_strength,
            (pUnitType->move_rate / SINGLE_MOVE),
	    unit_activity_text(pUnit));
    
	  create_active_iconlabel(pBuf, pWindow->dst, pStr,
	    		cBuf, adv_unit_select_callback);
	  pBuf->data.unit = pUnit;
          set_wstate(pBuf, FC_WS_NORMAL);
	
          add_to_gui_list(ID_LABEL, pBuf);
    
          w = MAX(w, pBuf->size.w);
          units_h += pBuf->size.h;
	  /* ---------------- */
	  /* separator */
          pBuf = create_iconlabel(NULL, pWindow->dst, NULL, WF_FREE_THEME);
    
          add_to_gui_list(ID_SEPARATOR, pBuf);
          h += pBuf->next->size.h;
        } else {
	  int att_chance, def_chance;
	
          my_snprintf(cBuf, sizeof(cBuf), _("%s %s %s (A:%d D:%d M:%d FP:%d) HP:%d%%"),
            get_nation_by_plr(unit_owner(pUnit))->name,
            (pUnit->veteran ? _("Veteran") : ""),
            pUnitType->name,
            pUnitType->attack_strength,
            pUnitType->defense_strength,
            (pUnitType->move_rate / SINGLE_MOVE),
      	    pUnitType->firepower,
	    ((pUnit->hp * 100) / pUnitType->hp));
    
	    /* calculate chance to win */
            if (sdl_get_chance_to_win(&att_chance, &def_chance, pUnit, pFocus_Unit)) {
              cat_snprintf(cBuf, sizeof(cBuf), _(" CtW: Att:%d%% Def:%d%%"),
                 att_chance, def_chance);
	    }
	    create_active_iconlabel(pBuf, pWindow->dst, pStr, cBuf, NULL);          
	    add_to_gui_list(ID_LABEL, pBuf);
            w = MAX(w, pBuf->size.w);
            units_h += pBuf->size.h;
	    /* ---------------- */
	    
	    /* separator */
            pBuf = create_iconlabel(NULL, pWindow->dst, NULL, WF_FREE_THEME);
    
            add_to_gui_list(ID_SEPARATOR, pBuf);
            h += pBuf->next->size.h;
	
        }
      }
      /* ---------------- */
      my_snprintf(cBuf, sizeof(cBuf),
            _("View Civiliopedia entry for %s"), pUnitType->name);
      create_active_iconlabel(pBuf, pWindow->dst, pStr,
	    cBuf, unit_help_callback);
      set_wstate(pBuf , FC_WS_NORMAL);
      add_to_gui_list(MAX_ID - pUnit->type->index, pBuf);
    
      w = MAX(w, pBuf->size.w);
      units_h += pBuf->size.h;
      /* ---------------- */  
      pAdvanced_Terrain_Dlg->pBeginWidgetList = pBuf;
    }
    
  }
  /* ---------- */
  
  w += (DOUBLE_FRAME_WH + adj_size(2));
  
  h += units_h;
  
  pWindow->size.x = pos_x;
  pWindow->size.y = pos_y;
  resize_window(pWindow, NULL, NULL, w, h);
  
  w -= (DOUBLE_FRAME_WH + adj_size(2));
  
  if (pAdvanced_Terrain_Dlg->pScroll)
  {
    units_h = n;
  }
  else
  {
    units_h = 0;
  }
  
  /* exit button */
  pBuf = pWindow->prev;
  
  pBuf->size.x = pWindow->size.x + pWindow->size.w-pBuf->size.w-FRAME_WH-1;
  pBuf->size.y = pWindow->size.y + 1;
  
  /* terrain info */
  pBuf = pBuf->prev;
  
  pBuf->size.x = pWindow->size.x + FRAME_WH + 1;
  pBuf->size.y = pWindow->size.y + WINDOW_TILE_HIGH + adj_size(2);
  pBuf->size.w = w;
  h = pBuf->size.h;
  
  area.x = adj_size(10);
  area.h = adj_size(2);
  
  pBuf = pBuf->prev;
  while(pBuf)
  {
    
    if (pBuf == pAdvanced_Terrain_Dlg->pEndActiveWidgetList)
    {
      w -= units_h;
    }
    
    pBuf->size.w = w;
    pBuf->size.x = pBuf->next->size.x;
    pBuf->size.y = pBuf->next->size.y + pBuf->next->size.h;
    
    if (pBuf->ID == ID_SEPARATOR)
    {
      FREESURFACE(pBuf->theme);
      pBuf->size.h = h;
      pBuf->theme = create_surf(w , h , SDL_SWSURFACE);
    
      area.y = pBuf->size.h / 2 - 1;
      area.w = pBuf->size.w - adj_size(20);
      
      SDL_FillRect(pBuf->theme, &area, 64);
      SDL_SetColorKey(pBuf->theme, SDL_SRCCOLORKEY|SDL_RLEACCEL, 0x0);
    }
    
    if(pBuf == pAdvanced_Terrain_Dlg->pBeginWidgetList || 
      pBuf == pAdvanced_Terrain_Dlg->pBeginActiveWidgetList) {
      break;
    }
    pBuf = pBuf->prev;
  }
  
  if (pAdvanced_Terrain_Dlg->pScroll)
  {
    setup_vertical_scrollbar_area(pAdvanced_Terrain_Dlg->pScroll,
	pWindow->size.x + pWindow->size.w - FRAME_WH,
    	pAdvanced_Terrain_Dlg->pEndActiveWidgetList->size.y,
    	pWindow->size.y - pAdvanced_Terrain_Dlg->pEndActiveWidgetList->size.y +
	pWindow->size.h - FRAME_WH, TRUE);
  }
  
  /* -------------------- */
  /* redraw */
  redraw_group(pAdvanced_Terrain_Dlg->pBeginWidgetList, pWindow, 0);

  flush_rect(pWindow->size, FALSE);
}

/* ====================================================================== */
/* ============================ PILLAGE DIALOG ========================== */
/* ====================================================================== */
static struct SMALL_DLG *pPillage_Dlg = NULL;

static int pillage_window_callback(struct GUI *pWindow)
{
  return std_move_window_group_callback(pPillage_Dlg->pBeginWidgetList, pWindow);
}

static int pillage_callback(struct GUI *pWidget)
{
  
  struct unit *pUnit = pWidget->data.unit;
  enum tile_special_type what = MAX_ID - pWidget->ID;
  
  popdown_pillage_dialog();
  
  if (pUnit) 
  {
    request_new_unit_activity_targeted(pUnit, ACTIVITY_PILLAGE, what);
  }
  
  return -1;
}

static int exit_pillage_dlg_callback(struct GUI *pWidget)
{
  popdown_pillage_dialog();
  return -1;
}

/**************************************************************************
  Popdown a dialog asking the unit which improvement they would like to
  pillage.
**************************************************************************/
static void popdown_pillage_dialog(void)
{
  if (pPillage_Dlg) {
    is_unit_move_blocked = FALSE;
    popdown_window_group_dialog(pPillage_Dlg->pBeginWidgetList,
				pPillage_Dlg->pEndWidgetList);
    FC_FREE(pPillage_Dlg);
    flush_dirty();
  }
}

/**************************************************************************
  Popup a dialog asking the unit which improvement they would like to
  pillage.
**************************************************************************/
void popup_pillage_dialog(struct unit *pUnit,
			  bv_special may_pillage)
{
  struct GUI *pWindow = NULL, *pBuf = NULL;
  SDL_String16 *pStr;
  enum tile_special_type what, prereq;
  int w = 0, h;
  
  if (pPillage_Dlg) {
    return;
  }
  
  is_unit_move_blocked = TRUE;
  pPillage_Dlg = fc_calloc(1, sizeof(struct SMALL_DLG));
  
  h = WINDOW_TILE_HIGH + adj_size(3) + FRAME_WH;
      
  /* window */
  pStr = create_str16_from_char(_("What To Pillage") , adj_font(12));
  pStr->style |= TTF_STYLE_BOLD;
  
  pWindow = create_window(NULL, pStr , adj_size(10), adj_size(10), WF_DRAW_THEME_TRANSPARENT);
    
  pWindow->action = pillage_window_callback;
  set_wstate(pWindow, FC_WS_NORMAL);
  w = MAX(w, pWindow->size.w);
  
  add_to_gui_list(ID_PILLAGE_DLG_WINDOW, pWindow);
  pPillage_Dlg->pEndWidgetList = pWindow;
    
  /* ---------- */
  /* exit button */
  pBuf = create_themeicon(pTheme->Small_CANCEL_Icon, pWindow->dst,
  			  			WF_DRAW_THEME_TRANSPARENT);
  w += pBuf->size.w + adj_size(10);
  pBuf->action = exit_pillage_dlg_callback;
  set_wstate(pBuf, FC_WS_NORMAL);
  pBuf->key = SDLK_ESCAPE;
  
  add_to_gui_list(ID_PILLAGE_DLG_EXIT_BUTTON, pBuf);
  /* ---------- */
  
  while ((what = get_preferred_pillage(may_pillage)) != S_LAST) {
      
    bv_special what_bv;
      
    BV_CLR_ALL(what_bv);
    BV_SET(what_bv, what);
    
    create_active_iconlabel(pBuf, pWindow->dst, pStr,
	    (char *) get_special_name(what), pillage_callback);
    pBuf->data.unit = pUnit;
    set_wstate(pBuf, FC_WS_NORMAL);
  
    add_to_gui_list(MAX_ID - what, pBuf);
    
    w = MAX(w, pBuf->size.w);
    h += pBuf->size.h;
        
    clear_special(&may_pillage, what);
    prereq = get_infrastructure_prereq(what);
    if (prereq != S_LAST) {
      clear_special(&may_pillage, prereq);  
    }
  }
  pPillage_Dlg->pBeginWidgetList = pBuf;
  
  /* setup window size and start position */
  
  pWindow->size.w = w + DOUBLE_FRAME_WH;
  pWindow->size.h = h;
  
  put_window_near_map_tile(pWindow,
  		w + DOUBLE_FRAME_WH, h, pUnit->tile);      
  resize_window(pWindow, NULL, NULL, pWindow->size.w, h);
  
  /* setup widget size and start position */

  /* exit button */  
  pBuf = pWindow->prev;
  pBuf->size.x = pWindow->size.x + pWindow->size.w-pBuf->size.w-FRAME_WH-1;
  pBuf->size.y = pWindow->size.y + 1;

  /* first special to pillage */
  pBuf = pBuf->prev;
  setup_vertical_widgets_position(1,
	pWindow->size.x + FRAME_WH,
  	pWindow->size.y + WINDOW_TILE_HIGH + adj_size(2), w, 0,
	pPillage_Dlg->pBeginWidgetList, pBuf);

  /* --------------------- */
  /* redraw */
  redraw_group(pPillage_Dlg->pBeginWidgetList, pWindow, 0);

  flush_rect(pWindow->size, FALSE);
}

/* ======================================================================= */
/* =========================== CONNECT DIALOG ============================ */
/* ======================================================================= */
static struct SMALL_DLG *pConnect_Dlg = NULL;

/**************************************************************************
  Popdown a dialog asking the unit how they want to "connect" their
  location to the destination.
**************************************************************************/
static void popdown_connect_dialog(void)
{
  if (pConnect_Dlg) {
    is_unit_move_blocked = FALSE;
    popdown_window_group_dialog(pConnect_Dlg->pBeginWidgetList,
				pConnect_Dlg->pEndWidgetList);
    FC_FREE(pConnect_Dlg);
  }
}

/**************************************************************************
                                  Revolutions
**************************************************************************/
static struct SMALL_DLG *pRevolutionDlg = NULL;
  
/**************************************************************************
  ...
**************************************************************************/
static int revolution_dlg_ok_callback(struct GUI *pButton)
{
  start_revolution();

  popdown_revolution_dialog();
  
  flush_dirty();
  return (-1);
}

/**************************************************************************
  ...
**************************************************************************/
static int revolution_dlg_cancel_callback(struct GUI *pCancel_Button)
{
  popdown_revolution_dialog();
  flush_dirty();
  return (-1);
}

/**************************************************************************
  ...
**************************************************************************/
static int move_revolution_dlg_callback(struct GUI *pWindow)
{
  return std_move_window_group_callback(pRevolutionDlg->pBeginWidgetList, pWindow);
}

/**************************************************************************
  Close the revolution dialog.
**************************************************************************/
static void popdown_revolution_dialog(void)
{
  if(pRevolutionDlg) {
    popdown_window_group_dialog(pRevolutionDlg->pBeginWidgetList,
			      pRevolutionDlg->pEndWidgetList);
    FC_FREE(pRevolutionDlg);
    enable_and_redraw_revolution_button();
  }
}

/* ==================== Public ========================= */

/**************************************************************************
                           Sellect Goverment Type
**************************************************************************/
static struct SMALL_DLG *pGov_Dlg = NULL;

/**************************************************************************
  Close the government dialog.
**************************************************************************/
static void popdown_government_dialog(void)
{
  if(pGov_Dlg) {
    popdown_window_group_dialog(pGov_Dlg->pBeginWidgetList,
                                pGov_Dlg->pEndWidgetList);
    FC_FREE(pGov_Dlg);
    enable_and_redraw_revolution_button();  
  }
}

/**************************************************************************
  ...
**************************************************************************/
static int government_dlg_callback(struct GUI *pGov_Button)
{
  set_government_choice(get_government(MAX_ID - pGov_Button->ID));
  
  popdown_government_dialog();
  return (-1);
}

/**************************************************************************
  ...
**************************************************************************/
static int move_government_dlg_callback(struct GUI *pWindow)
{
  return std_move_window_group_callback(pGov_Dlg->pBeginWidgetList, pWindow);
}

/**************************************************************************
  Public -

  Popup a dialog asking the player what government to switch to (this
  happens after a revolution completes).
**************************************************************************/
static void popup_government_dialog(void)
{
  SDL_Surface *pLogo = NULL;
  struct SDL_String16 *pStr = NULL;
  struct GUI *pGov_Button = NULL;
  struct GUI *pWindow = NULL;
  struct government *pGov = NULL;
  int i, j;
  Uint16 max_w, max_h = 0;

  if (pGov_Dlg) {
    return;
  }

  pGov_Dlg = fc_calloc(1, sizeof(struct SMALL_DLG));
  
  /* create window */
  pStr = create_str16_from_char(_("Choose Your New Government"), 12);
  pStr->style |= TTF_STYLE_BOLD;
  /* this win. size is temp. */
  pWindow = create_window(NULL, pStr, 10, 30, 0);
  pWindow->action = move_government_dlg_callback;
  pGov_Dlg->pEndWidgetList = pWindow;
  max_w = pWindow->size.w;
  add_to_gui_list(ID_GOVERNMENT_DLG_WINDOW, pWindow);

  /* create gov. buttons */
  j = 0;
  for (i = 0; i < game.control.government_count; i++) {

    if (i == game.government_when_anarchy->index) {
      continue;
    }

    if (can_change_to_government(game.player_ptr, get_government(i))) {

      pGov = get_government(i);
      pStr = create_str16_from_char(pGov->name, 12);
      pGov_Button =
          create_icon_button(GET_SURF(get_government_sprite(tileset, pGov)), pWindow->dst, pStr, 0);
      pGov_Button->action = government_dlg_callback;

      max_w = MAX(max_w, pGov_Button->size.w);
      max_h = MAX(max_h, pGov_Button->size.h);
      
      /* ugly hack */
      add_to_gui_list((MAX_ID - i), pGov_Button);
      j++;

    }
  }

  pGov_Dlg->pBeginWidgetList = pGov_Button;

  max_w += 10;
  max_h += 4;

  /* set window start positions */
  pWindow->size.x = (Main.screen->w - pWindow->size.w) / 2;
  pWindow->size.y = (Main.screen->h - pWindow->size.h) / 2;

  /* create window background */
  pLogo = get_logo_gfx();
  if (resize_window(pWindow, pLogo, NULL, max_w + 20,
                    j * (max_h + 10) + WINDOW_TILE_HIGH + 6)) {
    FREESURFACE(pLogo);
  }
  
  pWindow->size.w = max_w + 20;
  pWindow->size.h = j * (max_h + 10) + WINDOW_TILE_HIGH + 6;
  
  /* set buttons start positions and size */
  j = 1;
  while (pGov_Button != pGov_Dlg->pEndWidgetList) {
    pGov_Button->size.w = max_w;
    pGov_Button->size.h = max_h;
    pGov_Button->size.x = pWindow->size.x + 10;
    pGov_Button->size.y = pWindow->size.y + pWindow->size.h -
        (j++) * (max_h + 10);
    set_wstate(pGov_Button, FC_WS_NORMAL);

    pGov_Button = pGov_Button->next;
  }

  set_wstate(pWindow, FC_WS_NORMAL);

  /* redraw */
  redraw_group(pGov_Dlg->pBeginWidgetList, pWindow, 0);

  flush_rect(pWindow->size, FALSE);
}

/**************************************************************************
  Popup a dialog asking if the player wants to start a revolution.
**************************************************************************/
void popup_revolution_dialog(void)
{
  SDL_Surface *pLogo;
  struct SDL_String16 *pStr = NULL;
  struct GUI *pLabel = NULL;
  struct GUI *pWindow = NULL;
  struct GUI *pCancel_Button = NULL;
  struct GUI *pOK_Button = NULL;
  int ww;

  if(pRevolutionDlg) {
    return;
  }
  
  if (game.player_ptr->revolution_finishes >= 0) {
    popup_government_dialog();
    return;
  }
  
  pRevolutionDlg = fc_calloc(1, sizeof(struct SMALL_DLG));
    
  /* create ok button */
  pOK_Button =
      create_themeicon_button_from_chars(pTheme->Small_OK_Icon,
			  Main.gui, _("Revolution!"), adj_font(10), 0);

  /* create cancel button */
  pCancel_Button =
      create_themeicon_button_from_chars(pTheme->Small_CANCEL_Icon,
  					Main.gui, _("Cancel"), adj_font(10), 0);
  
  /* correct sizes */
  pCancel_Button->size.w += 6;

  /* create text label */
  pStr = create_str16_from_char(_("You say you wanna revolution?"), adj_font(10));
  pStr->style |= (TTF_STYLE_BOLD|SF_CENTER);
  pStr->fgcol.r = 255;
  pStr->fgcol.g = 255;
  /* pStr->fgcol.b = 255; */
  pLabel = create_iconlabel(NULL, Main.gui, pStr, 0);

  /* create window */
  pStr = create_str16_from_char(_("REVOLUTION!"), adj_font(12));
  pStr->style |= TTF_STYLE_BOLD;
  if ((pOK_Button->size.w + pCancel_Button->size.w + adj_size(30)) >
      pLabel->size.w + adj_size(20)) {
    ww = pOK_Button->size.w + pCancel_Button->size.w + adj_size(30);
  } else {
    ww = pLabel->size.w + adj_size(20);
  }

  pWindow = create_window(Main.gui, pStr, ww,
	pOK_Button->size.h + pLabel->size.h + WINDOW_TILE_HIGH + adj_size(25), 0);

  /* set actions */
  pWindow->action = move_revolution_dlg_callback;
  pCancel_Button->action = revolution_dlg_cancel_callback;
  pOK_Button->action = revolution_dlg_ok_callback;

  /* set keys */
  pOK_Button->key = SDLK_RETURN;
  
  /* I make this hack to center label on window */
  pLabel->size.w = pWindow->size.w;

  /* set start positions */
  pWindow->size.x = (Main.screen->w - pWindow->size.w) / 2;
  pWindow->size.y = (Main.screen->h - pWindow->size.h) / 2;

  pOK_Button->size.x = pWindow->size.x + adj_size(10);
  pOK_Button->size.y = pWindow->size.y + pWindow->size.h -
      pOK_Button->size.h - adj_size(10);

  pCancel_Button->size.y = pOK_Button->size.y;
  pCancel_Button->size.x = pWindow->size.x + pWindow->size.w -
      pCancel_Button->size.w - adj_size(10);
  pLabel->size.x = pWindow->size.x;
  pLabel->size.y = pWindow->size.y + WINDOW_TILE_HIGH + adj_size(5);

  /* create window background */
  pLogo = get_logo_gfx();
  if (resize_window
      (pWindow, pLogo, NULL, pWindow->size.w, pWindow->size.h)) {
    FREESURFACE(pLogo);
  }
  
  SDL_SetAlpha(pWindow->theme, 0x0, 0x0);
  /* enable widgets */
  set_wstate(pCancel_Button, FC_WS_NORMAL);
  set_wstate(pOK_Button, FC_WS_NORMAL);
  set_wstate(pWindow, FC_WS_NORMAL);

  /* add widgets to main list */
  pRevolutionDlg->pEndWidgetList = pWindow;
  add_to_gui_list(ID_REVOLUTION_DLG_WINDOW, pWindow);
  add_to_gui_list(ID_REVOLUTION_DLG_LABEL, pLabel);
  add_to_gui_list(ID_REVOLUTION_DLG_CANCEL_BUTTON, pCancel_Button);
  add_to_gui_list(ID_REVOLUTION_DLG_OK_BUTTON, pOK_Button);
  pRevolutionDlg->pBeginWidgetList = pOK_Button;

  /* redraw */
  redraw_group(pOK_Button, pWindow, 0);
  sdl_dirty_rect(pWindow->size);
  flush_dirty();
}

/**************************************************************************
                                Nation Wizard
**************************************************************************/
static struct ADVANCED_DLG *pNationDlg = NULL;
static struct SMALL_DLG *pHelpDlg = NULL;
  
struct NAT {
  Uint8 nation_city_style;	/* sellected city style */
  Uint8 selected_leader;	/* if not unique -> sellected leader */
  Sint8 nation;			/* sellected nation */
  bool leader_sex;		/* sellected leader sex */
  struct GUI *pChange_Sex;
  struct GUI *pName_Edit;
  struct GUI *pName_Next;
  struct GUI *pName_Prev;
};

static int nations_dialog_callback(struct GUI *pWindow);
static int nation_button_callback(struct GUI *pNation);
static int races_dialog_ok_callback(struct GUI *pStart_Button);
static int races_dialog_cancel_callback(struct GUI *pButton);
static int next_name_callback(struct GUI *pNext_Button);
static int prev_name_callback(struct GUI *pPrev_Button);
static int change_sex_callback(struct GUI *pSex);
static void select_random_leader(Nation_type_id nation);
static void change_nation_label(void);

/**************************************************************************
  ...
**************************************************************************/
static int nations_dialog_callback(struct GUI *pWindow)
{
  if(sellect_window_group_dialog(pNationDlg->pBeginWidgetList, pWindow)) {
    flush_rect(pWindow->size, FALSE);
  }      
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int races_dialog_ok_callback(struct GUI *pStart_Button)
{
  struct NAT *pSetup = (struct NAT *)(pNationDlg->pEndWidgetList->data.ptr);
  char *pStr = convert_to_chars(pSetup->pName_Edit->string16->text);

  /* perform a minimum of sanity test on the name */
  if (strlen(pStr) == 0) {
    append_output_window(_("You must type a legal name."));
    pSellected_Widget = pStart_Button;
    set_wstate(pStart_Button, FC_WS_SELLECTED);
    redraw_tibutton(pStart_Button);
    flush_rect(pStart_Button->size, FALSE);
    return (-1);
  }

  dsend_packet_nation_select_req(&aconnection, races_player->player_no, pSetup->nation,
				 pSetup->leader_sex, pStr,
				 pSetup->nation_city_style);
  FC_FREE(pStr);

  popdown_races_dialog();  
  flush_dirty();
  
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int change_sex_callback(struct GUI *pSex)
{
  struct NAT *pSetup = (struct NAT *)(pNationDlg->pEndWidgetList->data.ptr);
    
  if (pSetup->leader_sex) {
    copy_chars_to_string16(pSetup->pChange_Sex->string16, _("Female"));
  } else {
    copy_chars_to_string16(pSetup->pChange_Sex->string16, _("Male"));
  }
  pSetup->leader_sex = !pSetup->leader_sex;
  
  if (pSex) {
    pSellected_Widget = pSex;
    set_wstate(pSex, FC_WS_SELLECTED);

    redraw_ibutton(pSex);
    flush_rect(pSex->size, FALSE);
  }
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int next_name_callback(struct GUI *pNext)
{
  int dim;
  struct NAT *pSetup = (struct NAT *)(pNationDlg->pEndWidgetList->data.ptr);
  struct leader *leaders = get_nation_leaders(get_nation_by_idx(pSetup->nation), &dim);
    
  pSetup->selected_leader++;
  
  /* change leadaer sex */
  if (pSetup->leader_sex != leaders[pSetup->selected_leader].is_male) {
    change_sex_callback(NULL);
  }
    
  /* change leadaer name */
  copy_chars_to_string16(pSetup->pName_Edit->string16,
  				leaders[pSetup->selected_leader].name);
  
  FC_FREE(pLeaderName);
  pLeaderName = fc_calloc(1, strlen(leaders[pSetup->selected_leader].name) + 1);
  mystrlcpy(pLeaderName, leaders[pSetup->selected_leader].name,
                       strlen(leaders[pSetup->selected_leader].name) + 1);
  
  if ((dim - 1) == pSetup->selected_leader) {
    set_wstate(pSetup->pName_Next, FC_WS_DISABLED);
  }

  if (get_wstate(pSetup->pName_Prev) == FC_WS_DISABLED) {
    set_wstate(pSetup->pName_Prev, FC_WS_NORMAL);
  }

  if (!(get_wstate(pSetup->pName_Next) == FC_WS_DISABLED)) {
    pSellected_Widget = pSetup->pName_Next;
    set_wstate(pSetup->pName_Next, FC_WS_SELLECTED);
  }

  redraw_edit(pSetup->pName_Edit);
  redraw_tibutton(pSetup->pName_Prev);
  redraw_tibutton(pSetup->pName_Next);
  dirty_rect(pSetup->pName_Edit->size.x - pSetup->pName_Prev->size.w,
  		pSetup->pName_Edit->size.y,
		pSetup->pName_Edit->size.w + pSetup->pName_Prev->size.w +
  		pSetup->pName_Next->size.w, pSetup->pName_Edit->size.h);
  
  redraw_ibutton(pSetup->pChange_Sex);
  sdl_dirty_rect(pSetup->pChange_Sex->size);
  
  flush_dirty();
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int prev_name_callback(struct GUI *pPrev)
{
  int dim;
  struct NAT *pSetup = (struct NAT *)(pNationDlg->pEndWidgetList->data.ptr);
  struct leader *leaders = get_nation_leaders(get_nation_by_idx(pSetup->nation), &dim);
    
  pSetup->selected_leader--;

  /* change leadaer sex */
  if (pSetup->leader_sex != leaders[pSetup->selected_leader].is_male) {
    change_sex_callback(NULL);
  }
  
  /* change leadaer name */
  copy_chars_to_string16(pSetup->pName_Edit->string16,
  				leaders[pSetup->selected_leader].name);
  
  FC_FREE(pLeaderName);
  pLeaderName = fc_calloc(1, strlen(leaders[pSetup->selected_leader].name) + 1);
  mystrlcpy(pLeaderName, leaders[pSetup->selected_leader].name,
                       strlen(leaders[pSetup->selected_leader].name) + 1);
  
  if (!pSetup->selected_leader) {
    set_wstate(pSetup->pName_Prev, FC_WS_DISABLED);
  }

  if (get_wstate(pSetup->pName_Next) == FC_WS_DISABLED) {
    set_wstate(pSetup->pName_Next, FC_WS_NORMAL);
  }

  if (!(get_wstate(pSetup->pName_Prev) == FC_WS_DISABLED)) {
    pSellected_Widget = pSetup->pName_Prev;
    set_wstate(pSetup->pName_Prev, FC_WS_SELLECTED);
  }

  redraw_edit(pSetup->pName_Edit);
  redraw_tibutton(pSetup->pName_Prev);
  redraw_tibutton(pSetup->pName_Next);
  dirty_rect(pSetup->pName_Edit->size.x - pSetup->pName_Prev->size.w,
  		pSetup->pName_Edit->size.y, pSetup->pName_Edit->size.w +
  		pSetup->pName_Prev->size.w + pSetup->pName_Next->size.w,
		pSetup->pName_Edit->size.h);
  
  redraw_ibutton(pSetup->pChange_Sex);
  sdl_dirty_rect(pSetup->pChange_Sex->size);
  
  flush_dirty();
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int races_dialog_cancel_callback(struct GUI *pButton)
{
  popdown_races_dialog();
  flush_dirty();
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int city_style_callback(struct GUI *pWidget)
{
  struct NAT *pSetup = (struct NAT *)(pNationDlg->pEndWidgetList->data.ptr);
  struct GUI *pGUI = get_widget_pointer_form_main_list(MAX_ID - 1000 -
					    pSetup->nation_city_style);
  set_wstate(pGUI, FC_WS_NORMAL);
  redraw_icon2(pGUI);
  sdl_dirty_rect(pGUI->size);
  
  set_wstate(pWidget, FC_WS_DISABLED);
  redraw_icon2(pWidget);
  sdl_dirty_rect(pWidget->size);
  
  pSetup->nation_city_style = MAX_ID - 1000 - pWidget->ID;
  
  flush_dirty();
  pSellected_Widget = NULL;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int help_dlg_callback(struct GUI *pWindow)
{
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int cancel_help_dlg_callback(struct GUI *pWidget)
{
  if (pHelpDlg) {
    popdown_window_group_dialog(pHelpDlg->pBeginWidgetList,
			pHelpDlg->pEndWidgetList);
    FC_FREE(pHelpDlg);
    if (pWidget) {
      flush_dirty();
    }
  }
  return -1;
}

/**************************************************************************
   ...
**************************************************************************/
static int nation_button_callback(struct GUI *pNationButton)
{
      
  set_wstate(pNationButton, FC_WS_SELLECTED);
  pSellected_Widget = pNationButton;
  
  if (Main.event.button.button == SDL_BUTTON_LEFT) {
    struct NAT *pSetup = (struct NAT *)(pNationDlg->pEndWidgetList->data.ptr);
      
    if (pSetup->nation == MAX_ID - pNationButton->ID) {
      redraw_widget(pNationButton);
      flush_rect(pNationButton->size, FALSE);
      return -1;
    }
  
    pSetup->nation = MAX_ID - pNationButton->ID;
  
    change_nation_label();
  
    enable(MAX_ID - 1000 - pSetup->nation_city_style);
    pSetup->nation_city_style = get_nation_city_style(get_nation_by_idx(pSetup->nation));
    disable(MAX_ID - 1000 - pSetup->nation_city_style);
    
    select_random_leader(pSetup->nation);
    
    redraw_group(pNationDlg->pBeginWidgetList, pNationDlg->pEndWidgetList, 0);
    flush_rect(pNationDlg->pEndWidgetList->size, FALSE);
  } else {
    /* pop up legend info */
    int w = 0, h;
    struct GUI *pWindow, *pCancel;
    SDL_String16 *pStr;
    SDL_Surface *pText, *pText2;
    SDL_Rect area;
    struct nation_type *pNation = get_nation_by_idx(MAX_ID - pNationButton->ID);
      
    redraw_widget(pNationButton);
    sdl_dirty_rect(pNationButton->size);
  
    if (!pHelpDlg) {
    
      pHelpDlg = fc_calloc(1, sizeof(struct SMALL_DLG));
    
      pStr = create_str16_from_char("Nation's Legend", adj_font(12));
      pStr->style |= TTF_STYLE_BOLD;
  
      pWindow = create_window(NULL, pStr, adj_size(10), adj_size(10), 0);
      pWindow->action = help_dlg_callback;
      w = pWindow->size.w;
      set_wstate(pWindow, FC_WS_NORMAL);
    
      pHelpDlg->pEndWidgetList = pWindow;
      add_to_gui_list(ID_WINDOW, pWindow);
    
      pCancel = create_themeicon_button_from_chars(pTheme->CANCEL_Icon,
    				pWindow->dst, _("Cancel"), adj_font(14), 0);
      pCancel->action = cancel_help_dlg_callback;
      set_wstate(pCancel, FC_WS_NORMAL);
      pCancel->key = SDLK_ESCAPE;
      add_to_gui_list(ID_BUTTON, pCancel);
      pHelpDlg->pBeginWidgetList = pCancel;
    } else {
      pWindow = pHelpDlg->pEndWidgetList;
      pCancel = pHelpDlg->pBeginWidgetList;
      /* undraw window */
      blit_entire_src(pWindow->gfx, pWindow->dst,
				      pWindow->size.x, pWindow->size.y);
      sdl_dirty_rect(pWindow->size);
    }
    
    if (pNation->legend && *(pNation->legend) != '\0') {
      pStr = create_str16_from_char(pNation->legend, adj_font(12));
    } else {
      pStr = create_str16_from_char("SORRY... NO INFO", adj_font(12));
    }
    
    pStr->fgcol = (SDL_Color){255, 255, 255, 255};
    pText = create_text_surf_smaller_that_w(pStr, Main.screen->w - adj_size(20));
  
    copy_chars_to_string16(pStr, Q_(pNation->name_plural));
    pText2 = create_text_surf_from_str16(pStr);
  
    FREESTRING16(pStr);
    
    /* create window background */
    w = MAX(w, pText2->w + adj_size(20));
    w = MAX(w, pText->w + adj_size(20));
    w = MAX(w, pCancel->size.w + adj_size(20));
    h = WINDOW_TILE_HIGH + adj_size(10) + pText2->h + adj_size(10) + pText->h
                    + adj_size(10) + pCancel->size.h + adj_size(10) + FRAME_WH;
  
    pWindow->size.x = (pWindow->dst->w - w) / 2;
    pWindow->size.y = (pWindow->dst->h - h) / 2;
  
    resize_window(pWindow, NULL,
  	get_game_colorRGB(COLOR_THEME_BACKGROUND_BROWN), w, h);
  
    area.x = adj_size(10);
    area.y = WINDOW_TILE_HIGH + adj_size(10);
    SDL_BlitSurface(pText2, NULL, pWindow->theme, &area);
    area.y += (pText2->h + adj_size(10));
    FREESURFACE(pText2);
  
    SDL_BlitSurface(pText, NULL, pWindow->theme, &area);
    FREESURFACE(pText);
  
    pCancel->size.x = pWindow->size.x + (pWindow->size.w - pCancel->size.w) / 2;
    pCancel->size.y = pWindow->size.y +
     	            pWindow->size.h - pCancel->size.h - adj_size(10) - FRAME_WH;
  
    /* redraw */
    redraw_group(pCancel, pWindow, 0);

    sdl_dirty_rect(pWindow->size);
  
    flush_dirty();

  }
  
  return -1;
}

/**************************************************************************
   ...
**************************************************************************/
static int leader_name_edit_callback(struct GUI *pEdit)
{
  char *name = convert_to_chars(pEdit->string16->text);

  if (name) {
    FC_FREE(name);
  } else {
    /* empty input -> restore previous content */
    copy_chars_to_string16(pEdit->string16, pLeaderName);
    redraw_edit(pEdit);
    sdl_dirty_rect(pEdit->size);
    flush_dirty();
  }
  
  return -1;
}
/* =========================================================== */

/**************************************************************************
   ...
**************************************************************************/
static void change_nation_label(void)
{
  SDL_Surface *pTmp_Surf, *pTmp_Surf_zoomed;
  struct GUI *pWindow = pNationDlg->pEndWidgetList;
  struct NAT *pSetup = (struct NAT *)(pWindow->data.ptr);  
  struct GUI *pLabel = pSetup->pName_Edit->next;
  struct nation_type *pNation = get_nation_by_idx(pSetup->nation);
    
  pTmp_Surf = adj_surf(GET_SURF(get_nation_flag_sprite(tileset,
                                       get_nation_by_idx(pSetup->nation))));
  pTmp_Surf_zoomed = ZoomSurface(pTmp_Surf, 1.0, 1.0, 1);  

/* only free the flag sprite if it is a copy created by the adj_surf() macro */
#ifdef SMALL_SCREEN
  FREESURFACE(pTmp_Surf);
#endif
  
  FREESURFACE(pLabel->theme);
  pLabel->theme = pTmp_Surf_zoomed;
  SDL_SetAlpha(pLabel->theme, 0, 0);
  
  copy_chars_to_string16(pLabel->string16, Q_(pNation->name_plural));
  
  remake_label_size(pLabel);
  
  pLabel->size.x = pWindow->size.x + pWindow->size.w / 2 +
  				(pWindow->size.w/2 - pLabel->size.w) / 2;
    
}

/**************************************************************************
  Selectes a leader and the appropriate sex. Updates the gui elements
  and the selected_* variables.
**************************************************************************/
static void select_random_leader(Nation_type_id nation)
{
  int dim;
  struct NAT *pSetup = (struct NAT *)(pNationDlg->pEndWidgetList->data.ptr);
  struct leader *leaders = get_nation_leaders(get_nation_by_idx(nation), &dim);
  
    
  pSetup->selected_leader = myrand(dim);
  copy_chars_to_string16(pSetup->pName_Edit->string16,
  				leaders[pSetup->selected_leader].name);
  
  FC_FREE(pLeaderName);
  pLeaderName = fc_calloc(1, strlen(leaders[pSetup->selected_leader].name) + 1);
  mystrlcpy(pLeaderName, leaders[pSetup->selected_leader].name,
                       strlen(leaders[pSetup->selected_leader].name) + 1);
  
  /* initialize leader sex */
  pSetup->leader_sex = leaders[pSetup->selected_leader].is_male;

  if (pSetup->leader_sex) {
    copy_chars_to_string16(pSetup->pChange_Sex->string16, _("Male"));
  } else {
    copy_chars_to_string16(pSetup->pChange_Sex->string16, _("Female"));
  }

  /* disable navigation buttons */
  set_wstate(pSetup->pName_Prev, FC_WS_DISABLED);
  set_wstate(pSetup->pName_Next, FC_WS_DISABLED);

  if (dim > 1) {
    /* if selected leader is not the first leader, enable "previous leader" button */
    if (pSetup->selected_leader > 0) {
      set_wstate(pSetup->pName_Prev, FC_WS_NORMAL);
    }

    /* if selected leader is not the last leader, enable "next leader" button */
    if (pSetup->selected_leader < (dim - 1)) {
      set_wstate(pSetup->pName_Next, FC_WS_NORMAL);
    }
  }
  
}

static int get_playable_nation_count() {
 
  int playable_nation_count = 0;
    
  nations_iterate(pnation) {
    if (pnation->is_playable && !pnation->player && pnation->is_available)
      ++playable_nation_count;        
  } nations_iterate_end;

  return playable_nation_count;
  
}

/**************************************************************************
  Popup the nation selection dialog.
**************************************************************************/
void popup_races_dialog(struct player *pplayer)
{
  struct GUI *pWindow, *pWidget = NULL, *pBuf, *pLast_City_Style;
  SDL_String16 *pStr;
  int len = 0;
  int w = adj_size(10), h = adj_size(10);
  SDL_Surface *pTmp_Surf, *pTmp_Surf_zoomed = NULL;
  SDL_Surface *pMain_Bg, *pText_Name, *pText_Class;
  SDL_Color color = {255,255,255,128};
  SDL_Rect dst;
  struct NAT *pSetup;
    
  #define TARGETS_ROW 5
  #define TARGETS_COL 1
  
  if (pNationDlg) {
    return;
  }
  
  races_player = pplayer;
  
  pNationDlg = fc_calloc(1, sizeof(struct ADVANCED_DLG));
  
  /* create window widget */
  pStr = create_str16_from_char(_("What nation will you be?"), adj_font(12));
  pStr->style |= TTF_STYLE_BOLD;
  
  pWindow = create_window(NULL, pStr, w, h, WF_FREE_DATA);
  pWindow->action = nations_dialog_callback;
  set_wstate(pWindow, FC_WS_NORMAL);
  pSetup = fc_calloc(1, sizeof(struct NAT));
  pWindow->data.ptr = (void *)pSetup;
    
  pNationDlg->pEndWidgetList = pWindow;
  add_to_gui_list(ID_NATION_WIZARD_WINDOW, pWindow);
  /* --------------------------------------------------------- */
  /* create nations list */

  /* Create Imprv Background Icon */
  pMain_Bg = create_surf_alpha(adj_size(96*2), adj_size(64), SDL_SWSURFACE);
  
  SDL_FillRect(pMain_Bg, NULL, SDL_MapRGBA(pMain_Bg->format, color.r,
					    color.g, color.b, color.unused));
  putframe(pMain_Bg, 0, 0, pMain_Bg->w - 1, pMain_Bg->h - 1, 0xFF000000);
  
  pStr = create_string16(NULL, 0, adj_font(12));
  pStr->style |= (SF_CENTER|TTF_STYLE_BOLD);
  pStr->render = 3;
  pStr->bgcol = color;

  /* fill list */
  pText_Class = NULL;
    
  nations_iterate(pNation) {

    if (!is_nation_playable(pNation) || !pNation->is_available) {
      continue;
    }

    pTmp_Surf_zoomed = adj_surf(GET_SURF(get_nation_flag_sprite(tileset, pNation)));    

    pTmp_Surf = crop_rect_from_surface(pMain_Bg, NULL);
          
    copy_chars_to_string16(pStr, Q_(pNation->name_plural));
    change_ptsize16(pStr, adj_font(12));
    pText_Name = create_text_surf_smaller_that_w(pStr, pTmp_Surf->w - adj_size(4));
    SDL_SetAlpha(pText_Name, 0x0, 0x0);
    
#if 0      
    if (pNation->legend && *(pNation->legend) != '\0') {
      copy_chars_to_string16(pStr, pNation->legend);
      change_ptsize16(pStr, adj_font(10));
      pText_Class = create_text_surf_smaller_that_w(pStr, pTmp_Surf->w - adj_size(4));
      SDL_SetAlpha(pText_Class, 0x0, 0x0);
    }
#endif    
    
    dst.x = (pTmp_Surf->w - pTmp_Surf_zoomed->w) / 2;
    len = pTmp_Surf_zoomed->h +
	    adj_size(10) + pText_Name->h + (pText_Class ? pText_Class->h : 0);
    dst.y = (pTmp_Surf->h - len) / 2;
    SDL_BlitSurface(pTmp_Surf_zoomed, NULL, pTmp_Surf, &dst);
    dst.y += (pTmp_Surf_zoomed->h + adj_size(10));
    
    dst.x = (pTmp_Surf->w - pText_Name->w) / 2;
    SDL_BlitSurface(pText_Name, NULL, pTmp_Surf, &dst);
    dst.y += pText_Name->h;
    FREESURFACE(pText_Name);
    
    if (pText_Class) {
      dst.x = (pTmp_Surf->w - pText_Class->w) / 2;
      SDL_BlitSurface(pText_Class, NULL, pTmp_Surf, &dst);
      FREESURFACE(pText_Class);
    }
    
    pWidget = create_icon2(pTmp_Surf, pWindow->dst,
    			(WF_DRAW_THEME_TRANSPARENT|WF_FREE_THEME));
    
    set_wstate(pWidget, FC_WS_NORMAL);
    
    pWidget->action = nation_button_callback;

    w = MAX(w, pWidget->size.w);
    h = MAX(h, pWidget->size.h);
    
    add_to_gui_list(MAX_ID - pNation->index, pWidget);
    
    if (pNation->index > (TARGETS_ROW * TARGETS_COL - 1)) {
      set_wflag(pWidget, WF_HIDDEN);
    }
    
  } nations_iterate_end;
  
  FREESURFACE(pMain_Bg);
    
  pNationDlg->pEndActiveWidgetList = pWindow->prev;
  pNationDlg->pBeginActiveWidgetList = pWidget;
  pNationDlg->pBeginWidgetList = pWidget;
    
  if(get_playable_nation_count() > TARGETS_ROW * TARGETS_COL) {
      pNationDlg->pActiveWidgetList = pNationDlg->pEndActiveWidgetList;
      create_vertical_scrollbar(pNationDlg,
		    		TARGETS_COL, TARGETS_ROW, TRUE, TRUE);
  }
  
  /* ----------------------------------------------------------------- */
    
  /* nation name */
  
  pSetup->nation = myrand(get_playable_nation_count());
  pSetup->nation_city_style = get_nation_city_style(get_nation_by_idx(pSetup->nation));
  
  copy_chars_to_string16(pStr, Q_(get_nation_by_idx(pSetup->nation)->name_plural));
  change_ptsize16(pStr, adj_font(24));
  pStr->render = 2;
  pStr->fgcol = color;
  
  pTmp_Surf_zoomed = adj_surf(GET_SURF(get_nation_flag_sprite(tileset, get_nation_by_idx(pSetup->nation))));
  
  pWidget = create_iconlabel(pTmp_Surf_zoomed, pWindow->dst, pStr,
  			(WF_ICON_ABOVE_TEXT|WF_ICON_CENTER|WF_FREE_GFX));
  pBuf = pWidget;
  add_to_gui_list(ID_LABEL, pWidget);
  
  /* create leader name edit */
  pWidget = create_edit_from_unichars(NULL, pWindow->dst,
						  NULL, 0, adj_font(16), adj_size(200), 0);
  pWidget->size.h = adj_size(24);
  
  set_wstate(pWidget, FC_WS_NORMAL);
  pWidget->action = leader_name_edit_callback;
  add_to_gui_list(ID_NATION_WIZARD_LEADER_NAME_EDIT, pWidget);
  pSetup->pName_Edit = pWidget;
  
  /* create next leader name button */
  pWidget = create_themeicon_button(pTheme->R_ARROW_Icon,
						  pWindow->dst, NULL, 0);
  pWidget->action = next_name_callback;
  clear_wflag(pWidget, WF_DRAW_FRAME_AROUND_WIDGET);
  add_to_gui_list(ID_NATION_WIZARD_NEXT_LEADER_NAME_BUTTON, pWidget);
  pWidget->size.h = pWidget->next->size.h;
  pSetup->pName_Next = pWidget;
  
  /* create prev leader name button */
  pWidget = create_themeicon_button(pTheme->L_ARROW_Icon,
  						pWindow->dst, NULL, 0);
  pWidget->action = prev_name_callback;
  clear_wflag(pWidget, WF_DRAW_FRAME_AROUND_WIDGET);
  add_to_gui_list(ID_NATION_WIZARD_PREV_LEADER_NAME_BUTTON, pWidget);
  pWidget->size.h = pWidget->next->size.h;
  pSetup->pName_Prev = pWidget;
  
  /* change sex button */
  
  pWidget = create_icon_button_from_chars(NULL, pWindow->dst, _("Male"), adj_font(14), 0);
  pWidget->action = change_sex_callback;
  pWidget->size.w = adj_size(100);
  pWidget->size.h = adj_size(22);
  set_wstate(pWidget, FC_WS_NORMAL);
  pSetup->pChange_Sex = pWidget;
  
  /* add to main widget list */
  add_to_gui_list(ID_NATION_WIZARD_CHANGE_SEX_BUTTON, pWidget);

  /* ---------------------------------------------------------- */
  int i = 0;
  while (i < game.control.styles_count) {
    if (!city_style_has_requirements(&city_styles[i])) {
      pWidget = create_icon2(adj_surf(ZoomSurface(
            GET_SURF(get_sample_city_sprite(tileset, i)), 0.5, 0.5, 0)),
                                    pWindow->dst, WF_DRAW_THEME_TRANSPARENT);
      pWidget->action = city_style_callback;
      if (i != pSetup->nation_city_style) {
        set_wstate(pWidget, FC_WS_NORMAL);
      }
      len = pWidget->size.w;
      add_to_gui_list(MAX_ID - 1000 - i, pWidget);
      i++;
      break;
    }
    i++;
  }

  len += adj_size(3);

  for (; (i < game.control.styles_count && i < 64); i++) {
    if (!city_style_has_requirements(&city_styles[i])) {
      pWidget = create_icon2(adj_surf(ZoomSurface(
            GET_SURF(get_sample_city_sprite(tileset, i)), 0.5, 0.5, 0)),
                                    pWindow->dst, WF_DRAW_THEME_TRANSPARENT);
      pWidget->action = city_style_callback;
      if (i != pSetup->nation_city_style) {
        set_wstate(pWidget, FC_WS_NORMAL);
      }
      len += (pWidget->size.w + adj_size(3));
      add_to_gui_list(MAX_ID - 1000 - i, pWidget);
    }
  }
  pLast_City_Style = pWidget;
  /* ---------------------------------------------------------- */
  
  /* create disconnection button */
  pWidget = create_themeicon_button_from_chars(pTheme->BACK_Icon, pWindow->dst,
					 _("Cancel"), adj_font(12), 0);
  pWidget->action = races_dialog_cancel_callback;
  set_wstate(pWidget, FC_WS_NORMAL);
  
  add_to_gui_list(ID_NATION_WIZARD_DISCONNECT_BUTTON, pWidget);

  /* create start button */
  pWidget =
      create_themeicon_button_from_chars(pTheme->FORWARD_Icon, pWindow->dst,
				_("OK"), adj_font(12), WF_ICON_CENTER_RIGHT);
  pWidget->action = races_dialog_ok_callback;

  pWidget->size.w += adj_size(60);
  set_wstate(pWidget, FC_WS_NORMAL);
  add_to_gui_list(ID_NATION_WIZARD_START_BUTTON, pWidget);
  pWidget->size.w = MAX(pWidget->size.w, pWidget->next->size.w);
  pWidget->next->size.w = pWidget->size.w;
  
  pNationDlg->pBeginWidgetList = pWidget;
  /* ---------------------------------------------------------- */
      
  
  pWindow->size.x = (Main.screen->w - adj_size(640)) / 2;
  pWindow->size.y = (Main.screen->h - adj_size(480)) / 2;
    
  pMain_Bg = get_logo_gfx();
  if(resize_window(pWindow, pMain_Bg, NULL, adj_size(640), adj_size(480))) {
    FREESURFACE(pMain_Bg);
  }
  
  /* nations */
  
  h = pNationDlg->pEndActiveWidgetList->size.h * TARGETS_ROW;
  i = (pWindow->size.h - WINDOW_TILE_HIGH - h) / 2;
  setup_vertical_widgets_position(TARGETS_COL,
	pWindow->size.x + FRAME_WH + adj_size(10),
	pWindow->size.y + WINDOW_TILE_HIGH + i,
	  0, 0, pNationDlg->pBeginActiveWidgetList,
			  pNationDlg->pEndActiveWidgetList);
  
  if(pNationDlg->pScroll) {
    SDL_Rect area;
  
    w = pNationDlg->pEndActiveWidgetList->size.w * TARGETS_COL;    
    setup_vertical_scrollbar_area(pNationDlg->pScroll,
	pWindow->size.x + FRAME_WH + w + adj_size(12),
    	pWindow->size.y + WINDOW_TILE_HIGH + i,	h, FALSE);
    
    area.x = FRAME_WH + w + adj_size(11);
    area.y = WINDOW_TILE_HIGH + i;
    area.w = pNationDlg->pScroll->pUp_Left_Button->size.w + adj_size(2);
    area.h = h;
    SDL_FillRectAlpha(pWindow->theme, &area, &color);
    putframe(pWindow->theme, area.x, area.y - 1,
		area.x + area.w, area.y + area.h, 0xFF000000);
  }
   
  /* Sellected Nation Name */
  pBuf->size.x = pWindow->size.x + pWindow->size.w / 2 +
  				(pWindow->size.w/2 - pBuf->size.w) / 2;
  pBuf->size.y = pWindow->size.y + WINDOW_TILE_HIGH + adj_size(50);
  
  /* Leader Name Edit */
  pBuf = pBuf->prev;
  pBuf->size.x = pWindow->size.x + pWindow->size.w / 2 +
  				(pWindow->size.w/2 - pBuf->size.w) / 2;
  pBuf->size.y = pWindow->size.y + (pWindow->size.h - pBuf->size.h) / 2
                                 - adj_size(20);
  
  /* Next Leader Name Button */
  pBuf = pBuf->prev;
  pBuf->size.x = pBuf->next->size.x + pBuf->next->size.w;
  pBuf->size.y = pBuf->next->size.y;
  
  /* Prev Leader Name Button */
  pBuf = pBuf->prev;
  pBuf->size.x = pBuf->next->next->size.x - pBuf->size.w;
  pBuf->size.y = pBuf->next->size.y;
  
  /* Change Leader Sex Button */
  pBuf = pBuf->prev;
  pBuf->size.x = pWindow->size.x + pWindow->size.w / 2 +
  				(pWindow->size.w/2 - pBuf->size.w) / 2;
  pBuf->size.y = pBuf->next->size.y + pBuf->next->size.h + adj_size(20);
  
  /* First City Style Button */
  pBuf = pBuf->prev;
  pBuf->size.x = pWindow->size.x + pWindow->size.w / 2 +
  				(pWindow->size.w/2 - len) / 2 - adj_size(20);
  pBuf->size.y = pBuf->next->size.y + pBuf->next->size.h + adj_size(20);
  
  /* Rest City Style Buttons */
  if (pBuf != pLast_City_Style) {
    do {
      pBuf = pBuf->prev;
      pBuf->size.x = pBuf->next->size.x + pBuf->next->size.w + adj_size(3);
      pBuf->size.y = pBuf->next->size.y;
    } while (pLast_City_Style != pBuf);
  }
  
  /* Disconnect Button */
  pBuf = pBuf->prev;
  pBuf->size.x = pWindow->size.x + pWindow->size.w / 2 +
  				(pWindow->size.w/2 - pBuf->size.w) / 2;
  pBuf->size.y = pBuf->next->size.y + pBuf->next->size.h + adj_size(20);
  
  /* Start Button */
  pBuf = pBuf->prev;
  pBuf->size.x = pBuf->next->size.x;
  pBuf->size.y = pBuf->next->size.y + pBuf->next->size.h + adj_size(10);
  
  /* -------------------------------------------------------------------- */
  
  select_random_leader(pSetup->nation);
  
  redraw_group(pNationDlg->pBeginWidgetList, pWindow, 0);
  
  flush_rect(pWindow->size, FALSE);
}

/**************************************************************************
  Close the nation selection dialog.  This should allow the user to
  (at least) select a unit to activate.
**************************************************************************/
void popdown_races_dialog(void)
{
  if (pNationDlg) {
    popdown_window_group_dialog(pNationDlg->pBeginWidgetList,
			  pNationDlg->pEndWidgetList);

    cancel_help_dlg_callback(NULL);
    
    FC_FREE(pLeaderName);
    
    FC_FREE(pNationDlg->pScroll);
    FC_FREE(pNationDlg);
  }
}

/**************************************************************************
  In the nation selection dialog, make already-taken nations unavailable.
  This information is contained in the packet_nations_used packet.
**************************************************************************/
void races_toggles_set_sensitive()
{
  struct NAT *pSetup;
  bool change = FALSE;
  struct GUI *pNat;

  if (!pNationDlg)
    return;
  
  pSetup = (struct NAT *)(pNationDlg->pEndWidgetList->data.ptr);
  
  nations_iterate(nation) {
  
    if (!nation->is_available || nation->player) {
      freelog(LOG_DEBUG,"  [%d]: %d = %s", nation->index, (!nation->is_available || nation->player),
	      get_nation_name(nation));

      pNat = get_widget_pointer_form_main_list(MAX_ID - nation->index);
      set_wstate(pNat, FC_WS_DISABLED);
    
      if (nation->index == pSetup->nation) {
        change = TRUE;
      }
    }
  } nations_iterate_end;
  
  if (change) {
    do {
      pSetup->nation = myrand(get_playable_nation_count());
      pNat = get_widget_pointer_form_main_list(MAX_ID - pSetup->nation);
    } while(get_wstate(pNat) == FC_WS_DISABLED);
    if (get_wstate(pSetup->pName_Edit) == FC_WS_PRESSED) {
      force_exit_from_event_loop();
      set_wstate(pSetup->pName_Edit, FC_WS_NORMAL);
    }
    change_nation_label();
    enable(MAX_ID - 1000 - pSetup->nation_city_style);
    pSetup->nation_city_style = get_nation_city_style(get_nation_by_idx(pSetup->nation));
    disable(MAX_ID - 1000 - pSetup->nation_city_style);
    select_random_leader(pSetup->nation);
  }
  redraw_group(pNationDlg->pBeginWidgetList, pNationDlg->pEndWidgetList, 0);
  flush_rect(pNationDlg->pEndWidgetList->size, FALSE);
}
