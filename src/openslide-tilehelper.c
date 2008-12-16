/*
 *  OpenSlide, a library for reading whole slide image files
 *
 *  Copyright (c) 2007-2008 Carnegie Mellon University
 *  All rights reserved.
 *
 *  OpenSlide is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2.
 *
 *  OpenSlide is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with OpenSlide. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Linking OpenSlide statically or dynamically with other modules is
 *  making a combined work based on OpenSlide. Thus, the terms and
 *  conditions of the GNU General Public License cover the whole
 *  combination.
 */

#include "openslide-tilehelper.h"

#include <glib.h>

static void copy_tile(const uint32_t *tile,
		      uint32_t *dest,
		      int64_t src_w, int64_t src_h,
		      int64_t dest_origin_x, int64_t dest_origin_y,
		      int64_t dest_w, int64_t dest_h) {
  int64_t src_origin_y;
  if (dest_origin_y < 0) {  // off the top
    src_origin_y = -dest_origin_y;
  } else {
    src_origin_y = 0;
  }

  //  g_debug("src_origin_y: %d, dest_origin_y: %d", src_origin_y, dest_origin_y);

  int64_t src_origin_x;
  if (dest_origin_x < 0) {  // off the left
    src_origin_x = -dest_origin_x;
  } else {
    src_origin_x = 0;
  }

  //  g_debug("src_origin_x: %d, dest_origin_x: %d", src_origin_x, dest_origin_x);

  //  g_debug("");

  int64_t src_y = src_origin_y;
  int64_t dest_y = dest_origin_y + src_y;

  while ((src_y < src_h) && (dest_y < dest_h)) {
    int64_t src_x = src_origin_x;
    int64_t dest_x = dest_origin_x + src_x;

    while((src_x < src_w) && (dest_x < dest_w)) {
      int64_t dest_i = dest_y * dest_w + dest_x;
      int64_t i = src_y * src_w + src_x;
      dest[dest_i] = tile[i];

      src_x++;
      dest_x++;
    }

    src_y++;
    dest_y++;
  }
}


void _openslide_read_tiles(int64_t start_x, int64_t start_y, int64_t end_x, int64_t end_y,
			   int32_t ovr_x, int32_t ovr_y,
			   int64_t dest_w, int64_t dest_h,
			   int32_t layer,
			   int64_t tw, int64_t th,
			   void (*tilereader_read)(void *tilereader_data,
						   uint32_t *dest, int64_t x, int64_t y),
			   void *tilereader_data,
			   uint32_t *dest,
			   struct _openslide_cache *cache) {
  int tile_size = tw * th * 4;

  int num_tiles_decoded = 0;

  int64_t src_y = start_y;
  int64_t dst_y = 0;

  while (src_y < ((end_y / th) + 1) * th) {
    int64_t src_x = start_x;
    int64_t dst_x = 0;

    while (src_x < ((end_x / tw) + 1) * tw) {
      int round_x = (src_x / tw) * tw;
      int round_y = (src_y / th) * th;
      int off_x = src_x - round_x;
      int off_y = src_y - round_y;

      //      g_debug("going to readRGBA @ %d,%d", round_x, round_y);
      //      g_debug(" offset: %d,%d", off_x, off_y);
      uint32_t *cache_tile = _openslide_cache_get(cache, round_x, round_y, layer);
      uint32_t *new_tile = NULL;
      if (cache_tile != NULL) {
	// use cached tile
	copy_tile(cache_tile, dest, tw, th, dst_x - off_x, dst_y - off_y, dest_w, dest_h);
      } else {
	// make new tile
	new_tile = g_slice_alloc(tile_size);
	tilereader_read(tilereader_data, new_tile, round_x, round_y);
	num_tiles_decoded++;

	copy_tile(new_tile, dest, tw, th, dst_x - off_x, dst_y - off_y, dest_w, dest_h);
      }

      if (new_tile != NULL) {
	// if not cached already, store into the cache
	_openslide_cache_put(cache, round_x, round_y, layer, new_tile, tile_size);
      }

      src_x += tw;
      dst_x += tw - ovr_x;
    }

    src_y += th;
    dst_y += th - ovr_y;
  }

  //g_debug("tiles decoded: %d", num_tiles_decoded);
}