#include <regex.h>
#include <carbon/compressor/auto/selector-rule-based.h>

#define CARBON_COMPRESSOR_REGEX_NUM_GROUPS 4

// Avoid writing out these every time...
static carbon_compressor_configurable_prefix_type_e const inc = carbon_compressor_configurable_prefix_type_incremental;
static carbon_compressor_configurable_prefix_type_e const table = carbon_compressor_configurable_prefix_type_prefix_dict_coding;

typedef enum rule_params {
    rule_param_pattern,
    rule_param_joinable,
    rule_param_option,
    rule_param_unknown
} rule_params_e;

typedef struct {
    rule_params_e param;
    char const *  name;
} rule_param_description_t;

static rule_param_description_t rule_params[] = {
    { .param = rule_param_pattern, .name = "pattern" },
    { .param = rule_param_joinable, .name = "joinable" }
};

static rule_params_e rule_by_string(char const *name) {
    for(size_t i = 0; i < sizeof(rule_params)/sizeof(rule_params[ 0 ]); ++i) {
        if(strcmp(name, rule_params[ i ].name) == 0)
            return rule_params[ i ].param;
    }

    if(strlen(name) > strlen("option.") && strncmp(name, "option.", strlen("option.")) == 0) {
        return rule_param_option;
    }

    return rule_param_unknown;
}

static char *
selector_read_file(char const * filename) {
    FILE *file = fopen(filename, "rb");
    fseek(file, 0, SEEK_END);
    size_t file_size = (size_t)ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(file_size + 1);
    fread(content, 1, file_size, file);
    fclose(file);
    content[file_size] = 0;

    return content;
}

static double
selector_rule_match_percentage(carbon_vec_t ofType(char *) *strings, carbon_selector_rule_t *rule, double min_support_relative);

carbon_vec_t ofType(carbon_selector_rule_t) *carbon_compressor_selector_rule_based_parse(
        char const *content
    ) {

    regex_t    line_parser;
    regmatch_t line_parser_matches[CARBON_COMPRESSOR_REGEX_NUM_GROUPS];
    regcomp(&line_parser, "([^\\.\\=]+)\\.([^\\=]+)=(.*)", REG_EXTENDED|REG_NEWLINE);

    carbon_vec_t ofType(carbon_selector_rule_t) *rules = malloc(sizeof(carbon_vec_t));
    carbon_vec_create(rules, NULL, sizeof(carbon_selector_rule_t), 10);

    carbon_hashmap_t rule_map = carbon_hashmap_new();

    char const * cursor = content;
    while(regexec(&line_parser, cursor, CARBON_COMPRESSOR_REGEX_NUM_GROUPS, line_parser_matches, REG_EXTENDED) == 0) {
        char *tokens[CARBON_COMPRESSOR_REGEX_NUM_GROUPS];

        for(size_t i = 0; i < CARBON_COMPRESSOR_REGEX_NUM_GROUPS; ++i) {
            size_t match_length = (size_t)(line_parser_matches[ i ].rm_eo - line_parser_matches[ i ].rm_so);
            tokens[ i ] = malloc(match_length + 1);
            tokens[ i ][ match_length ] = 0;

            strncpy(tokens[ i ], cursor + line_parser_matches[ i ].rm_so, match_length);
        }

        cursor += line_parser_matches[ 0 ].rm_eo;

        carbon_selector_rule_t *rule;
        if(carbon_hashmap_get(rule_map, tokens[ 1 ], (carbon_hashmap_any_t)&rule) == carbon_hashmap_status_missing) {
            rule = malloc(sizeof(carbon_selector_rule_t));
            rule->name = strdup(tokens[ 1 ]);
            rule->pattern = malloc( 1 );
            rule->pattern[ 0 ] = 0;
            rule->options = carbon_hashmap_new();
            rule->join_group = 0;

            carbon_vec_push(rules, &rule, 1);
            carbon_hashmap_put(rule_map, tokens[ 1 ], rule);
        }

        switch(rule_by_string(tokens[ 2 ])) {
            case rule_param_pattern: { free(rule->pattern); rule->pattern = strdup(tokens[ 3 ]); break; }
            case rule_param_join_group: { rule->join_group = strtoul(tokens[ 3 ], NULL, 10); break; }
            case rule_param_option: { carbon_hashmap_put(rule->options, tokens[ 2 ] + strlen("option."), strdup(tokens[ 3 ])); break; }
            case rule_param_unknown:
            {
                fprintf(stderr, "Encountered unknown option '%s' while loading rule-based selector ruleset.\n", tokens[ 2 ]);
                return NULL;
            }
        }

        for(size_t i = 0; i < CARBON_COMPRESSOR_REGEX_NUM_GROUPS; ++i)
            free(tokens[ i ]);
    }

    regfree(&line_parser);
    carbon_hashmap_drop(rule_map);

    return rules;
}

void carbon_compressor_selector_rule_based_drop(
        carbon_vec_t ofType(carbon_selector_rule_t) *rules
    ) {
    for(size_t i = 0; i < rules->num_elems; ++i) {
        carbon_selector_rule_t* rule = *(carbon_selector_rule_t * const *)carbon_vec_at(rules, i);
        free(rule->name);
        free(rule->pattern);

        for(carbon_hashmap_iterator_t it = carbon_hashmap_begin(rule->options);it.valid;carbon_hashmap_next(&it)) {
            free(it.value);
        }

        carbon_hashmap_drop(rule->options);
        free(rule);
    }

    carbon_vec_drop(rules);
    free(rules);
}


