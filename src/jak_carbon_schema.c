/**
 * Copyright 2018 Marcus Pinnecke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------
#include "jak_carbon_schema.h"
#include <jak_carbon.h>
#include <jak_carbon_array_it.h>

bool jak_carbon_iterate_schema(jak_schema *schema) {
    JAK_UNUSED(schema);

    jak_carbon_array_it it;
    jak_carbon_field_type_e field_type;
    jak_carbon_iterator_open(&it, schema->carbon);
    jak_carbon_array_it_next(&it)
    jak_carbon_array_it_field_type(&field_type, &it);

    // top level of schema has to be an object
    if (!(jak_carbon_field_type_is_object(field_type))){
        JAK_ERROR_WDETAILS(&schema->err, JAK_ERR_BADTYPE, "expected object in schemafile.");
        jak_carbon_iterator_close(&it);
        return false;
    }
    jak_carbon_object_it *oit = jak_carbon_array_it_object_value(&it);
    
    jak_carbon_iterator_close(&it);
    


    return true;
}

bool jak_carbon_create_schema(jak_schema *schema, jak_carbon *schemaCarbon) {
    JAK_ERROR_IF_NULL(schemaCarbon);
    schema->carbon=schemaCarbon;

    JAK_UNUSED(schema);
    JAK_UNUSED(schemaCarbon);
    


    return true;
}

bool jak_carbon_validate_schema(jak_carbon *schemaCarbon, jak_carbon *carbon) {
    JAK_ERROR_IF_NULL(schemaCarbon);
    JAK_ERROR_IF_NULL(carbon);
    
    jak_schema schema;
    jak_carbon_create_schema(&schema, schemaCarbon);

    JAK_UNUSED(schemaCarbon);
    JAK_UNUSED(carbon);

    return true;   

}
