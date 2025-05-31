//----------------------------------------------------------------------------
//  EDGE Movement, Collision & Blockmap utility functions
//----------------------------------------------------------------------------
//
//  Copyright (c) 1999-2024 The EDGE Team.
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
//
//  Based on the DOOM source code, released by Id Software under the
//  following copyright:
//
//    Copyright (C) 1993-1996 by id Software, Inc.
//
//----------------------------------------------------------------------------
//
// DESCRIPTION:
//   Movement/collision utility functions,
//   as used by function in p_map.c.
//   BLOCKMAP Iterator functions,
//   and some PIT_* functions to use for iteration.
//   Gap utility functions.
//   Touch Node code.
//
//

#include <float.h>

#include <algorithm>
#include <vector>

#include "AlmostEquals.h"
#include "dm_defs.h"
#include "dm_state.h"
#include "epi.h"
#include "epi_doomdefs.h"
#include "m_bbox.h"
#include "p_local.h"
#include "p_spec.h"
#include "r_state.h"

extern unsigned int root_node;

//
// ApproximateDistance
//
// Gives an estimation of distance (not exact)
//
float ApproximateDistance(float dx, float dy)
{
    dx = fabs(dx);
    dy = fabs(dy);

    return (dy > dx) ? dy + dx / 2 : dx + dy / 2;
}

float ApproximateDistance(float dx, float dy, float dz)
{
    dx = fabs(dx);
    dy = fabs(dy);
    dz = fabs(dz);

    float dxy = (dy > dx) ? dy + dx / 2 : dx + dy / 2;

    return (dz > dxy) ? dz + dxy / 2 : dxy + dz / 2;
}

//
// ApproximateSlope
//
// Gives an estimation of slope (not exact)
//
// -AJA- 1999/09/11: written.
//
float ApproximateSlope(float dx, float dy, float dz)
{
    float dist = ApproximateDistance(dx, dy);

    // kludge to prevent overflow or division by zero.
    if (dist < 1.0f / 32.0f)
        dist = 1.0f / 32.0f;

    return dz / dist;
}

void ComputeIntersection(DividingLine *div, float x1, float y1, float x2, float y2, float *ix, float *iy)
{
    if (AlmostEquals(div->delta_x, 0.0f))
    {
        *ix = div->x;
        *iy = y1 + (y2 - y1) * (div->x - x1) / (x2 - x1);
    }
    else if (AlmostEquals(div->delta_y, 0.0f))
    {
        *iy = div->y;
        *ix = x1 + (x2 - x1) * (div->y - y1) / (y2 - y1);
    }
    else
    {
        // perpendicular distances (unnormalised)
        float p1 = (x1 - div->x) * div->delta_y - (y1 - div->y) * div->delta_x;
        float p2 = (x2 - div->x) * div->delta_y - (y2 - div->y) * div->delta_x;

        *ix = x1 + (x2 - x1) * p1 / (p1 - p2);
        *iy = y1 + (y2 - y1) * p1 / (p1 - p2);
    }
}

//
// PointOnDividingLineSide
//
// Tests which side of the line the given point lies on.
// Returns 0 (front/right) or 1 (back/left).  If the point lies
// directly on the line, result is undefined (either 0 or 1).
//
int PointOnDividingLineSide(float x, float y, DividingLine *div)
{
    float dx, dy;
    float left, right;

    if (AlmostEquals(div->delta_x, 0.0f))
        return ((x <= div->x) ^ (div->delta_y > 0)) ? 0 : 1;

    if (AlmostEquals(div->delta_y, 0.0f))
        return ((y <= div->y) ^ (div->delta_x < 0)) ? 0 : 1;

    dx = x - div->x;
    dy = y - div->y;

    // try to quickly decide by looking at sign bits
    if ((div->delta_y < 0) ^ (div->delta_x < 0) ^ (dx < 0) ^ (dy < 0))
    {
        // left is negative
        if ((div->delta_y < 0) ^ (dx < 0))
            return 1;

        return 0;
    }

    left  = dx * div->delta_y;
    right = dy * div->delta_x;

    return (right < left) ? 0 : 1;
}

