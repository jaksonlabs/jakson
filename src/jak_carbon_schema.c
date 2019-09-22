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
#include <jak_carbon_object_it.h>
#include <jak_carbon_schema_keywords.h>

bool jak_carbon_schema_validate(jak_carbon *schemaCarbon, jak_carbon **filesToVal) {
    JAK_ERROR_IF_NULL(schemaCarbon);
    JAK_ERROR_IF_NULL(filesToVal);
    
    jak_carbon_array_it it;
    jak_carbon_field_type_e field_type;

    jak_carbon_iterator_open(&it, schemaCarbon);
    jak_carbon_array_it_next(&it);
    jak_carbon_array_it_field_type(&field_type, &it);
    // a schema always has to be an object.
    if (!(jak_carbon_field_type_is_object(field_type))){
        JAK_ERROR_WDETAILS(&it.err, JAK_ERR_BADTYPE, "schema has to be an object.");
        jak_carbon_iterator_close(&it);
        //TODO: cleanup?
        return false;
    }

    jak_carbon_schema schema;
    if(!(jak_carbon_schema_createSchema(&schema, jak_carbon_array_it_object_value(&it), filesToVal))) {
        //TODO: error handling
        return false;
    }
    jak_carbon_iterator_close(&it);

    return true;   
}

bool jak_carbon_schema_createSchema(jak_carbon_schema *schema, jak_carbon_object_it *oit, jak_carbon **filesToVal) {

    // get schema size to avoid unnecessary reallocs
    unsigned int schemaSize = jak_carbon_schema_getSchemaSize(oit);
    jak_carbon_schema_content *content = malloc(schemaSize * sizeof(jak_carbon_schema_content*));
    schema->content = content;
    schema->content_size = schemaSize;
    
    unsigned int pos = 0;
    while(jak_carbon_object_it_next(oit)) {
        content[pos].key = oit->field.key.name;
        content[pos].value = &(oit->field.value.data);
    }
    if(!(jak_carbon_schema_handleKeys(schema, filesToVal))) {
        //TODO: error handling
        free(content);
        return false;
    }
    free(content);
    return true;
}


bool jak_carbon_schema_handleKeys(jak_carbon_schema *schema, jak_carbon **filesToVal) {
    bool status = true;
    for (unsigned int i = 0; i < schema->content_size; i++) {
        if (strcmp(schema->content[i].key, "type")==0) {
            status = jak_carbon_schema_keywords_type(schema, filesToVal);
        }
        else if (strcmp(schema->content[i].key, "properties")==0) {
                status = jak_carbon_schema_keywords_properties(schema, filesToVal);
        }
        else {
            //TODO: error handling
            return false;
        }
        if (status != true) {
            //TODO: error handling
            return false;
        }
    }
    return true;
}


unsigned int jak_carbon_schema_getSchemaSize(jak_carbon_object_it *oit) {
    unsigned int size = 0;
    while (jak_carbon_object_it_next(oit)) {
        size++;
    }
    jak_carbon_object_it_rewind(oit);
    return size;
}
