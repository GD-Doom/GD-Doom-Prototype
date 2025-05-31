//----------------------------------------------------------------------------
//  EDGE Doom/PC Speaker Sound Loader
//----------------------------------------------------------------------------
//
//  Copyright (c) 2024 The EDGE Team.
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//----------------------------------------------------------------------------

#include "s_doom.h"

#include "epi.h"
#include "epi_endian.h"
#include "epi_file.h"
#include "epi_filesystem.h"
#include "s_blit.h"
#include "s_cache.h"
#include "snd_gather.h"
#include "w_wad.h"

extern int sound_device_frequency;

bool LoadDoomSound(SoundData *buf, const uint8_t *data, int length)
{
    if (data[0] != 0x3)
        return false;

    buf->frequency_ = data[2] + (data[3] << 8);

    if (buf->frequency_ < 8000 || buf->frequency_ > 48000)
        LogWarning("Sound Load: weird frequency: %d Hz\n", buf->frequency_);

    if (buf->frequency_ < 4000)
        buf->frequency_ = 4000;

    length -= 8;

    if (length <= 0)
        return false;

    buf->Allocate(length);

    // convert to signed 16-bit format
    const uint8_t *src   = data + 8;
    const uint8_t *s_end = src + length;

    float *dest = buf->data_;
    float  out  = 0;

    for (; src < s_end; src++)
    {
        ma_pcm_u8_to_f32(&out, src, 1, ma_dither_mode_none);
        *dest++ = out;
        *dest++ = out;
    }

    return true;
}

//--- editor settings ---
// vi:ts=4:sw=4:noexpandtab