//
// PointOnDividingLineThick
//
// Tests which side of the line the given point is on.   The thickness
// parameter determines when the point is considered "on" the line.
// Returns 0 (front/right), 1 (back/left), or 2 (on).
//
int PointOnDividingLineThick(float x, float y, DividingLine *div, float div_len, float thickness)
{
    float dx, dy;
    float left, right;

    if (AlmostEquals(div->delta_x, 0.0f))
    {
        if (fabs(x - div->x) <= thickness)
            return 2;

        return ((x < div->x) ^ (div->delta_y > 0)) ? 0 : 1;
    }

    if (AlmostEquals(div->delta_y, 0.0f))
    {
        if (fabs(y - div->y) <= thickness)
            return 2;

        return ((y < div->y) ^ (div->delta_x < 0)) ? 0 : 1;
    }

    dx = x - div->x;
    dy = y - div->y;

    // need divline's length here to compute proper distances
    left  = (dx * div->delta_y) / div_len;
    right = (dy * div->delta_x) / div_len;

    if (fabs(left - right) < thickness)
        return 2;

    return (right < left) ? 0 : 1;
}

//
// BoxOnLineSide
//
// Considers the line to be infinite
// Returns side 0 or 1, -1 if box crosses the line.
//
int BoxOnLineSide(const float *tmbox, Line *ld)
{
    int p1 = 0;
    int p2 = 0;

    DividingLine div;

    div.x       = ld->vertex_1->X;
    div.y       = ld->vertex_1->Y;
    div.delta_x = ld->delta_x;
    div.delta_y = ld->delta_y;

    switch (ld->slope_type)
    {
    case kLineClipHorizontal:
        p1 = tmbox[kBoundingBoxTop] > ld->vertex_1->Y;
        p2 = tmbox[kBoundingBoxBottom] > ld->vertex_1->Y;
        if (ld->delta_x < 0)
        {
            p1 ^= 1;
            p2 ^= 1;
        }
        break;

    case kLineClipVertical:
        p1 = tmbox[kBoundingBoxRight] < ld->vertex_1->X;
        p2 = tmbox[kBoundingBoxLeft] < ld->vertex_1->X;
        if (ld->delta_y < 0)
        {
            p1 ^= 1;
            p2 ^= 1;
        }
        break;

    case kLineClipPositive:
        p1 = PointOnDividingLineSide(tmbox[kBoundingBoxLeft], tmbox[kBoundingBoxTop], &div);
        p2 = PointOnDividingLineSide(tmbox[kBoundingBoxRight], tmbox[kBoundingBoxBottom], &div);
        break;

    case kLineClipNegative:
        p1 = PointOnDividingLineSide(tmbox[kBoundingBoxRight], tmbox[kBoundingBoxTop], &div);
        p2 = PointOnDividingLineSide(tmbox[kBoundingBoxLeft], tmbox[kBoundingBoxBottom], &div);
        break;
    }

    if (p1 == p2)
        return p1;

    return -1;
}