void carbon_compressor_selector_rule_print(
        FILE *file, carbon_selector_rule_t *rule
    ) {
    fprintf(file, "%s.pattern=%s\n", rule->name, rule->pattern);
    fprintf(file, "%s.join_group=%zu\n", rule->name, rule->join_group);

    for(carbon_hashmap_iterator_t it = carbon_hashmap_begin(rule->options);it.valid;carbon_hashmap_next(&it)) {
        fprintf(file, "%s.option.%s=%s\n", rule->name, it.key, (char const *)it.value);
    }
}

void carbon_compressor_selector_rule_based_config_init(
        carbon_compressor_selector_rule_based_config_t *config
    ) {
    config->rules = NULL;
    config->min_support = 0.0;
}

carbon_compressor_selector_result_t carbon_compressor_find_by_strings_rule_based(
        carbon_vec_t ofType(char *) *strings,
        carbon_doc_bulk_t const *context,
        carbon_compressor_selector_rule_based_config_t *config,
        carbon_compressor_selector_sampling_config_t const sampling_config
    ) {
    if(config->rules == 0) {
        char * content = selector_read_file(config->ruleset_filename);
        config->rules = carbon_compressor_selector_rule_based_parse(content);
        free(content);
    }

    carbon_err_t err;
    size_t rule_idx = 0;
    size_t default_idx = 0;
    carbon_selector_rule_t *rule = NULL;
    carbon_selector_rule_t *default_rule = NULL;
    carbon_vec_t ofType(char *) *samples = carbon_compressor_selector_sample_strings(strings, &sampling_config);

    double max_matches = 0.0;
    for(size_t i = 0; i < config->rules->num_elems; ++i) {
        carbon_selector_rule_t *tmp = *(carbon_selector_rule_t * const *)carbon_vec_at(config->rules, i);

        if(strcmp(tmp->name, "default") == 0) {
            default_rule = tmp;
            default_idx  = i;
        } else {
            double matches = selector_rule_match_percentage(samples, tmp, config->min_support);
            if(matches > max_matches) {
                max_matches = matches;
                rule_idx    = i;
                rule        = tmp;
            }
        }
    }

    if(rule == NULL) {
        rule     = default_rule;
        rule_idx = default_idx;
    }

    carbon_compressor_selector_result_t result;
    carbon_huffman_create_all_eq_encoder(&result.huffman);

    result.compressor = malloc(sizeof(carbon_compressor_t));
    carbon_compressor_by_type(&err, result.compressor, context, CARBON_COMPRESSOR_CONFIGURABLE);
    for(carbon_hashmap_iterator_t it = carbon_hashmap_begin(rule->options);it.valid;carbon_hashmap_next(&it)) {
        if(!carbon_compressor_set_option(&err, result.compressor, it.key, (char *)it.value)) {
            CARBON_PRINT_ERROR_AND_DIE(err.code);
        }
    }

    carbon_compressor_incremental_extra_t * extra =
            (carbon_compressor_incremental_extra_t *)result.compressor->extra;
    result.config.prefix          = extra->config.prefix;
    result.config.suffix          = extra->config.suffix;
    result.config.huffman         = extra->config.huffman;
    result.config.reverse_sort    = extra->config.reverse_sort;
    result.config.reverse_strings = extra->config.reverse_strings;
    result.joinable_group         = rule->joinable ? rule_idx : SIZE_MAX;
    result.estimated_size         = 0;

    CARBON_CONSOLE_WRITELN(
        stdout,
        "            Detected settings: rule: %s, prefix = %s, suffix = %s, huffman = %s, rev_str = %s, rev_sort = %s",
        rule->name,
        (result.config.prefix == inc ? "incremental" : ( result.config.prefix == table ? "prefix-dict" : "none" )),
        (result.config.suffix == inc ? "incremental" : ( result.config.suffix == table ? "prefix-dict" : "none" )),
        result.config.huffman ?         "true" : "false",
        result.config.reverse_strings ? "true" : "false",
        result.config.reverse_sort ?    "true" : "false"
    );

    return result;
}

static double selector_rule_match_percentage(
        carbon_vec_t ofType(char *) *strings,
        carbon_selector_rule_t *rule,
        double min_support_relative
    ) {
    size_t  count = 0;
    size_t  min_support_abs = (size_t)(min_support_relative * strings->num_elems);

    regex_t match;
    regmatch_t matches[1];
    regcomp(&match, rule->pattern, REG_EXTENDED);

    for(size_t i = 0; i < strings->num_elems; ++i) {
        char const * string = *(char const * const *)carbon_vec_at(strings, i);
        if(regexec(&match, string, 1, matches, REG_EXTENDED) == 0) {
            if((size_t)(matches[0].rm_eo - matches[0].rm_so) == strlen(string))
                ++count;
        } else {
            if(min_support_abs > count && strings->num_elems - i + 1 < min_support_abs - count) {
                // There are not even enough entries left to achieve at least min_support_abs matches -> early out
                regfree(&match);

                printf("             - Early out on trying rule %s after %zu values: only %zu matched so far\n", rule->name, i + 1, count);

                return 0.0;
            }
        }
    }

    regfree(&match);

    return (double)count / (double)strings->num_elems;
}
