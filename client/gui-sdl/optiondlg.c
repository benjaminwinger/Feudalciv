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

/**********************************************************************
                          optiondlg.c  -  description
                             -------------------
    begin                : Sun Aug 11 2002
    copyright            : (C) 2002 by Rafa� Bursig
    email                : Rafa� Bursig <bursig@poczta.fm>
 **********************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*#include <ctype.h>*/
#include <stdlib.h>
#include <string.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "fcintl.h"
#include "support.h"

#include "gui_mem.h"

#include "clinet.h"

#include "graphics.h"
#include "unistring.h"
#include "gui_string.h"
#include "gui_id.h"
#include "gui_stuff.h"
#include "gui_zoom.h"
#include "gui_tilespec.h"
#include "gui_main.h"

#include "civclient.h"
#include "chatline.h"
#include "menu.h"
#include "dialogs.h"
#include "control.h"
#include "mapctrl.h"
#include "mapview.h"
#include "messagewin.h"
#include "wldlg.h"
#include "colors.h"
#include "connectdlg.h"

#include "optiondlg.h"
#include "options.h"

static struct OPT_DLG {
  struct GUI *pBeginOptionsWidgetList;
  struct GUI *pEndOptionsWidgetList;
  struct GUI *pBeginCoreOptionsWidgetList;
  struct GUI *pBeginMainOptionsWidgetList;
  struct ADVANCED_DLG *pADlg;
} *pOption_Dlg = NULL;

static struct GUI *pOptions_Button = NULL;

/**************************************************************************
  ...
**************************************************************************/
static void center_optiondlg(void)
{
  Sint16 newX =
	  (pOption_Dlg->pEndOptionsWidgetList->dst->w - 
  			pOption_Dlg->pEndOptionsWidgetList->size.w) / 2;
  Sint16 newY =
	  (pOption_Dlg->pEndOptionsWidgetList->dst->h - 
  			pOption_Dlg->pEndOptionsWidgetList->size.h) / 2;

  set_new_group_start_pos(pOption_Dlg->pBeginOptionsWidgetList,
			  pOption_Dlg->pEndOptionsWidgetList,
			  newX - pOption_Dlg->pEndOptionsWidgetList->size.x,
			  newY - pOption_Dlg->pEndOptionsWidgetList->size.y);
  
}

/**************************************************************************
  ...
**************************************************************************/
static int main_optiondlg_callback(struct GUI *pWindow)
{
  return std_move_window_group_callback(pOption_Dlg->pBeginOptionsWidgetList,
				pWindow);
}

/**************************************************************************
  ...
**************************************************************************/
static int sound_callback(struct GUI *pWidget)
{
  return -1;
}

/**************************************************************************
			Global WorkLists Handler
**************************************************************************/

/**************************************************************************
  ...
**************************************************************************/
static int edit_worklist_callback(struct GUI *pWidget)
{
  /* Port ME */
  return -1;  
}


/**************************************************************************
  ...
**************************************************************************/
static int add_new_worklist_callback(struct GUI *pWidget)
{
  struct GUI *pNew_WorkList_Widget = NULL;
  struct GUI *pWindow = pOption_Dlg->pEndOptionsWidgetList;
  bool scroll = pOption_Dlg->pADlg->pActiveWidgetList == NULL;
  bool redraw_all = FALSE;
  int j;

  set_wstate(pWidget, WS_NORMAL);
  pSellected_Widget = NULL;
  
  /* Find the next free worklist for this player */

  for (j = 0; j < MAX_NUM_WORKLISTS; j++)
    if (!game.player_ptr->worklists[j].is_valid)
      break;

  /* No more worklist slots free.  (!!!Maybe we should tell the user?) */
  if (j == MAX_NUM_WORKLISTS) {
    return -2;
  }
  
  /* Validate this slot. */
  init_worklist(&game.player_ptr->worklists[j]);
  game.player_ptr->worklists[j].is_valid = TRUE;
  strcpy(game.player_ptr->worklists[j].name, _("empty worklist"));
  
  /* create list element */
  pNew_WorkList_Widget = create_iconlabel_from_chars(NULL, pWidget->dst, 
      		game.player_ptr->worklists[j].name, 12, WF_DRAW_THEME_TRANSPARENT);
  pNew_WorkList_Widget->ID = MAX_ID - j;
  set_wstate(pNew_WorkList_Widget, WS_NORMAL);
  pNew_WorkList_Widget->size.w = pWidget->size.w;
  pNew_WorkList_Widget->action = edit_worklist_callback;
  
  /* add to widget list */
  redraw_all = add_widget_to_vertical_scroll_widget_list(pOption_Dlg->pADlg,
				      pNew_WorkList_Widget,
				      pWidget, TRUE,
					pWindow->size.x + 20,
		      pWindow->size.y + WINDOW_TILE_HIGH + 1 + 20);

  /* find if there was scrollbar shown */
  if(scroll && pOption_Dlg->pADlg->pActiveWidgetList != NULL) {
    pWindow = pOption_Dlg->pADlg->pEndActiveWidgetList->next;
    int len = pOption_Dlg->pADlg->pScroll->pUp_Left_Button->size.w;
    do {
      pWindow = pWindow->prev;
      pWindow->size.w -= len;
      FREESURFACE(pWindow->gfx);
    } while(pWindow != pOption_Dlg->pADlg->pBeginActiveWidgetList);
  }
  
  /* find if that was last empty list */
  for (j = 0; j < MAX_NUM_WORKLISTS; j++)
    if (!game.player_ptr->worklists[j].is_valid)
      break;

  /* No more worklist slots free. */
  if (j == MAX_NUM_WORKLISTS) {
    set_wstate(pWidget, WS_DISABLED);
    pWidget->string16->forecol = *(get_game_colorRGB(COLOR_STD_DISABLED));
  }
  
  
  if(redraw_all) {
    redraw_group(pOption_Dlg->pBeginOptionsWidgetList,
  				pOption_Dlg->pEndOptionsWidgetList, 0);
    sdl_dirty_rect(pOption_Dlg->pEndOptionsWidgetList->size);
  } else {
    /* redraw only new widget and dock widget */
    if (!pWidget->gfx) {
      refresh_widget_background(pWidget);
    }
    redraw_widget(pWidget);
    sdl_dirty_rect(pWidget->size);
    if (!pNew_WorkList_Widget->gfx) {
      refresh_widget_background(pNew_WorkList_Widget);
    }
    redraw_widget(pNew_WorkList_Widget);
    sdl_dirty_rect(pNew_WorkList_Widget->size);
  }
  flush_dirty();
  return -1;
}


