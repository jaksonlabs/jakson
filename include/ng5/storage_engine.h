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

typedef uint64_t prop_id_t;

typedef uint64_t relation_id_t;

struct IRI
{
  const char        *path;
};

enum rdf_builtin_type { RDF_BUILTIN_TYPE_INT, RDF_BUILTIN_TYPE_STRING };

struct storage_engine
{
    void            *extra;

    int              create_resources(resource_id_t *out, const struct IRI *iri);

    int              create_blank_resources(resource_id_t *out);

    int              drop_resources(resource_id_t out);

    int              find_resource_by_IRI(resource_id_t *out, const struct IRI *iri);

    int              get_IRI_of_resource(const struct IRI *out, resource_id_t id);

    int              create_property(prop_id_t *out, resource_id_t subject, const struct IRI *predicate_iri,
                                     enum rdf_builtin_type type, const void *literal);

    int              create_relationship(relation_id_t *out, resource_id_t subject, const struct IRI *relationship_iri,
                                         resource_id_t object);

};

#endif
