#include <gtest/gtest.h>

#include <fstream>
#include <unordered_map>

#include <carbon/carbon.h>
#include <carbon/compressor/auto/selector-rule-based.h>

TEST(RuleSelector, Parser) {
    char const * content = "url.pattern=[a-z]+://[^\\s]+\n"
                           "url.join_group=1\n"
                           "url.option.reverse_strings=true\n"
                           "url.option.reverse_sort=false\n"
                           "url.option.prefix=table\n"
                           "url.option.suffix=configurable\n"
                           "url.option.huffman=true\n"
                           "\n"
                           "email.pattern=[^\\s]+@[^\\s]+\n"
                           "email.join_group=0\n"
                           "email.option.reverse_strings=true\n"
                           "email.option.reverse_sort=false\n"
                           "email.option.prefix=table\n"
                           "email.option.suffix=configurable\n"
                           "email.option.huffman=true\n";

    carbon_vec_t *rules = carbon_compressor_selector_rule_based_parse(content);

    for(size_t i = 0; i < rules->num_elems; ++i) {
        carbon_compressor_selector_rule_print(stdout, *(carbon_selector_rule_t * const *)carbon_vec_at(rules, i));
    }
    carbon_compressor_selector_rule_based_drop(rules);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