/**************************************************************************
  ...
**************************************************************************/
static int work_lists_callback(struct GUI *pWidget)
{
  struct GUI *pBuf = NULL, *pWindow = pOption_Dlg->pEndOptionsWidgetList;
  int i , count = 0, len;
  SDL_Rect area = {pWindow->size.x + 15,
    			pWindow->size.y + WINDOW_TILE_HIGH + 1 + 15,
    			pWindow->size.w - 30,
			pWindow->size.h - WINDOW_TILE_HIGH - 2 - 30};
  
  /* clear flag */
  SDL_Client_Flags &= ~CF_OPTION_MAIN;

  /* hide main widget group */
  hide_group(pOption_Dlg->pBeginMainOptionsWidgetList,
	     pOption_Dlg->pBeginCoreOptionsWidgetList->prev);
  /* ----------------------------- */
	/* create white background */		
  pBuf = create_iconlabel(create_surf(area.w, area.h - 30, SDL_SWSURFACE),
  			pWindow->dst, NULL, WF_FREE_THEME);
  pBuf->size = area;
  SDL_FillRect(pBuf->theme, NULL, get_game_color(COLOR_STD_WHITE, pBuf->theme));
  SDL_SetAlpha(pBuf->theme, SDL_SRCALPHA, 128);
  putframe(pBuf->theme, 0, 0, pBuf->theme->w - 1, pBuf->theme->h - 1, 0x0);
  add_to_gui_list(ID_LABEL, pBuf);
  
  /* ----------------------------- */
  for (i = 0; i < MAX_NUM_WORKLISTS; i++) {
    if (game.player_ptr->worklists[i].is_valid) {
      pBuf = create_iconlabel_from_chars(NULL, pWindow->dst, 
      		game.player_ptr->worklists[i].name, 12,
					      WF_DRAW_THEME_TRANSPARENT);
      set_wstate(pBuf, WS_NORMAL);
      add_to_gui_list(MAX_ID - i, pBuf);
      pBuf->action = edit_worklist_callback;
      
      count++;
    
      if(count>13) {
	set_wflag(pBuf, WF_HIDDEN);
      }
    }
  }
  
  if(count < MAX_NUM_WORKLISTS) {
    pBuf = create_iconlabel_from_chars(NULL, pWindow->dst, 
      		_("Add new worklist"), 12, WF_DRAW_THEME_TRANSPARENT);
    set_wstate(pBuf, WS_NORMAL);
    add_to_gui_list(ID_ADD_NEW_WORKLIST, pBuf);
    pBuf->action = add_new_worklist_callback;
    
    count++;
    
    if(count>13) {
      set_wflag(pBuf, WF_HIDDEN);
    }
  }
  /* ----------------------------- */
  
  pOption_Dlg->pADlg = MALLOC(sizeof(struct ADVANCED_DLG));
  
  pOption_Dlg->pADlg->pEndWidgetList = pOption_Dlg->pEndOptionsWidgetList;   
  
  pOption_Dlg->pADlg->pEndActiveWidgetList =
		  pOption_Dlg->pBeginMainOptionsWidgetList->prev->prev;
  
  pOption_Dlg->pADlg->pBeginActiveWidgetList = pBuf;
  pOption_Dlg->pADlg->pBeginWidgetList = pBuf;
  
  
  pOption_Dlg->pADlg->pScroll = MALLOC(sizeof(struct ScrollBar));
  pOption_Dlg->pADlg->pScroll->count = count;
  pOption_Dlg->pADlg->pScroll->active = 13;
  
  len = create_vertical_scrollbar(pOption_Dlg->pADlg,
		  area.x + area.w - 1, area.y + 1, area.h - 32, 13,
		  TRUE, TRUE, TRUE);
  
  if(count>13) {
    pOption_Dlg->pADlg->pActiveWidgetList =
				pOption_Dlg->pADlg->pEndActiveWidgetList;
  } else {
    hide_scrollbar(pOption_Dlg->pADlg->pScroll);
    len = 0;
  }
  /* ----------------------------- */
  
  pBuf = pOption_Dlg->pADlg->pEndActiveWidgetList;
  pBuf->size.y = pWindow->size.y + WINDOW_TILE_HIGH + 1 + 20;
  pBuf->size.x = pWindow->size.x + 20;
  pBuf->size.w = area.w - 10 - len;
  if(pBuf != pOption_Dlg->pADlg->pBeginActiveWidgetList) {
    pBuf = pBuf->prev;
    while(pBuf) {
      pBuf->size.x = pBuf->next->size.x;
      pBuf->size.y = pBuf->next->size.y + pBuf->next->size.h;
      pBuf->size.w = area.w - 10 - len;
      if(pBuf == pOption_Dlg->pADlg->pBeginActiveWidgetList) {
        break;
      }
      pBuf = pBuf->prev;
    }
  }
  
  pOption_Dlg->pBeginOptionsWidgetList = pOption_Dlg->pADlg->pBeginWidgetList;
  /* ----------------------------- */
  
  redraw_group(pOption_Dlg->pBeginOptionsWidgetList,
  				pOption_Dlg->pEndOptionsWidgetList, 0);
  flush_rect(pWindow->size);

  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int change_mode_callback(struct GUI *pWidget)
{

  char cBuf[50] = "";
  int mode;
  Uint32 tmp_flags = Main.screen->flags;
  struct GUI *pWindow =
      pOption_Dlg->pBeginMainOptionsWidgetList->prev->prev->prev->prev;

  /* don't free this */
  SDL_Rect **pModes_Rect =
      SDL_ListModes(NULL, SDL_FULLSCREEN | Main.screen->flags);

  mode = 0;
  while (pWindow) {

    if (get_wstate(pWindow) == WS_DISABLED) {
      if (pModes_Rect[mode]) {
        set_wstate(pWindow, WS_NORMAL);
      }
      break;
    }
    mode++;
    pWindow = pWindow->prev;
  }

  set_wstate(pWidget, WS_DISABLED);


  if (SDL_Client_Flags & CF_TOGGLED_FULLSCREEN) {
    SDL_Client_Flags &= ~CF_TOGGLED_FULLSCREEN;
    tmp_flags ^= SDL_FULLSCREEN;
    tmp_flags ^= SDL_RESIZABLE;
  }

  mode = MAX_ID - pWidget->ID;
  
  if (pModes_Rect[mode])
  {
    set_video_mode(pModes_Rect[mode]->w, pModes_Rect[mode]->h, tmp_flags);
  } else {
    set_video_mode(640, 480, tmp_flags);
  }


  /* change setting label */
  if (Main.screen->flags & SDL_FULLSCREEN) {
    my_snprintf(cBuf, sizeof(cBuf), _("Current Setup\nFullscreen %dx%d"),
	    Main.screen->w, Main.screen->h);
  } else {
    my_snprintf(cBuf, sizeof(cBuf), _("Current Setup\n%dx%d"),
	    Main.screen->w, Main.screen->h);
  }

  FREE(pOption_Dlg->pBeginMainOptionsWidgetList->prev->string16->text);
  pOption_Dlg->pBeginMainOptionsWidgetList->prev->string16->text =
      convert_to_utf16(cBuf);


  /* move units window to botton-right corrner */
  set_new_units_window_pos();

  /* move minimap window to botton-left corrner */
  set_new_mini_map_window_pos();

  /* move cooling/warming icons to botton-right corrner */

  pWindow = get_widget_pointer_form_main_list(ID_WARMING_ICON);
  pWindow->size.x = pWindow->dst->w - 10 - (pWindow->size.w << 1);

  /* ID_COOLING_ICON */
  pWindow = pWindow->next;
  pWindow->size.x = pWindow->dst->w - 10 - pWindow->size.w;

  center_optiondlg();
  center_meswin_dialog();
  new_input_line_position();
  
  /* Options Dlg Window */
  pWindow = pOption_Dlg->pEndOptionsWidgetList;

  if (get_client_state() != CLIENT_GAME_RUNNING_STATE) {

    draw_intro_gfx();

    refresh_widget_background(pWindow);

    redraw_group(pOption_Dlg->pBeginOptionsWidgetList, 
    				pOption_Dlg->pEndOptionsWidgetList, 0);

  } else {

    get_new_view_rectsize();
    
    update_info_label();
    update_unit_info_label(get_unit_in_focus());
    refresh_overview_viewrect();
    
    /* with redrawing full map */
    center_on_something();
    
    refresh_widget_background(pWindow);

    redraw_group(pOption_Dlg->pBeginOptionsWidgetList,
			    pOption_Dlg->pEndOptionsWidgetList, 0);

  }
  
  flush_all();
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int togle_fullscreen_callback(struct GUI *pWidget)
{
  int i = 0;
  struct GUI *pTmp = NULL;

  /* don't free this */
  SDL_Rect **pModes_Rect =
      SDL_ListModes(NULL, SDL_FULLSCREEN | Main.screen->flags);

  redraw_icon(pWidget);
  flush_rect(pWidget->size);

  SDL_Client_Flags ^= CF_TOGGLED_FULLSCREEN;

  while (pModes_Rect[i] && pModes_Rect[i]->w != Main.screen->w) {
    i++;
  }

  if (pModes_Rect[i])
  {
    pTmp = get_widget_pointer_form_main_list(MAX_ID - i);

    if (get_wstate(pTmp) == WS_DISABLED) {
      set_wstate(pTmp, WS_NORMAL);
    } else {
      set_wstate(pTmp, WS_DISABLED);
    }

    redraw_ibutton(pTmp);
    
    if (!pModes_Rect[i+1] && pTmp->prev)
    {
      sdl_dirty_rect(pTmp->size);
      if (get_checkbox_state(pWidget)) {
        set_wstate(pTmp->prev, WS_DISABLED);
      } else {
        set_wstate(pTmp->prev, WS_NORMAL);
      }
      redraw_ibutton(pTmp->prev);
      sdl_dirty_rect(pTmp->prev->size);
      flush_dirty();
    } else {
      flush_rect(pTmp->size);
    }
  } else {
    
    pTmp = get_widget_pointer_form_main_list(MAX_ID - i);

    if (get_checkbox_state(pWidget)||(Main.screen->w == 640)) {
      set_wstate(pTmp, WS_DISABLED);
    } else {
      set_wstate(pTmp, WS_NORMAL);
    }

    redraw_ibutton(pTmp);
    flush_rect(pTmp->size);
  }
  
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int video_callback(struct GUI *pWidget)
{
  int i = 0;
  char cBuf[50] = "";
  Uint16 len = 0, count = 0;
  Sint16 xxx;	/* tmp */
  SDL_Rect **pModes_Rect = NULL;
  struct GUI *pTmpGui = NULL, *pWindow = pOption_Dlg->pEndOptionsWidgetList;
  Uint16 **pModes = get_list_modes(Main.screen->flags);

  if (!pModes) {
    return 0;
  }

  /* don't free this */
  pModes_Rect = SDL_ListModes(NULL, SDL_FULLSCREEN | Main.screen->flags);

  /* clear flag */
  SDL_Client_Flags &= ~CF_OPTION_MAIN;

  /* hide main widget group */
  hide_group(pOption_Dlg->pBeginMainOptionsWidgetList,
	     pOption_Dlg->pBeginCoreOptionsWidgetList->prev);

  /* create setting label */
  if (Main.screen->flags & SDL_FULLSCREEN) {
    my_snprintf(cBuf, sizeof(cBuf),_("Current Setup\nFullscreen %dx%d"),
	    Main.screen->w, Main.screen->h);
  } else {
    my_snprintf(cBuf, sizeof(cBuf),_("Current Setup\n%dx%d"), Main.screen->w,
	    Main.screen->h);
  }

  pTmpGui = create_iconlabel(NULL, pWindow->dst,
  			create_str16_from_char(cBuf, 10), 0);
  pTmpGui->string16->style |= TTF_STYLE_BOLD;
  pTmpGui->string16->forecol.r = 255;
  pTmpGui->string16->forecol.g = 255;
  /*pTmpGui->string16->forecol.b = 255; */

  /* set window width to 'pTmpGui' for center string */
  pTmpGui->size.w = pWindow->size.w;

  pTmpGui->size.x = pWindow->size.x;
  pTmpGui->size.y = pWindow->size.y + WINDOW_TILE_HIGH + 6;

  add_to_gui_list(ID_OPTIONS_RESOLUTION_LABEL, pTmpGui);

  /* fullscreen mode label */
  pTmpGui =
      create_themelabel(create_filled_surface
			(150, 30, SDL_SWSURFACE, NULL), pWindow->dst,
			create_str16_from_char(_("Fullscreen Mode"),
					       10), 150, 30, 0);
  
  pTmpGui->string16->style |= TTF_STYLE_BOLD;
  
  xxx = pTmpGui->size.x = pWindow->size.x +
      ((pWindow->size.w - pTmpGui->size.w) / 2);
  pTmpGui->size.y = pWindow->size.y + WINDOW_TILE_HIGH + 40;

  add_to_gui_list(ID_OPTIONS_FULLSCREEN_LABEL, pTmpGui);

  /* fullscreen check box */
  if (Main.screen->flags & SDL_FULLSCREEN) {
    pTmpGui = create_checkbox(pWindow->dst, TRUE, WF_DRAW_THEME_TRANSPARENT);
  } else {
    pTmpGui = create_checkbox(pWindow->dst, FALSE, WF_DRAW_THEME_TRANSPARENT);
  }

  pTmpGui->action = togle_fullscreen_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = xxx + 5;
  pTmpGui->size.y = pWindow->size.y + WINDOW_TILE_HIGH + 45;

  add_to_gui_list(ID_OPTIONS_TOGGLE_FULLSCREEN_CHECKBOX, pTmpGui);
  /* ------------------------- */
  
  /* create modes buttons */
  for (i = 0; pModes[i]; i++) {
    if(i&&pModes_Rect[i]->w == pModes_Rect[i - 1]->w) {
      continue;
    }
  
    pTmpGui = create_icon_button_from_unichar(NULL, pWindow->dst,
    							pModes[i], 14, 0);

    if (len) {
      pTmpGui->size.w = len;
    } else {
      pTmpGui->size.w += 6;
      len = pTmpGui->size.w;
    }

    if (pModes_Rect[i]->w != Main.screen->w) {
      set_wstate(pTmpGui, WS_NORMAL);
    }
    
    count++;
    pTmpGui->action = change_mode_callback;

    /* ugly hack */
    add_to_gui_list((MAX_ID - i), pTmpGui);
  }

  if ((i == 1)&&(pModes_Rect[0]->w > 640))
  {
    pTmpGui = create_icon_button_from_unichar(NULL, pWindow->dst,
			    convert_to_utf16("640x480"), 14, 0);

    if (len) {
      pTmpGui->size.w = len;
    } else {
      pTmpGui->size.w += 6;
      len = pTmpGui->size.w;
    }
    
    if(!(Main.screen->flags & SDL_FULLSCREEN)&&(Main.screen->w != 640))
    {
      set_wstate(pTmpGui, WS_NORMAL);
    }
    
    count++;
    pTmpGui->action = change_mode_callback;

    /* ugly hack */
    add_to_gui_list((MAX_ID - 1), pTmpGui);
  }
  /* ------------------------- */
  pOption_Dlg->pBeginOptionsWidgetList = pTmpGui;

  if(count % 5) {
    count /= 5;
    count++;
  } else {
    count /= 5;
  }
  
  /* set start positions */
  pTmpGui = pOption_Dlg->pBeginMainOptionsWidgetList->prev->prev->prev->prev;
  
  pTmpGui->size.x =
      pWindow->size.x +
	  (pWindow->size.w - count * (pTmpGui->size.w + 10) - 10) / 2;
  pTmpGui->size.y = pWindow->size.y + 110;
  
  count = 0;
  for (pTmpGui = pTmpGui->prev; pTmpGui; pTmpGui = pTmpGui->prev) {
    if(count < 4) {
      pTmpGui->size.x = pTmpGui->next->size.x;
      pTmpGui->size.y = pTmpGui->next->size.y + pTmpGui->next->size.h + 10;
      count++;
    } else {
      pTmpGui->size.x = pTmpGui->next->size.x + pTmpGui->size.w + 10;
      pTmpGui->size.y = pWindow->size.y + 110;
      count = 0;
    }
  }

  FREE(pModes);

  redraw_group(pOption_Dlg->pBeginOptionsWidgetList,
			  pOption_Dlg->pEndOptionsWidgetList, 0);
  flush_rect(pWindow->size);

  return -1;
}

/* ===================================================================== */

/**************************************************************************
  ...
**************************************************************************/
static int sound_bell_at_new_turn_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  sound_bell_at_new_turn ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int smooth_move_units_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  if (smooth_move_units) {
    smooth_move_units = FALSE;
    set_wstate(pWidget->prev->prev, WS_DISABLED);
    redraw_edit(pWidget->prev->prev);
    flush_rect(pWidget->prev->prev->size);
  } else {
    smooth_move_units = TRUE;
    set_wstate(pWidget->prev->prev, WS_NORMAL);
    redraw_edit(pWidget->prev->prev);
    flush_rect(pWidget->prev->prev->size);
  }
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int smooth_move_unit_steps_callback(struct GUI *pWidget)
{
  char *tmp = convert_to_chars(pWidget->string16->text);
  sscanf(tmp, "%d", &smooth_move_unit_steps);
  FREE(tmp);
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int do_combat_animation_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  do_combat_animation ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int auto_center_on_unit_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  auto_center_on_unit ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int auto_center_on_combat_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  auto_center_on_combat ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int wakeup_focus_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  wakeup_focus ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int popup_new_cities_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  popup_new_cities ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int ask_city_names_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  ask_city_name ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int auto_turn_done_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  auto_turn_done ^= 1;
  return -1;
}

/**************************************************************************
  popup local settings.
**************************************************************************/
static int local_setting_callback(struct GUI *pWidget)
{
  SDL_Color text_color = {255, 255, 0, 255};
  SDL_String16 *pStr = NULL;
  struct GUI *pTmpGui = NULL, *pWindow = pOption_Dlg->pEndOptionsWidgetList;
  char cBuf[3];
  
  /* clear flag */
  SDL_Client_Flags &= ~CF_OPTION_MAIN;

  /* hide main widget group */
  hide_group(pOption_Dlg->pBeginMainOptionsWidgetList,
	     pOption_Dlg->pBeginCoreOptionsWidgetList->prev);

  /* 'sound befor new turn' */
  /* check box */
  pTmpGui = create_checkbox(pWindow->dst, sound_bell_at_new_turn,
			    WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = sound_bell_at_new_turn_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 15;
  pTmpGui->size.y = pWindow->size.y + WINDOW_TILE_HIGH + 6;

  add_to_gui_list(ID_OPTIONS_LOCAL_SOUND_CHECKBOX, pTmpGui);

  /* 'sound befor new turn' label */
  pStr = create_str16_from_char(_("Sound bell at new turn"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 55;

  add_to_gui_list(ID_OPTIONS_LOCAL_SOUND_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      ((pTmpGui->next->size.h - pTmpGui->size.h) / 2);

  /* 'smooth unit moves' */
  /* check box */
  pTmpGui = create_checkbox(pWindow->dst,
  		smooth_move_units, WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = smooth_move_units_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 15;

  add_to_gui_list(ID_OPTIONS_LOCAL_MOVE_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("Smooth unit moves"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 55;

  add_to_gui_list(ID_OPTIONS_LOCAL_MOVE_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      ((pTmpGui->next->size.h - pTmpGui->size.h) / 2);

  /* 'smooth unit move steps' */

  /* edit */
  my_snprintf(cBuf, sizeof(cBuf), "%d", smooth_move_unit_steps);
  pTmpGui = create_edit_from_chars(NULL, pWindow->dst, cBuf, 11, 25,
					  WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = smooth_move_unit_steps_callback;

  if (smooth_move_units) {
    set_wstate(pTmpGui, WS_NORMAL);
  }

  pTmpGui->size.x = pWindow->size.x + 12;

  add_to_gui_list(ID_OPTIONS_LOCAL_MOVE_STEP_EDIT, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("Smooth unit move steps"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 55;

  add_to_gui_list(ID_OPTIONS_LOCAL_MOVE_STEP_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      (pTmpGui->next->size.h - pTmpGui->size.h) / 2;

  /* 'show combat anim' */

  /* check box */
  pTmpGui = create_checkbox(pWindow->dst,
      			do_combat_animation, WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = do_combat_animation_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 15;

  add_to_gui_list(ID_OPTIONS_LOCAL_COMBAT_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("Show combat animation"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 55;

  add_to_gui_list(ID_OPTIONS_LOCAL_COMBAT_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      (pTmpGui->next->size.h - pTmpGui->size.h) / 2;

  /* 'auto center on units' */
  /* check box */
  pTmpGui = create_checkbox(pWindow->dst,
      		auto_center_on_unit, WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = auto_center_on_unit_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 15;

  add_to_gui_list(ID_OPTIONS_LOCAL_ACENTER_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("Auto Center on Units"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 55;

  add_to_gui_list(ID_OPTIONS_LOCAL_ACENTER_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      ((pTmpGui->next->size.h - pTmpGui->size.h) / 2);

  /* 'auto center on combat' */
  /* check box */
  pTmpGui = create_checkbox(pWindow->dst, auto_center_on_combat,
			    WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = auto_center_on_combat_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 15;

  add_to_gui_list(ID_OPTIONS_LOCAL_COMBAT_CENTER_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("Auto Center on Combat"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 55;

  add_to_gui_list(ID_OPTIONS_LOCAL_COMBAT_CENTER_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      (pTmpGui->next->size.h - pTmpGui->size.h) / 2;

  /* 'wakeup focus' */

  /* check box */
  pTmpGui = create_checkbox(pWindow->dst,
  		wakeup_focus, WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = wakeup_focus_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 15;

  add_to_gui_list(ID_OPTIONS_LOCAL_ACTIVE_UNITS_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("Focus on Awakened Units"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 55;

  add_to_gui_list(ID_OPTIONS_LOCAL_ACTIVE_UNITS_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      (pTmpGui->next->size.h - pTmpGui->size.h) / 2;

  /* 'popup new city window' */
  /* check box */
  pTmpGui = create_checkbox(pWindow->dst, popup_new_cities,
			    WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = popup_new_cities_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 15;

  add_to_gui_list(ID_OPTIONS_LOCAL_CITY_CENTER_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("Pop up city dialog for new cities"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);

  pTmpGui->size.x = pWindow->size.x + 55;

  add_to_gui_list(ID_OPTIONS_LOCAL_CITY_CENTER_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      (pTmpGui->next->size.h - pTmpGui->size.h) / 2;

  /* 'popup new city window' */
  /* check box */
  pTmpGui = create_checkbox(pWindow->dst, ask_city_name,
			    WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = ask_city_names_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 15;

  add_to_gui_list(ID_OPTIONS_LOCAL_CITY_CENTER_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("Prompt for city names"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 55;

  add_to_gui_list(ID_OPTIONS_LOCAL_CITY_CENTER_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      (pTmpGui->next->size.h - pTmpGui->size.h) / 2;
  /* 'auto turn done' */

  /* check box */
  pTmpGui = create_checkbox(pWindow->dst,
  		auto_turn_done, WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = auto_turn_done_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 15;

  add_to_gui_list(ID_OPTIONS_LOCAL_END_TURN_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("End Turn when done moving"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 55;

  add_to_gui_list(ID_OPTIONS_LOCAL_END_TURN_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      (pTmpGui->next->size.h - pTmpGui->size.h) / 2;
  /* ------------------------- */

  pOption_Dlg->pBeginOptionsWidgetList = pTmpGui;
  redraw_group(pOption_Dlg->pBeginOptionsWidgetList,
				  pOption_Dlg->pEndOptionsWidgetList, 0);
  flush_rect(pWindow->size);

  return -1;
}

/* ===================================================================== */

/**************************************************************************
  ...
**************************************************************************/
static int map_grid_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  draw_map_grid ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int draw_city_names_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  draw_city_names ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int draw_city_productions_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  draw_city_productions ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int draw_terrain_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  sdl_dirty_rect(pWidget->size);
  draw_terrain ^= 1;
  /* check draw_coastline checkbox state */
  if (get_wstate(pWidget->prev->prev) == WS_NORMAL)
  {
    set_wstate(pWidget->prev->prev , WS_DISABLED);
  }
  else
  {
    set_wstate(pWidget->prev->prev , WS_NORMAL);
  }
  redraw_icon(pWidget->prev->prev);
  sdl_dirty_rect(pWidget->prev->prev->size);
  flush_dirty();
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int draw_coastline_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  draw_coastline ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int draw_specials_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  draw_specials ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int draw_pollution_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  draw_pollution ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int draw_cities_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  draw_cities ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int draw_units_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  draw_units ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int draw_fog_of_war_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  draw_fog_of_war ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int draw_roads_rails_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  draw_roads_rails ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int draw_irrigation_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  draw_irrigation ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int draw_mines_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  draw_mines ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int draw_fortress_airbase_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  draw_fortress_airbase ^= 1;
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int draw_civ3_city_text_style_callback(struct GUI *pWidget)
{
  redraw_icon(pWidget);
  flush_rect(pWidget->size);
  SDL_Client_Flags ^= CF_CIV3_CITY_TEXT_STYLE;
  return -1;
}


/**************************************************************************
  ...
**************************************************************************/
static int map_setting_callback(struct GUI *pWidget)
{
  SDL_Color text_color = {255, 255, 0, 255};
  SDL_String16 *pStr = NULL;
  struct GUI *pTmpGui = NULL, *pWindow = pOption_Dlg->pEndOptionsWidgetList;

  /* clear flag */
  SDL_Client_Flags &= ~CF_OPTION_MAIN;

  /* hide main widget group */
  hide_group(pOption_Dlg->pBeginMainOptionsWidgetList,
	     pOption_Dlg->pBeginCoreOptionsWidgetList->prev);

  /* 'draw map gird' */
  /* check box */
  pTmpGui = create_checkbox(pWindow->dst,
		  draw_map_grid, WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = map_grid_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 15;
  pTmpGui->size.y = pWindow->size.y + WINDOW_TILE_HIGH + 6;

  add_to_gui_list(ID_OPTIONS_MAP_GRID_CHECKBOX, pTmpGui);

  /* 'sound befor new turn' label */
  pStr = create_str16_from_char(_("Map Grid"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 55;

  add_to_gui_list(ID_OPTIONS_MAP_GRID_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      ((pTmpGui->next->size.h - pTmpGui->size.h) / 2);

  /* 'draw city names' */
  /* check box */
  pTmpGui = create_checkbox(pWindow->dst,
		  draw_city_names, WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = draw_city_names_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 15;

  add_to_gui_list(ID_OPTIONS_MAP_CITY_NAMES_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("City Names"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 55;

  add_to_gui_list(ID_OPTIONS_MAP_CITY_NAMES_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      ((pTmpGui->next->size.h - pTmpGui->size.h) / 2);

  /* 'draw city prod.' */
  /* check box */
  pTmpGui = create_checkbox(pWindow->dst, draw_city_productions,
			    WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = draw_city_productions_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 15;

  add_to_gui_list(ID_OPTIONS_MAP_CITY_PROD_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("City Production"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 55;

  add_to_gui_list(ID_OPTIONS_MAP_CITY_NAMES_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      ((pTmpGui->next->size.h - pTmpGui->size.h) / 2);

  /* 'draw terrain' */
  /* check box */
  pTmpGui = create_checkbox(pWindow->dst,
  			draw_terrain, WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = draw_terrain_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 15;

  add_to_gui_list(ID_OPTIONS_MAP_CITY_PROD_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("Terrain"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 55;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      (pTmpGui->next->size.h - pTmpGui->size.h) / 2;

  /* 'draw coast line' */

  /* check box */
  pTmpGui = create_checkbox(pWindow->dst,
  			draw_coastline, WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = draw_coastline_callback;

  if (!draw_terrain) {
    set_wstate(pTmpGui, WS_NORMAL);
  }

  pTmpGui->size.x = pWindow->size.x + 35;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_COAST_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("Coast outline"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 75;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_COAST_LABEL, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->size.y +
      (pTmpGui->next->size.h - pTmpGui->size.h) / 2;

  /* 'draw specials' */

  /* check box */
  pTmpGui = create_checkbox(pWindow->dst,
  			draw_specials, WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = draw_specials_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 15;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_SPEC_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("Special Resources"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 55;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_SPEC_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      (pTmpGui->next->size.h - pTmpGui->size.h) / 2;

  /* 'draw pollutions' */

  /* check box */
  pTmpGui = create_checkbox(pWindow->dst,
			  draw_pollution, WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = draw_pollution_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 15;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_POLL_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("Pollution"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 55;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_POLL_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      (pTmpGui->next->size.h - pTmpGui->size.h) / 2;

  /* 'draw cities' */

  /* check box */
  pTmpGui = create_checkbox(pWindow->dst, 
  				draw_cities, WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = draw_cities_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 15;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_CITY_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("Cities"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 55;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_CITY_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      (pTmpGui->next->size.h - pTmpGui->size.h) / 2;

  /* 'draw units' */

  /* check box */
  pTmpGui = create_checkbox(pWindow->dst,
  			draw_units, WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = draw_units_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 15;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_UNITS_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 3;

  /* label */
  pStr = create_str16_from_char(_("Units"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 55;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_UNITS_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      (pTmpGui->next->size.h - pTmpGui->size.h) / 2;

  /* 'draw fog of war' */

  /* check box */
  pTmpGui = create_checkbox(pWindow->dst,
  			draw_fog_of_war, WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = draw_fog_of_war_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 15;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_FOG_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 3;

  /* label */
  pStr = create_str16_from_char(_("Fog of War"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 55;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_FOG_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      (pTmpGui->next->size.h - pTmpGui->size.h) / 2;

  /* label inpr. */
  pStr = create_str16_from_char(_("Infrastructure"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 185;
  pTmpGui->size.y = pWindow->size.y + WINDOW_TILE_HIGH + 6;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_INPR_LABEL, pTmpGui);
#if 0
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 10;
#endif

  /* 'draw road / rails' */
  /* check box */
  pTmpGui = create_checkbox(pWindow->dst,
  			draw_roads_rails, WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = draw_roads_rails_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 170;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_RR_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->size.y + pTmpGui->next->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("Roads and Rails"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 210;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_RR_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      (pTmpGui->next->size.h - pTmpGui->size.h) / 2;

  /* 'draw irrigations' */
  /* check box */
  pTmpGui = create_checkbox(pWindow->dst, 
  			draw_irrigation, WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = draw_irrigation_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 170;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_IR_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("Irrigation"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 210;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_IR_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      (pTmpGui->next->size.h - pTmpGui->size.h) / 2;

  /* 'draw mines' */

  /* check box */
  pTmpGui = create_checkbox(pWindow->dst,
  			draw_mines, WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = draw_mines_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 170;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_M_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("Mines"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);

  pTmpGui->size.x = pWindow->size.x + 210;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_M_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      (pTmpGui->next->size.h - pTmpGui->size.h) / 2;

  /* 'draw fortress / air bases' */
  /* check box */
  pTmpGui = create_checkbox(pWindow->dst, draw_fortress_airbase,
			    WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = draw_fortress_airbase_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 170;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_FA_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("Fortress and Airbase"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 210;

  add_to_gui_list(ID_OPTIONS_MAP_TERRAIN_FA_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      (pTmpGui->next->size.h - pTmpGui->size.h) / 2;

/* 'Civ3 / Classic CITY Text Style ' */
  /* check box */
  pTmpGui = create_checkbox(pWindow->dst,
  	((SDL_Client_Flags & CF_CIV3_CITY_TEXT_STYLE) > 0),
			    WF_DRAW_THEME_TRANSPARENT);

  pTmpGui->action = draw_civ3_city_text_style_callback;
  set_wstate(pTmpGui, WS_NORMAL);

  pTmpGui->size.x = pWindow->size.x + 170;

  add_to_gui_list(ID_OPTIONS_MAP_CITY_CIV3_TEXT_STYLE_CHECKBOX, pTmpGui);
  pTmpGui->size.y = pTmpGui->next->next->size.y + pTmpGui->size.h + 4;

  /* label */
  pStr = create_str16_from_char(_("Civ3 city text style"), 10);
  pStr->style |= TTF_STYLE_BOLD;
  pStr->forecol = text_color;
  pTmpGui = create_iconlabel(NULL, pWindow->dst, pStr, 0);
  
  pTmpGui->size.x = pWindow->size.x + 210;

  add_to_gui_list(ID_OPTIONS_MAP_CITY_CIV3_TEXT_STYLE_LABEL, pTmpGui);

  pTmpGui->size.y = pTmpGui->next->size.y +
      (pTmpGui->next->size.h - pTmpGui->size.h) / 2;
      
  /* ================================================== */
  
  pOption_Dlg->pBeginOptionsWidgetList = pTmpGui;

  /* redraw window group */
  redraw_group(pOption_Dlg->pBeginOptionsWidgetList,
			  pOption_Dlg->pEndOptionsWidgetList, 0);
  flush_rect(pWindow->size);

  return -1;
}

/* ===================================================================== */

/**************************************************************************
  ...
**************************************************************************/
static int disconnect_callback(struct GUI *pWidget)
{
  if (get_client_state() == CLIENT_PRE_GAME_STATE) {
    SDL_Rect area;
    popdown_window_group_dialog(pOption_Dlg->pBeginOptionsWidgetList,
				pOption_Dlg->pEndOptionsWidgetList);
    SDL_Client_Flags &= ~(CF_OPTION_MAIN | CF_OPTION_OPEN |
			  CF_TOGGLED_FULLSCREEN);
    /* hide buton */
    area = pOptions_Button->size;
    SDL_BlitSurface(pOptions_Button->gfx, NULL, pOptions_Button->dst, &area);
    sdl_dirty_rect(pOptions_Button->size);
    
#if 0
    /* hide "waiting for game start" label */
    pOptions_Button = get_widget_pointer_form_main_list(ID_WAITING_LABEL);
    area = pOptions_Button->size;
    SDL_BlitSurface(pOptions_Button->gfx, NULL, pOptions_Button->dst, &area);
    sdl_dirty_rect(pOptions_Button->size);
#endif    
      
    flush_dirty();
  }

  disconnect_from_server();
  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int back_callback(struct GUI *pWidget)
{

  if(pOption_Dlg->pADlg) {
    FREE(pOption_Dlg->pADlg->pScroll);
    FREE(pOption_Dlg->pADlg);
  }
  if (SDL_Client_Flags & CF_OPTION_MAIN) {
    popdown_window_group_dialog(pOption_Dlg->pBeginOptionsWidgetList,
				pOption_Dlg->pEndOptionsWidgetList);
    SDL_Client_Flags &= ~(CF_OPTION_MAIN | CF_OPTION_OPEN |
			  CF_TOGGLED_FULLSCREEN);
    FREE(pOption_Dlg);
    if(aconnection.established) {
      set_wstate(pOptions_Button, WS_NORMAL);
      redraw_icon(pOptions_Button);
      sdl_dirty_rect(pOptions_Button->size);
      update_menus();
      flush_dirty();
    } else {
      gui_server_connect();
    }
    return -1;
  }

  del_group_of_widgets_from_gui_list(pOption_Dlg->pBeginOptionsWidgetList,
			pOption_Dlg->pBeginMainOptionsWidgetList->prev);

  pOption_Dlg->pBeginOptionsWidgetList =
			  pOption_Dlg->pBeginMainOptionsWidgetList;

  show_group(pOption_Dlg->pBeginOptionsWidgetList,
			  pOption_Dlg->pBeginCoreOptionsWidgetList->prev);

  SDL_Client_Flags |= CF_OPTION_MAIN;
  SDL_Client_Flags &= ~CF_TOGGLED_FULLSCREEN;

  redraw_group(pOption_Dlg->pBeginOptionsWidgetList,
			  pOption_Dlg->pEndOptionsWidgetList, 0);

  flush_rect(pOption_Dlg->pEndOptionsWidgetList->size);

  return -1;
}

/**************************************************************************
  ...
**************************************************************************/
static int optiondlg_callback(struct GUI *pButton)
{
  set_wstate(pButton, WS_DISABLED);
  SDL_FillRect(pButton->dst, &pButton->size, 0x0);
  real_redraw_icon(pButton);
  flush_rect(pButton->size);

  popup_optiondlg();

  return -1;
}

/* ===================================================================== */
/* =================================== Public ========================== */
/* ===================================================================== */

void init_options_button(void)
{
  pOptions_Button = create_themeicon(pTheme->Options_Icon, Main.gui,
				       (WF_WIDGET_HAS_INFO_LABEL |
					WF_DRAW_THEME_TRANSPARENT));

  pOptions_Button->action = optiondlg_callback;
  pOptions_Button->string16 = create_str16_from_char(_("Options"), 12);
  pOptions_Button->key = SDLK_TAB;
  add_to_gui_list(ID_CLIENT_OPTIONS, pOptions_Button);
}

/**************************************************************************
  ...
**************************************************************************/
void popup_optiondlg(void)
{
  struct GUI *pTmp_GUI = NULL;
  Uint16 longest = 0;
  Uint16 w = 350;
  Uint16 h = 300;
  Sint16 start_x = (Main.screen->w - w) / 2;
  Sint16 start_y = (Main.screen->h - h) / 2;
  SDL_Surface *pLogo;

  if(pOption_Dlg) {
    return;
  }
  
  popdown_all_game_dialogs();
  flush_dirty();
  
  if(Main.guis_count) {
    int i;
    for(i=0; i<Main.guis_count; i++)
      if(Main.guis[i])
	printf("Buffers not free = %d \n", i);
  }
  
  pOption_Dlg = MALLOC(sizeof(struct OPT_DLG));
  pOption_Dlg->pADlg = NULL;
  pLogo = get_logo_gfx();
  
  /* create window widget */
  pTmp_GUI =
      create_window(Main.gui, create_str16_from_char(_("Options"), 12), w, h, 0);
  	//create_window(NULL, create_str16_from_char(_("Options"), 12), w, h, 0);
  pTmp_GUI->string16->style |= TTF_STYLE_BOLD;
  pTmp_GUI->size.x = start_x;
  pTmp_GUI->size.y = start_y;
  pTmp_GUI->action = main_optiondlg_callback;

  if (resize_window(pTmp_GUI, pLogo, NULL, w, h)) {
    FREESURFACE(pLogo);
  }

  set_wstate(pTmp_GUI, WS_NORMAL);
  add_to_gui_list(ID_OPTIONS_WINDOW, pTmp_GUI);
  pOption_Dlg->pEndOptionsWidgetList = pTmp_GUI;

  /* create exit button */
  pTmp_GUI = create_themeicon_button_from_chars(pTheme->CANCEL_Icon,
				pTmp_GUI->dst, _("Quit"), 12, 0);
  pTmp_GUI->size.x = start_x + w - pTmp_GUI->size.w - 10;
  pTmp_GUI->size.y = start_y + h - pTmp_GUI->size.h - 10;
  /* pTmp_GUI->action = exit_callback; */
  pTmp_GUI->key = SDLK_q;
  set_wstate(pTmp_GUI, WS_NORMAL);
  add_to_gui_list(ID_OPTIONS_EXIT_BUTTON, pTmp_GUI);

  /* create disconnection button */
  if(aconnection.established) {
    pTmp_GUI = create_themeicon_button_from_chars(pTheme->BACK_Icon,
				pTmp_GUI->dst, _("Disconnect"), 12, 0);
    pTmp_GUI->size.x = start_x + (w - pTmp_GUI->size.w) / 2;
    pTmp_GUI->size.y = start_y + h - pTmp_GUI->size.h - 10;
    pTmp_GUI->action = disconnect_callback;
    set_wstate(pTmp_GUI, WS_NORMAL);
    add_to_gui_list(ID_OPTIONS_DISC_BUTTON, pTmp_GUI);
  }
  
  /* create back button */
  pTmp_GUI = create_themeicon_button_from_chars(pTheme->BACK_Icon,
				pTmp_GUI->dst, _("Back"), 12, 0);
  pTmp_GUI->size.x = start_x + 10;
  pTmp_GUI->size.y = start_y + h - pTmp_GUI->size.h - 10;
  pTmp_GUI->action = back_callback;
  pTmp_GUI->key = SDLK_ESCAPE;
  set_wstate(pTmp_GUI, WS_NORMAL);
  add_to_gui_list(ID_OPTIONS_BACK_BUTTON, pTmp_GUI);
  pOption_Dlg->pBeginCoreOptionsWidgetList = pTmp_GUI;

  /* ============================================================= */

  /* create video button widget */
  pTmp_GUI = create_icon_button_from_chars(NULL,
			pTmp_GUI->dst, _("Video options"), 12, 0);
  pTmp_GUI->size.y = start_y + 60;
  pTmp_GUI->action = video_callback;
  set_wstate(pTmp_GUI, WS_NORMAL);
  pTmp_GUI->size.h += 4;

  longest = MAX(longest, pTmp_GUI->size.w);

  add_to_gui_list(ID_OPTIONS_VIDEO_BUTTON, pTmp_GUI);

  /* create sound button widget */
  pTmp_GUI = create_icon_button_from_chars(NULL,
				pTmp_GUI->dst, _("Sound options"), 12, 0);
  pTmp_GUI->size.y = start_y + 90;
  pTmp_GUI->action = sound_callback;
  /* set_wstate( pTmp_GUI, WS_NORMAL ); */
  pTmp_GUI->size.h += 4;
  longest = MAX(longest, pTmp_GUI->size.w);

  add_to_gui_list(ID_OPTIONS_SOUND_BUTTON, pTmp_GUI);


  /* create local button widget */
  pTmp_GUI =
      create_icon_button_from_chars(NULL, pTmp_GUI->dst,
				      _("Game options"), 12, 0);
  pTmp_GUI->size.y = start_y + 120;
  pTmp_GUI->action = local_setting_callback;
  set_wstate(pTmp_GUI, WS_NORMAL);
  pTmp_GUI->size.h += 4;
  longest = MAX(longest, pTmp_GUI->size.w);

  add_to_gui_list(ID_OPTIONS_LOCAL_BUTTON, pTmp_GUI);

  /* create map button widget */
  pTmp_GUI = create_icon_button_from_chars(NULL,
				  pTmp_GUI->dst, _("Map options"), 12, 0);
  pTmp_GUI->size.y = start_y + 150;
  pTmp_GUI->action = map_setting_callback;
  set_wstate(pTmp_GUI, WS_NORMAL);
  pTmp_GUI->size.h += 4;
  longest = MAX(longest, pTmp_GUI->size.w);

  add_to_gui_list(ID_OPTIONS_MAP_BUTTON, pTmp_GUI);


  /* create work lists widget */
  pTmp_GUI = create_icon_button_from_chars(NULL, 
  				pTmp_GUI->dst, _("Worklists"), 12, 0);
  pTmp_GUI->size.y = start_y + 180;
  pTmp_GUI->action = work_lists_callback;
  set_wstate(pTmp_GUI, WS_NORMAL);

  pTmp_GUI->size.h += 4;
  longest = MAX(longest, pTmp_GUI->size.w);

  add_to_gui_list(ID_OPTIONS_WORKLIST_BUTTON, pTmp_GUI);

  pOption_Dlg->pBeginOptionsWidgetList = pTmp_GUI;
  pOption_Dlg->pBeginMainOptionsWidgetList = pTmp_GUI;

  /* seting witdth and stat x */
  do {
    pTmp_GUI->size.w = longest;
    pTmp_GUI->size.x = start_x + (w - pTmp_GUI->size.w) / 2;

    pTmp_GUI = pTmp_GUI->next;
  } while (pTmp_GUI != pOption_Dlg->pBeginCoreOptionsWidgetList);

  /* draw window group */
  redraw_group(pOption_Dlg->pBeginOptionsWidgetList,
  			pOption_Dlg->pEndOptionsWidgetList, 0);

  flush_rect(pOption_Dlg->pEndOptionsWidgetList->size);

  SDL_Client_Flags |= (CF_OPTION_MAIN | CF_OPTION_OPEN);
  SDL_Client_Flags &= ~CF_TOGGLED_FULLSCREEN;

  update_menus();
}

/**************************************************************************
  ...
**************************************************************************/
void podown_optiondlg(void)
{
  if (pOption_Dlg) {
    popdown_window_group_dialog(pOption_Dlg->pBeginOptionsWidgetList,
				pOption_Dlg->pEndOptionsWidgetList);
    SDL_Client_Flags &= ~(CF_OPTION_MAIN | CF_OPTION_OPEN |
			  CF_TOGGLED_FULLSCREEN);
    FREE(pOption_Dlg);
    flush_dirty();
  }
}
