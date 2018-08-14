// file: storage_engine.h

/**
 *  Copyright (C) 2018 Marcus Pinnecke
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _NG5_STORAGE_ENGINE
#define _NG5_STORAGE_ENGINE

#include <common.h>

enum storage_engine_tag {
    STORAGE_ENGINE_TAG_ROADFIRE
};


typedef uint64_t resource_id_t;

struct IRI
{
  const char        *path;
};


struct storage_engine
{
    void            *extra;

    int              create_resources(struct resource_id_t **resources, size_t num_resources);

    int              drop_resources(const struct resource_id_t *resources, size_t num_resources);

    int

};

#endif
