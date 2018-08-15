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
#include <stdx/string_dic.h>

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

struct resource_cursor {

};

struct resource {

};

typedef bool (*resource_pred_func)(struct resource *resource, void *capture);

struct prop_cursor {

};

struct property {

};

typedef bool (*prop_pred_func)(struct property *prop, void *capture);


struct relationship_cursor {

};

struct relationship {

};

typedef bool (*relationship_pred_func)(struct relationship *relationship, void *capture);


struct string_cursor {

};

struct string {

};

typedef bool (*string_pred_func)(struct relationship *string, void *capture);

typedef uint64_t statment_id_t;

struct statement {

};


struct statement_cursor {

};

typedef bool (*statement_prop_pred_func)(struct statement *statement, void *capture);

typedef bool (*statement_relationship_pred_func)(struct statement *statement, void *capture);

enum comp_pred { COMP_LESS, COMP_LESSEQ, COMP_EQUAL, COMP_UNEQUAL, COMP_GREATEREQ, COMP_GREATER };

enum rdf_property { RDF_PROPERTY_CLASS };

struct vector_cursor {

};

enum rdf_object_type { RDF_OBJECT_TYPE_RESOURCE, RDF_OBJECT_TYPE_IRI };

struct thread_conf {
    size_t offset;
};

struct compressed_string {

};

typedef int (*pred_func_t)(bool *result, const void *begin, size_t num_elems, enum rdf_object_type type,
                           struct storage_engine *engine, struct thread_conf *thread);

struct storage_engine
{
    void *extra;

    struct string_dic dictionary;

    /**
     * Imports <code>strings</code> into the database by applying dictionary compression to them.
     * Afterwards, elements in <code>strings</code> can be queried resulting in their dictionary compressed
     * counterpart. During importing, duplicates in <code>strings</code> are removed for both the
     * input <code>strings</code> and the already imported strings inside the database. In case a string
     * in <code>strings</code> is already imported, this string will be skipped.
     */
    int (*import_strings)(struct storage_engine *self,
                          optional struct vector_cursor of_type(struct compressed_string *) out,
                          const struct vector_cursor of_type(char *) strings);

    int (*find_strings_by_name)(struct storage_engine *self, )

    /**
     * Create new nodes in the database but without attachment these nodes to the graph.
     * Each node is expected to have its own unique IRI. For this, a deduplication is
     * performed. In case the parameter <code>iris</code> contains duplicates, the
     * result <code>out</code> will have less elements than <code>num_iris</code>
     * (i.e., exactly the number of distinct values in <code>irir</code>.
     *
     * Even if a node is not attached to the graph by being part of an RDF statement,
     * nodes created with this function can be queried by both their IRI and their
     * internal resource identifier.
     *
     * The implementer must ensure thread-safeness.
     */
    int  (*create_resources)(struct storage_engine *self, struct vector_cursor of_type(struct resource) *out,
                             const struct IRI *iris, size_t num_iris);

    /**
     * As <code>create_resource</code> but creates blank nodes, i.e., nodes that are
     * not associated with an IRI.
     *
     * Even if a node is not attached to the graph by being part of an RDF statement,
     * nodes created with this function can be queried by their internal resource identifier.
     * However, they cannot be queried otherwise unless they are directly connected to the
     * graph.
     *
     * The implementer must ensure thread-safeness.
     */
    int  (*create_blank_resources)(struct storage_engine *self, struct vector_cursor of_type(struct resource) *out,
                                   size_t num_nodes);

    /**
     * Removes nodes by their resource identifier given by the parameter <code>cursor</code>. In case
     * a node is being part of a statement, that statements gets removed, too. Unknown nodes are
     * skipped.
     */
    int  (*drop_resources)(struct storage_engine *self, struct vector_cursor of_type(struct resource) *cursor);

    /**
     * Queries nodes given their IRIs
     */
    int  (*find_resources_by_IRIs)(struct storage_engine *self, struct vector_cursor of_type(struct resource) *out,
                                   const struct vector_cursor of_type(IRI) *iris);

    /**
     * Return all nodes in the database that satisfy the predicate <code>pred</code>.
     */
    int  (*scan_resources)(struct storage_engine *self, struct vector_cursor of_type(struct resource) *out,
                           pred_func_t of_type(struct resource) pred);



    int  (*get_IRI_of_resource)(struct storage_engine *self, const struct IRI *out, resource_id_t id);

    int  (*create_property)(struct storage_engine *self, prop_id_t *out, resource_id_t subject,
                            const struct IRI *predicate_iri, enum rdf_builtin_type type, const void *literal);

    int  (*create_property_builtin)(struct storage_engine *self, prop_id_t *out, resource_id_t subject,
          enum rdf_property type, const char *value);

    int  (*drop_property)(struct storage_engine *self, prop_id_t prop_id);

    int  (*create_relationship)(struct storage_engine *self, relation_id_t *out, resource_id_t subject,
                                const struct IRI *relationship_iri, resource_id_t object);

    int  (*scan_resources)(struct storage_engine *self, struct resource_cursor *cursor, resource_pred_func pred,
                           void *capture);

    int  (*scan_properties)(struct storage_engine *self, struct prop_cursor *cursor, prop_pred_func pred,
                            void *capture);

    int  (*scan_relationships)(struct storage_engine *self, struct relationship_cursor *cursor,
                               relationship_pred_func pred, void *capture);

    int  (*scan_strings)(struct storage_engine *self, struct string_cursor *cursor, string_pred_func pred,
                         void *capture);

    int  (*scan_statement_prop)(struct storage_engine *self, struct statement_cursor *cursor, statement_prop_pred_func pred,
                                void *capture);

    int  (*scan_statement_relationship)(struct storage_engine *self, struct statement_cursor *cursor,
                                        statement_relationship_pred_func pred, void *capture);

    /* Query (S, P, O) for O is literal */
    int  (*find_statement_prop)(struct storage_engine *self, statment_id_t *out, resource_id_t subject,
                                prop_id_t predicate, enum rdf_builtin_type type, const void *literal);

    /* Query (S, P, O) for O is resource */
    int  (*find_statement_relationship)(struct storage_engine *self, statment_id_t *out, resource_id_t subject,
                                        relation_id_t predicate, resource_id_t object);

    /* Query (S, P, *) for O is literal */
    int  (*find_subject_by_property_value)(struct storage_engine *self, struct resource_cursor *cursor, enum rdf_builtin_type type, enum comp_pred pred, const void *comp_value);

    int  (*find_subject_by_property_builtin_with)(struct storage_engine *self, struct resource_cursor *cursor, enum rdf_property type, enum comp_pred pred, const char *comp_value);

    int  (*find_subject_by_relationship_with(struct storage_engine *self, struct resource_cursor *cursor, relation_id_t id, resource_id_t object);

    int  (*find_property_(struct storage_engine *self, struct resource_cursor *cursor, relation_id_t id, resource_id_t object); ...

};

#endif
