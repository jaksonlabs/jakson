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

#include <ng5_common.h>
#include <stdx/ng5_string_dic.h>
#include <stdx/ng5_slot_vector.h>
#include <ng5/ng5_result_handle.h>

enum storage_engine_tag {
    STORAGE_ENGINE_TAG_ROADFIRE
};

typedef size_t store_string_id_t;

typedef size_t store_iri_id_t;

typedef size_t store_resource_id_t;

typedef size_t store_prop_id_t;

typedef size_t store_relationship_id_t;

typedef size_t store_statement_id_t;

typedef int (*pred_func_t)(bool *result, const void *begin, size_t num_elems,
                           struct storage_engine *engine);

struct storage_engine
{
    /* implementation-specific objects */
    void                   *extra;

    /* ng5_allocator_t used */
    ng5_allocator_t        alloc;

    /* implementation tag */
    enum storage_engine_tag tag;

    int (*import_strings)(struct storage_engine *self,
                          optional struct result_handle of_type(store_string_id_t) *out,
                          const ng5_vector_t of_type(char *) *strings);

    int (*locate_strings)(struct storage_engine *self, optional struct result_handle of_type(store_string_id_t) *out,
                          const ng5_vector_t of_type(char *) *strings);

    int (*extract_strings)(struct storage_engine *self, struct result_handle of_type(char *) strings,
                           const ng5_vector_t of_type(struct compressed_string) *input);

    int (*find_strings)(struct storage_engine *self, struct result_handle of_type(store_string_id_t) *out,
                        pred_func_t pred);

    int (*drop_result)(struct storage_engine *self, slot_vector_slot_t result_slot);

//    /**
//     * Create new nodes in the database but without attachment these nodes to the graph.
//     * Each node is expected to have its own unique IRI. For this, a deduplication is
//     * performed. In case the parameter <code>iris</code> contains duplicates, the
//     * result <code>out</code> will have less elements than <code>num_iris</code>
//     * (i.e., exactly the number of distinct values in <code>irir</code>.
//     *
//     * Even if a node is not attached to the graph by being part of an RDF statement,
//     * nodes created with this function can be queried by both their IRI and their
//     * internal resource identifier.
//     *
//     * The implementer must ensure thread-safeness.
//     */
//    int  (*create_resources)(struct storage_engine *self, struct vector_cursor of_type(struct resource) *out,
//                             const struct IRI *iris, size_t num_iris);
//
//    /**
//     * As <code>create_resource</code> but creates blank nodes, i.e., nodes that are
//     * not associated with an IRI.
//     *
//     * Even if a node is not attached to the graph by being part of an RDF statement,
//     * nodes created with this function can be queried by their internal resource identifier.
//     * However, they cannot be queried otherwise unless they are directly connected to the
//     * graph.
//     *
//     * The implementer must ensure thread-safeness.
//     */
//    int  (*create_blank_resources)(struct storage_engine *self, struct vector_cursor of_type(struct resource) *out,
//                                   size_t num_nodes);
//
//    /**
//     * Removes nodes by their resource identifier given by the parameter <code>cursor</code>. In case
//     * a node is being part of a statement, that statements gets removed, too. Unknown nodes are
//     * skipped.
//     */
//    int  (*drop_resources)(struct storage_engine *self, struct vector_cursor of_type(struct resource) *cursor);
//
//    /**
//     * Queries nodes given their IRIs
//     */
//    int  (*find_resources_by_IRIs)(struct storage_engine *self, struct vector_cursor of_type(struct resource) *out,
//                                   const struct vector_cursor of_type(IRI) *iris);
//
//    /**
//     * Return all nodes in the database that satisfy the predicate <code>pred</code>.
//     */
//    int  (*scan_resources)(struct storage_engine *self, struct vector_cursor of_type(struct resource) *out,
//                           pred_func_t of_type(struct resource) pred);
//
//
//    /**
//     *
//     */
//    int  (*get_IRI_of_resource)(struct storage_engine *self, const struct IRI *out, resource_id_t id);
//
//    int  (*create_property)(struct storage_engine *self, prop_id_t *out, resource_id_t subject,
//                            const struct IRI *predicate_iri, enum rdf_builtin_type type, const void *literal);
//
//    int  (*create_property_builtin)(struct storage_engine *self, prop_id_t *out, resource_id_t subject,
//          enum rdf_property type, const char *value);
//
//    int  (*drop_property)(struct storage_engine *self, prop_id_t prop_id);
//
//    int  (*create_relationship)(struct storage_engine *self, relation_id_t *out, resource_id_t subject,
//                                const struct IRI *relationship_iri, resource_id_t object);
//
//    int  (*scan_resources)(struct storage_engine *self, struct resource_cursor *cursor, resource_pred_func pred,
//                           void *capture);
//
//    int  (*scan_properties)(struct storage_engine *self, struct prop_cursor *cursor, prop_pred_func pred,
//                            void *capture);
//
//    int  (*scan_relationships)(struct storage_engine *self, struct relationship_cursor *cursor,
//                               relationship_pred_func pred, void *capture);
//
//    int  (*scan_strings)(struct storage_engine *self, struct string_cursor *cursor, string_pred_func pred,
//                         void *capture);
//
//    int  (*scan_statement_prop)(struct storage_engine *self, struct statement_cursor *cursor, statement_prop_pred_func pred,
//                                void *capture);
//
//    int  (*scan_statement_relationship)(struct storage_engine *self, struct statement_cursor *cursor,
//                                        statement_relationship_pred_func pred, void *capture);
//
//    /* Query (S, P, O) for O is literal */
//    int  (*find_statement_prop)(struct storage_engine *self, statment_id_t *out, resource_id_t subject,
//                                prop_id_t predicate, enum rdf_builtin_type type, const void *literal);
//
//    /* Query (S, P, O) for O is resource */
//    int  (*find_statement_relationship)(struct storage_engine *self, statment_id_t *out, resource_id_t subject,
//                                        relation_id_t predicate, resource_id_t object);
//
//    /* Query (S, P, *) for O is literal */
//    int  (*find_subject_by_property_value)(struct storage_engine *self, struct resource_cursor *cursor, enum rdf_builtin_type type, enum comp_pred pred, const void *comp_value);
//
//    int  (*find_subject_by_property_builtin_with)(struct storage_engine *self, struct resource_cursor *cursor, enum rdf_property type, enum comp_pred pred, const char *comp_value);
//
//    int  (*find_subject_by_relationship_with(struct storage_engine *self, struct resource_cursor *cursor, relation_id_t id, resource_id_t object);
//
//    int  (*find_property_(struct storage_engine *self, struct resource_cursor *cursor, relation_id_t id, resource_id_t object); ...

};

