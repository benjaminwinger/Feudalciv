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
                          gui_dither.h  -  description
                             -------------------
    begin                : Mon Jul 15 2002
    copyright            : (C) 2002 by Rafał Bursig
    email                : Rafał Bursig <bursig@poczta.fm>
 **********************************************************************/

#ifndef FC__GUI_DITHER_H
#define FC__GUI_DITHER_H

void dither_surface(SDL_Surface * pDither, SDL_Surface * pMask,
                    SDL_Surface * pDest,
                    int mask_offset_x, int mask_offset_y);

void dither_north(SDL_Surface *pDither,
		  SDL_Surface *pMask, SDL_Surface *pDest);
void dither_south(SDL_Surface *pDither,
		  SDL_Surface *pMask, SDL_Surface *pDest);
void dither_east(SDL_Surface *pDither,
		 SDL_Surface *pMask, SDL_Surface *pDest);
void dither_west(SDL_Surface *pDither,
		 SDL_Surface *pMask, SDL_Surface *pDest);

#endif
