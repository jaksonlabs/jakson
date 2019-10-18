// bin/examples-read-record-contents

#include <stdio.h>
#include <jakson/jakson.h>

int main (void)
{
        carbon record;
        err err;
        carbon_array_it it;
        //string_buffer str;

        carbon_from_json(&record, "[\"Hello\", \"Number\", 23]", CARBON_KEY_NOKEY, NULL, &err);

        carbon_read_begin(&it, &record);
        while (carbon_array_it_next(&it)) {
                //carbon_array_it_
        }

        carbon_drop(&record);

        return 0;
}