//
// BoxOnDividingLineSide
//
// Considers the line to be infinite
// Returns side 0 or 1, -1 if box crosses the line.
//
int BoxOnDividingLineSide(const float *tmbox, DividingLine *div)
{
    int p1 = 0;
    int p2 = 0;

    if (AlmostEquals(div->delta_y, 0.0f))
    {
        p1 = tmbox[kBoundingBoxTop] > div->y;
        p2 = tmbox[kBoundingBoxBottom] > div->y;

        if (div->delta_x < 0)
        {
            p1 ^= 1;
            p2 ^= 1;
        }
    }
    else if (AlmostEquals(div->delta_x, 0.0f))
    {
        p1 = tmbox[kBoundingBoxRight] < div->x;
        p2 = tmbox[kBoundingBoxLeft] < div->x;

        if (div->delta_y < 0)
        {
            p1 ^= 1;
            p2 ^= 1;
        }
    }
    else if (div->delta_y / div->delta_x > 0) // optimise ?
    {
        p1 = PointOnDividingLineSide(tmbox[kBoundingBoxLeft], tmbox[kBoundingBoxTop], div);
        p2 = PointOnDividingLineSide(tmbox[kBoundingBoxRight], tmbox[kBoundingBoxBottom], div);
    }
    else
    {
        p1 = PointOnDividingLineSide(tmbox[kBoundingBoxRight], tmbox[kBoundingBoxTop], div);
        p2 = PointOnDividingLineSide(tmbox[kBoundingBoxLeft], tmbox[kBoundingBoxBottom], div);
    }

    if (p1 == p2)
        return p1;

    return -1;
}

int ThingOnLineSide(const MapObject *mo, Line *ld)
{
    float bbox[4];

    bbox[kBoundingBoxLeft]   = mo->x - mo->radius_;
    bbox[kBoundingBoxRight]  = mo->x + mo->radius_;
    bbox[kBoundingBoxBottom] = mo->y - mo->radius_;
    bbox[kBoundingBoxTop]    = mo->y + mo->radius_;

    return BoxOnLineSide(bbox, ld);
}

//------------------------------------------------------------------------
//
//  GAP UTILITY FUNCTIONS
//

static bool GapConstruct(VerticalGap *gap, Sector *sec, MapObject *thing, float floor_slope_z = 0,
                         float ceiling_slope_z = 0)
{
    EPI_UNUSED(thing); // Maybe for WaterWalker + new swimmable liquid? - Dasho

    // early out for FUBAR sectors
    if (sec->floor_height >= sec->ceiling_height)
        return false;

    gap->floor   = sec->floor_height + floor_slope_z;
    gap->ceiling = sec->ceiling_height - ceiling_slope_z;

    return true;
}

static bool GapSightConstruct(VerticalGap *gap, Sector *sec)
{
    // early out for closed or FUBAR sectors
    if (sec->ceiling_height <= sec->floor_height)
        return false;

    gap->floor   = sec->floor_height;
    gap->ceiling = sec->ceiling_height;

    return true;
}

static bool GapRestrict(VerticalGap *dest, bool d_gap, VerticalGap *src, bool s_gap)
{
    bool        has_new_gap = false;
    VerticalGap new_gap;

    if (s_gap)
    {
        // ignore empty gaps.
        if (src->ceiling <= src->floor)
        {
            // nothing
        }
        else if (d_gap)
        {
            // ignore empty gaps.
            if (dest->ceiling <= dest->floor)
            {
                // nothing
            }
            else
            {
                float f = HMM_MAX(src->floor, dest->floor);
                float c = HMM_MIN(src->ceiling, dest->ceiling);

                if (f < c)
                {
                    new_gap.ceiling = c;
                    new_gap.floor   = f;
                    has_new_gap     = true;
                }
            }
        }
    }

    if (has_new_gap)
        memmove(dest, &new_gap, sizeof(VerticalGap));

    return has_new_gap;
}

//
// ComputeThingGap
//
// Determine the initial floorz and ceilingz that a thing placed at a
// particular Z would have.  Returns the nominal Z height.  Some special
// values of Z are recognised: kOnFloorZ & kOnCeilingZ.
//
float ComputeThingGap(MapObject *thing, Sector *sec, float z, float *f, float *c, float floor_slope_z,
                      float ceiling_slope_z)
{
    VerticalGap temp_gap;
    int         temp_num;

    temp_num = GapConstruct(&temp_gap, sec, thing, floor_slope_z, ceiling_slope_z);

    if (AlmostEquals(z, kOnFloorZ))
        z = sec->floor_height + floor_slope_z;

    if (AlmostEquals(z, kOnCeilingZ))
        z = sec->ceiling_height - thing->height_ - ceiling_slope_z;

    if (temp_num == 0)
    {
        // thing is stuck in a closed door.
        *f = *c = sec->floor_height;
        return *f;
    }

    *f = temp_gap.floor;
    *c = temp_gap.ceiling;

    return z;
}

