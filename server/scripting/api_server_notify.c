/*****************************************************************************
 Freeciv - Copyright (C) 2005 - The Freeciv Project
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*****************************************************************************/

#ifdef HAVE_CONFIG_H
#include <fc_config.h>
#endif

/* common */
#include "featured_text.h"

/* common/scriptcore */
#include "luascript.h"
#include "packets.h"

/* server */
#include "notify.h"
#include "triggers.h"

#include "api_server_notify.h"


/*****************************************************************************
  Notify players which have embassies with pplayer with the given message.
*****************************************************************************/
void api_notify_embassies_msg(lua_State *L, Player *pplayer, Tile *ptile,
                              int event, const char *message)
{
  LUASCRIPT_CHECK_STATE(L);

  notify_embassies(pplayer, NULL, ptile, event, ftc_any, "%s", message);
}

/*****************************************************************************
  Notify pplayer of a complex event.
*****************************************************************************/
void api_notify_event_msg(lua_State *L, Player *pplayer, Tile *ptile,
                          int event, const char *message)
{
  LUASCRIPT_CHECK_STATE(L);

  notify_player(pplayer, ptile, event, ftc_any, "%s", message);
}

/*****************************************************************************
  Send a manual trigger to a player.
*****************************************************************************/
void api_notify_trigger(lua_State *L, Player *pplayer, const char *name)
{
  //trigger_by_name(pplayer, name);
}