/**
 * Imports <code>strings</code> into the database by applying dictionary compression to them.
 * Afterwards, elements in <code>strings</code> can be queried resulting in their dictionary compressed
 * counterpart. During importing, duplicates in <code>strings</code> are removed for both the
 * input <code>strings</code> and the already imported strings inside the database. In case a string
 * in <code>strings</code> is already imported, this string will be skipped.
*/
unused_fn static int storage_engine_import_strings(struct storage_engine *engine,
        optional struct result_handle of_type(store_string_id_t) *out,
        const ng5_vector_t of_type(char *) *strings)
{
    check_non_null(engine)
    check_non_null(strings)
    assert(engine->import_strings);
    return engine->import_strings(engine, out, strings);
}

/**
 * Same as <code>import_strings</code> but does not add strings into then database which are not yet
 * imported.
 */
unused_fn static int storage_engine_locate_strings(struct storage_engine *engine,
        optional struct result_handle of_type(store_string_id_t) *out,
        const ng5_vector_t of_type(char *) *strings)
{
    check_non_null(engine)
    check_non_null(strings)
    assert(engine->locate_strings);
    return engine->locate_strings(engine, out, strings);
}

/**
 * Decodes strings given in <code>input</code> and returns c-strings for them.
 */
unused_fn static int storage_engine_extract_strings(struct result_handle of_type(char *) *out, struct storage_engine *engine,
        const ng5_vector_t of_type(struct compressed_string) *input)
{
    check_non_null(engine)
    check_non_null(out)
    check_non_null(input)
    assert(engine->extract_strings);
    unused(out);
    unused(engine);
    unused(input);
    NOT_YET_IMPLEMENTED
    return 0;
   // return engine->extract_strings(engine, out, input);
}

/**
 * Returns encoded strings where the original uncoded strings satisfy the predicate <code>pred</code>
 */
unused_fn static int storage_engine_find_strings(struct result_handle of_type(store_string_id_t) *out,
        struct storage_engine *engine, pred_func_t pred)
{
    check_non_null(engine)
    check_non_null(out)
    check_non_null(pred)
    assert(engine->find_strings);
    return engine->find_strings(engine, out, pred);
}

/**
 * Drops the handle to this result, eventually freeing resources or recycle resources for other results
 */
unused_fn static int storage_engine_drop_result(struct result_handle *handle)
{
    check_non_null(handle)
    check_non_null(handle->context)
    assert(handle->context->drop_result);
    return handle->context->drop_result(handle->context, handle->id);
}

#endif
