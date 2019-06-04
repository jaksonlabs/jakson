#include <stdlib.h>
#include <stdio.h>
#include <core/bison/bison.h>

int main(void)
{

        struct bison doc;
        u64 rev;
        struct string_builder builder;
        bool status;

        string_builder_create(&builder);

        status = bison_create(&doc);


        status = bison_revision(&rev, &doc);

        struct bison_revise revise;
        bison_revise_begin(&revise, &doc);
        bison_revise_end(&revise);

        status = bison_revision(&rev, &doc);

        bison_to_str(&builder, JSON_FORMATTER, &doc);
        printf("%s\n", string_builder_cstr(&builder));
        string_builder_drop(&builder);

        bison_drop(&doc);

}