//
// ComputeGaps
//
// Determine the gaps between the front & back sectors of the line,
// taking into account any extra floors.
//
// -AJA- 1999/07/19: This replaces P_LineOpening.
//
void ComputeGaps(Line *ld)
{
    Sector *front = ld->front_sector;
    Sector *back  = ld->back_sector;

    bool        has_temp_gap;
    VerticalGap temp_gap;

    ld->blocked = true;
    ld->has_gap = false;

    if (!front || !back)
    {
        // single sided line
        return;
    }

    // NOTE: this check is rather lax.  It mirrors the check in original
    // Doom r_bsp.c, in order for transparent doors to work properly.
    // In particular, the blocked flag can be clear even when one of the
    // sectors is closed (has ceiling <= floor).

    if (back->ceiling_height <= front->floor_height || front->ceiling_height <= back->floor_height)
    {
        // closed door.

        // -AJA- MUNDO HACK for slopes (and line 242)!!!!
        if (front->floor_slope || back->floor_slope || front->ceiling_slope || back->ceiling_slope ||
            front->height_sector || back->height_sector)
        {
            ld->blocked = false;
        }

        return;
    }

    // FIXME: strictly speaking this is not correct, as the front or
    // back sector may be filled up with thick opaque extrafloors.

    // TODO Dasho - Should this still hold true with extrafloors gone?
    ld->blocked = false;

    // handle horizontal sliders
    if (ld->slide_door)
    {
        SlidingDoorMover *smov = ld->slider_move;

        if (!smov)
            return;

        // these semantics copied from XDoom
        if (smov->direction > 0 && smov->opening < smov->target * 0.5f)
            return;

        if (smov->direction < 0 && smov->opening < smov->target * 0.75f)
            return;
    }

    // handle normal gaps ("movement" gaps)

    ld->has_gap  = GapConstruct(ld->gap, front, nullptr);
    has_temp_gap = GapConstruct(&temp_gap, back, nullptr);

    ld->has_gap = GapRestrict(ld->gap, ld->has_gap, &temp_gap, has_temp_gap);
}

void RecomputeGapsAroundSector(Sector *sec)
{
    int i;

    for (i = 0; i < sec->line_count; i++)
    {
        ComputeGaps(sec->lines[i]);
    }

    // now do the sight gaps...

    if (sec->ceiling_height <= sec->floor_height)
    {
        sec->has_sight_gap = false;
        return;
    }

    sec->has_sight_gap = GapSightConstruct(sec->sight_gap, sec);
}

static inline bool CheckBoundingBoxOverlap(float *bspcoord, float *test)
{
    return (test[kBoundingBoxRight] < bspcoord[kBoundingBoxLeft] ||
            test[kBoundingBoxLeft] > bspcoord[kBoundingBoxRight] ||
            test[kBoundingBoxTop] < bspcoord[kBoundingBoxBottom] ||
            test[kBoundingBoxBottom] > bspcoord[kBoundingBoxTop])
               ? false
               : true;
}

static bool TraverseSubsector(unsigned int bspnum, float *bbox, bool (*func)(MapObject *mo))
{
    Subsector *sub;
    BSPNode   *node;
    MapObject *obj;

    // just a normal node ?
    if (!(bspnum & kLeafSubsector))
    {
        node = level_nodes + bspnum;

        // recursively check the children nodes
        // OPTIMISE: check against partition lines instead of bboxes.

        if (CheckBoundingBoxOverlap(node->bounding_boxes[0], bbox))
        {
            if (!TraverseSubsector(node->children[0], bbox, func))
                return false;
        }

        if (CheckBoundingBoxOverlap(node->bounding_boxes[1], bbox))
        {
            if (!TraverseSubsector(node->children[1], bbox, func))
                return false;
        }

        return true;
    }

    // the sharp end: check all things in the subsector

    sub = level_subsectors + (bspnum & ~kLeafSubsector);

    for (obj = sub->thing_list; obj; obj = obj->subsector_next_)
    {
        if (!(*func)(obj))
            return false;
    }

    return true;
}

//
// SubsectorThingIterator
//
// Iterate over all things that touch a certain rectangle on the map,
// using the BSP tree.
//
// If any function returns false, then this routine returns false and
// nothing else is checked.  Otherwise true is returned.
//
bool SubsectorThingIterator(float *bbox, bool (*func)(MapObject *mo))
{
    return TraverseSubsector(root_node, bbox, func);
}

static float checkempty_bbox[4];
static Line *checkempty_line;

static bool CheckThingInArea(MapObject *mo)
{
    if (mo->x + mo->radius_ < checkempty_bbox[kBoundingBoxLeft] ||
        mo->x - mo->radius_ > checkempty_bbox[kBoundingBoxRight] ||
        mo->y + mo->radius_ < checkempty_bbox[kBoundingBoxBottom] ||
        mo->y - mo->radius_ > checkempty_bbox[kBoundingBoxTop])
    {
        // keep looking
        return true;
    }

    // ignore corpses and pickup items
    if (!(mo->flags_ & kMapObjectFlagSolid) && (mo->flags_ & kMapObjectFlagCorpse))
        return true;

    if (mo->flags_ & kMapObjectFlagSpecial)
        return true;

    // we've found a thing in that area: we can stop now
    return false;
}

static bool CheckThingOnLine(MapObject *mo)
{
    float bbox[4];
    int   side;

    bbox[kBoundingBoxLeft]   = mo->x - mo->radius_;
    bbox[kBoundingBoxRight]  = mo->x + mo->radius_;
    bbox[kBoundingBoxBottom] = mo->y - mo->radius_;
    bbox[kBoundingBoxTop]    = mo->y + mo->radius_;

    // found a thing on the line ?
    side = BoxOnLineSide(bbox, checkempty_line);

    if (side != -1)
        return true;

    // ignore corpses and pickup items
    if (!(mo->flags_ & kMapObjectFlagSolid) && (mo->flags_ & kMapObjectFlagCorpse))
        return true;

    if (mo->flags_ & kMapObjectFlagSpecial)
        return true;

    return false;
}

//
// CheckAreaForThings
//
// Checks if there are any things contained within the given rectangle
// on the 2D map.
//
bool CheckAreaForThings(float *bbox)
{
    checkempty_bbox[kBoundingBoxLeft]   = bbox[kBoundingBoxLeft];
    checkempty_bbox[kBoundingBoxRight]  = bbox[kBoundingBoxRight];
    checkempty_bbox[kBoundingBoxBottom] = bbox[kBoundingBoxBottom];
    checkempty_bbox[kBoundingBoxTop]    = bbox[kBoundingBoxTop];

    return !SubsectorThingIterator(bbox, CheckThingInArea);
}

//
// CheckSliderPathForThings
//
//
bool CheckSliderPathForThings(Line *ld)
{
    Line *temp_line = new Line;
    memcpy(temp_line, ld, sizeof(Line));
    temp_line->bounding_box[kBoundingBoxLeft] -= 32;
    temp_line->bounding_box[kBoundingBoxRight] += 32;
    temp_line->bounding_box[kBoundingBoxBottom] -= 32;
    temp_line->bounding_box[kBoundingBoxTop] += 32;

    checkempty_line = temp_line;

    bool slider_check = SubsectorThingIterator(temp_line->bounding_box, CheckThingOnLine);

    delete temp_line;
    temp_line = nullptr;

    return !slider_check;
}

//--- editor settings ---
// vi:ts=4:sw=4:noexpandtab
