
#include <inttypes.h>
#include <jak_pack.h>
#include <jak_archive_query.h>
#include <jak_archive_int.h>
#include <jak_carbon.h>

#include "modules.h"

struct js_to_context
{
    struct jak_string_dict dictionary;
    struct jak_doc_bulk context;
    struct jak_doc_entries *partition;
    struct jak_column_doc *partitionMetaModel;
    char *jsonContent;
};

static int convertJs2Model(struct js_to_context *context, FILE *file, bool optimizeForReads, const char *fileName,
                           size_t fileNum, size_t fileMax)
{

    JAK_CONSOLE_WRITELN(file, "** Process file %zu of %zu **", fileNum, fileMax);
    JAK_CONSOLE_WRITELN(file, "%s", fileName);

    JAK_CONSOLE_WRITE(file, "  - Read contents into memory%s", "");

    FILE *f = fopen(fileName, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    context->jsonContent = JAK_MALLOC(fsize + 1);
    size_t nread = fread(context->jsonContent, fsize, 1, f);
    JAK_UNUSED(nread);
    fclose(f);
    context->jsonContent[fsize] = 0;

    JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");


    JAK_CONSOLE_WRITE(file, "  - Setup string dictionary%s", "");

    encode_async_create(&context->dictionary, 1000, 1000, 1000, 8, NULL);
    JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");

    JAK_CONSOLE_WRITE(file, "  - Parse JSON file%s", "");
    struct jak_json_parser parser;
    struct jak_json_err error_desc;
    struct jak_json jsonAst;
    json_parser_create(&parser);
    int status = json_parse(&jsonAst, &error_desc, &parser, context->jsonContent);
    if (!status) {
        JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
        if (error_desc.token) {
            JAK_CONSOLE_WRITELN(file, "** ERROR ** Parsing failed: %s\nBut token %s was found in line %u column %u",
                                   error_desc.msg, error_desc.token_type_str, error_desc.token->line,
                                   error_desc.token->column);
        } else {
            JAK_CONSOLE_WRITELN(file, "** ERROR ** Parsing failed: %s", error_desc.msg);
        }
        return false;
    } else {
        JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");
    }

    JAK_CONSOLE_WRITE(file, "  - Test document restrictions%s", "");
    struct jak_error err;
    status = json_test(&err, &jsonAst);
    if (!status) {
        JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
        error_print_to_stderr(&err);
        return false;
    } else {
        JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");
    }

    JAK_CONSOLE_WRITE(file, "  - Create bulk insertion bulk%s", "");
    doc_bulk_create(&context->context, &context->dictionary);
    JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");

    context->partition = doc_bulk_new_entries(&context->context);

    JAK_CONSOLE_WRITE(file, "  - Add file to new partition%s", "");
    doc_bulk_add_json(context->partition, &jsonAst);
    json_drop(&jsonAst);
    JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");

    JAK_CONSOLE_WRITE(file, "  - Cleanup reserved memory%s", "");
    doc_bulk_shrink(&context->context);
    JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");

    JAK_CONSOLE_WRITE(file, "  - Finalize partition%s", "");
    context->partitionMetaModel =
        doc_entries_columndoc(&context->context, context->partition, optimizeForReads);
    JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");

    return true;
}

static void cleanup(FILE *file, struct js_to_context *context)
{
    JAK_CONSOLE_WRITE(file, "  - Perform cleanup operations%s", "");
    strdic_drop(&context->dictionary);
    doc_bulk_Drop(&context->context);
    doc_entries_drop(context->partition);
    columndoc_free(context->partitionMetaModel);
    free(context->jsonContent);
    free(context->partitionMetaModel);
    JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");
}

static int testFileExists(FILE *file, const char *fileName, size_t fileNum, size_t fileMax, bool requireExistence)
{
    JAK_CONSOLE_WRITE(file, "Check file %zu of %zu", fileNum, fileMax);
    if (access( fileName, F_OK ) == 0) {
        if (requireExistence) {
            goto success;
        } else {
            goto fail;
        }
    } else {
        if (requireExistence) {
            goto fail;
        } else {
            goto success;
        }
    }

fail:
    JAK_CONSOLE_WRITE_CONT(file, "[%s]", "ERROR");
    JAK_CONSOLE_WRITE_ENDL(file);
    JAK_CONSOLE_WRITELN(file, "** ERROR ** file I/O error for file '%s'", fileName);
    return false;

success:
    JAK_CONSOLE_WRITE_CONT(file, "[%s]", "OK");
    JAK_CONSOLE_WRITE_ENDL(file);
    return true;

}

bool moduleCheckJsInvoke(int argc, char **argv, FILE *file, struct cmdopt_mgr *manager)
{
    JAK_UNUSED(manager);

    struct js_to_context cabContext;

    for (int i = 0; i < argc; i++) {
        if (testFileExists(file, argv[i], i + 1, argc, true) != true) {
            return false;
        }
    }

    for (int i = 0; i < argc; i++) {
        if (convertJs2Model(&cabContext, file, false, argv[i], i + 1, argc) != true) {
            return false;
        }
        cleanup(file, &cabContext);
    }

    JAK_CONSOLE_WRITELN(file, "Input files passed tests%s", "");

    return true;
}

#define JS_2_CAB_OPTION_FORCE_OVERWRITE "--force-overwrite"
#define JS_2_CAB_OPTION_SILENT_OUTPUT "--silent"
#define JS_2_CAB_OPTION_SIZE_OPTIMIZED "--size-optimized"
#define JS_2_CAB_OPTION_READ_OPTIMIZED "--read-optimized"
#define JS_2_CAB_OPTION_DIC_TYPE "--dic-type"
#define JS_2_CAB_OPTION_DIC_NTHREADS "--dic-nthreads"
#define JS_2_CAB_OPTION_NO_STRING_ID_INDEX "--no-string-id-index"
#define JS_2_CAB_OPTION_USE_COMPRESSOR "--compressor"
#define JS_2_CAB_OPTION_USE_COMPRESSOR_HUFFMAN "huffman"

static void tracker_begin_create_from_model()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Create from model started");
}

static void tracker_end_create_from_model()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Create from model finished");
}

static void tracker_begin_create_from_json()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Create from json started");
}

static void tracker_end_create_from_json()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Create from json finished");
}

static void tracker_begin_jak_archive_stream_from_json()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Create stream from json started");
}

static void tracker_end_jak_archive_stream_from_json()

{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Create stream from json finished");
}
static void tracker_begin_write_archive_file_to_disk()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Write archive to disk started");
}

static void tracker_end_write_archive_file_to_disk()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Write archive to disk finished");
}

static void tracker_begin_load_archive()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Load archive from disk started");
}

static void tracker_end_load_archive()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Load archive from disk finished");
}

static void tracker_begin_setup_string_dictionary()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Setup string dictionary started");
}

static void tracker_end_setup_string_dictionary()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Setup string dictionary finished");
}

static void tracker_begin_parse_json()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Parsing json started");
}

static void tracker_end_parse_json()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Parsing json finished");
}

static void tracker_begin_test_json()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Test document for restrictions started");
}

static void tracker_end_test_json()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Test document for restrictions finished");
}

static void tracker_begin_import_json()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Import document started");
}

static void tracker_end_import_json()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Import document finished");
}

static void tracker_begin_cleanup()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Cleanup started");
}

static void tracker_end_cleanup()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Cleanup finished");
}

static void tracker_begin_write_string_table()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Writing string table started");
}

static void tracker_end_write_string_table()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Writing string table finished");
}

static void tracker_begin_write_record_table()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Writing record table started");
}

static void tracker_end_write_record_table()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Writing record table finished");
}

static void tracker_skip_string_id_index_baking()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Backing indexes skipped");
}

static void tracker_begin_string_id_index_baking()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Backing string id to offset index started");
}

static void tracker_end_string_id_index_baking()
{
    JAK_CONSOLE_WRITELN(stdout, "%s", "  - Backing string id to offset index finished");
}


bool moduleJs2CabInvoke(int argc, char **argv, FILE *file, struct cmdopt_mgr *manager)
{
    JAK_UNUSED(manager);

    if (argc < 2) {
        JAK_CONSOLE_WRITE(file, "Require at least <output> and <input> parameters for <args>.%s", "");
        JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
        JAK_CONSOLE_WRITELN(file, "Run '%s' to see an example on the usage.", "$ types convert");
        return false;
    } else {
        bool flagSizeOptimized = false;
        bool flagReadOptimized = false;
        bool flagForceOverwrite = false;
        bool flagBakeStringIdIndex = true;
        enum jak_packer_type compressor = PACK_NONE;
        enum jak_str_dict_tag dic_type = ASYNC;
        int string_dic_async_nthreads = 8;

        int outputIdx = 0, inputIdx = 1;
        int i;

        for (i = 0; i < argc; i++) {
            char *opt = argv[i];
            if (strncmp(opt, "--", 2) == 0) {
                if (strcmp(opt, JS_2_CAB_OPTION_SIZE_OPTIMIZED) == 0) {
                    flagSizeOptimized = true;
                    compressor = PACK_HUFFMAN;
                } else if (strcmp(opt, JS_2_CAB_OPTION_READ_OPTIMIZED) == 0) {
                    flagReadOptimized = true;
                } else if (strcmp(opt, JS_2_CAB_OPTION_NO_STRING_ID_INDEX) == 0) {
                    flagBakeStringIdIndex = false;
                } else if (strcmp(opt, JS_2_CAB_OPTION_SILENT_OUTPUT) == 0) {
                    JAK_CONSOLE_OUTPUT_OFF();
                } else if (strcmp(opt, JS_2_CAB_OPTION_FORCE_OVERWRITE) == 0) {
                    flagForceOverwrite = true;
                } else if (strcmp(opt, JS_2_CAB_OPTION_USE_COMPRESSOR) == 0 && i++ < argc) {
                    const char *compressor_name = argv[i];
                    if (!pack_by_name(&compressor, compressor_name)) {
                        JAK_CONSOLE_WRITE(file, "unsupported pack requested: '%s'",
                                             compressor_name);
                        JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
                        JAK_CONSOLE_WRITELN(file, "** ERROR ** unsupported operation requested: %s", opt);
                        return false;
                    }
                } else if (strcmp(opt, JS_2_CAB_OPTION_DIC_TYPE) == 0 && i++ < argc) {
                    const char *dic_type_name = argv[i];
                    if (strcmp(dic_type_name, "async") == 0) {
                        dic_type = ASYNC;
                    } else if (strcmp(dic_type_name, "sync") == 0) {
                        dic_type = SYNC;
                    } else {
                        JAK_CONSOLE_WRITE(file, "unsupported dictionary type requested: '%s'",
                                             dic_type_name);
                        JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
                        JAK_CONSOLE_WRITELN(file, "** ERROR ** unsupported operation requested: %s", opt);
                        return false;
                    }
                } else if (strcmp(opt, JS_2_CAB_OPTION_DIC_NTHREADS) == 0 && i++ < argc) {
                    const char *nthreads_str = argv[i];
                    int nthreads_atoid = atoi(nthreads_str);
                    if (nthreads_atoid > 0) {
                        string_dic_async_nthreads = nthreads_atoid;
                    } else {
                        JAK_CONSOLE_WRITE(file, "not a number or zero threads assigned: '%s'",
                                             nthreads_str);
                        JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
                        JAK_CONSOLE_WRITELN(file, "** ERROR ** thread setting cannot be applied: %s", opt);
                        return false;
                    }
                } else {
                    JAK_CONSOLE_WRITELN(file, "** ERROR ** unrecognized option '%s'", opt);
                    return false;
                }
            } else {
                break;
            }
        }

        if (!flagSizeOptimized && compressor != PACK_NONE) {
            JAK_CONSOLE_WRITELN(file, "** WARNING ** a pack was specified but will be ignored because size "
                "optimization is turned off. Use '--size-optimized' such that a pack has any effect%s", "");
        }

        if (i + 1 >= argc) {
            JAK_CONSOLE_WRITELN(file, "** ERROR ** require <output> and <input> parameter: %d remain", argc);
            return false;
        } else if (argc - (i + 1) != 1) {
            JAK_CONSOLE_WRITELN(file, "** ERROR ** unsupported number of arguments: %d provided beyond <input>", argc - (i + 1));
            return false;
        } else {
            outputIdx = i;
            inputIdx = i + 1;
        }

        const char *pathCarbonFileOut = argv[outputIdx];
        const char *pathJsonFileIn = argv[inputIdx];

        if (!flagForceOverwrite && (testFileExists(file, pathCarbonFileOut, 1, 1, false) != true)) {
            JAK_CONSOLE_WRITELN(file, "Output file already exists. Remove it first, or use --force-overwrite. %s", "STOP.");
            return false;
        }
        if (testFileExists(file, pathJsonFileIn, 1, 1, true) != true) {
            JAK_CONSOLE_WRITELN(file, "Input file cannot be found. %s", "STOP.");
            return false;
        }

        JAK_CONSOLE_WRITELN(file, "  - Read contents into memory%s", "");

        FILE *f = fopen(pathJsonFileIn, "rb");
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        char *jsonContent = JAK_MALLOC(fsize + 1);
        size_t nread = fread(jsonContent, fsize, 1, f);
        JAK_UNUSED(nread);
        fclose(f);
        jsonContent[fsize] = 0;

        jak_archive archive;
        struct jak_error err;

        jak_archive_callback progress_tracker = { 0 };
        progress_tracker.begin_create_from_model = tracker_begin_create_from_model;
        progress_tracker.end_create_from_model = tracker_end_create_from_model;
        progress_tracker.begin_create_from_json = tracker_begin_create_from_json;
        progress_tracker.end_create_from_json = tracker_end_create_from_json;
        progress_tracker.begin_jak_archive_stream_from_json = tracker_begin_jak_archive_stream_from_json;
        progress_tracker.end_jak_archive_stream_from_json = tracker_end_jak_archive_stream_from_json;
        progress_tracker.begin_write_archive_file_to_disk = tracker_begin_write_archive_file_to_disk;
        progress_tracker.end_write_archive_file_to_disk = tracker_end_write_archive_file_to_disk;
        progress_tracker.begin_load_archive = tracker_begin_load_archive;
        progress_tracker.end_load_archive = tracker_end_load_archive;
        progress_tracker.begin_setup_string_dictionary = tracker_begin_setup_string_dictionary;
        progress_tracker.end_setup_string_dictionary = tracker_end_setup_string_dictionary;
        progress_tracker.begin_parse_json = tracker_begin_parse_json;
        progress_tracker.end_parse_json = tracker_end_parse_json;
        progress_tracker.begin_test_json = tracker_begin_test_json;
        progress_tracker.end_test_json = tracker_end_test_json;
        progress_tracker.begin_import_json = tracker_begin_import_json;
        progress_tracker.end_import_json = tracker_end_import_json;
        progress_tracker.begin_cleanup = tracker_begin_cleanup;
        progress_tracker.end_cleanup = tracker_end_cleanup;
        progress_tracker.begin_write_string_table = tracker_begin_write_string_table;
        progress_tracker.end_write_string_table = tracker_end_write_string_table;
        progress_tracker.begin_write_record_table = tracker_begin_write_record_table;
        progress_tracker.end_write_record_table = tracker_end_write_record_table;
        progress_tracker.skip_string_id_index_baking = tracker_skip_string_id_index_baking;
        progress_tracker.begin_string_id_index_baking = tracker_begin_string_id_index_baking;
        progress_tracker.end_string_id_index_baking = tracker_end_string_id_index_baking;

        if (!jak_archive_from_json(&archive, pathCarbonFileOut, &err, jsonContent,
                                      compressor, dic_type, string_dic_async_nthreads, flagReadOptimized,
                                      flagBakeStringIdIndex, &progress_tracker)) {
            error_print_and_abort(&err);
        } else {
            jak_archive_close(&archive);
        }


        free(jsonContent);

//        struct jak_memblock *carbonFile;
//        JAK_CONSOLE_WRITE(file, "  - Convert partition into in-memory CARBON file%s", "");
//        struct jak_error err;
//        if (!jak_archive_from_model(&carbonFile, &err, cabContext.partitionMetaModel, pack, flagBakeStringIdIndex)) {
//            error_print_and_abort(&err);
//        }
//        JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");
//
//        JAK_CONSOLE_WRITE(file, "  - Write in-memory CARBON file to disk%s", "");
//        FILE *outputFile = fopen(pathCarbonFileOut, "w");
//        if (!outputFile) {
//            JAK_CONSOLE_WRITE(file, "Unable to open file for writing: '%s'", pathCarbonFileOut);
//            JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
//            return false;
//        } else {
//            if (jak_archive_write(outputFile, carbonFile) != true) {
//                JAK_CONSOLE_WRITE(file, "Unable to write to file: '%s'", pathCarbonFileOut);
//                JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
//                return false;
//            }
//            fclose(outputFile);
//        }
//        JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");
//
//        JAK_CONSOLE_WRITE(file, "  - Clean up in-memory CARBON file%s", "");
//        memblock_drop(carbonFile);
//        JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");
//
//        cleanup(file, &cabContext);

        JAK_CONSOLE_WRITELN(file, "Conversion successfull%s", "");

        return true;
    }
}

bool moduleViewCabInvoke(int argc, char **argv, FILE *file, struct cmdopt_mgr *manager)
{
    JAK_UNUSED(argc);
    JAK_UNUSED(argv);
    JAK_UNUSED(file);
    JAK_UNUSED(manager);

    JAK_CONSOLE_OUTPUT_OFF()

    if (argc != 1) {
        JAK_CONSOLE_WRITE(file, "Require exactly one argument <types-file> that is a path to a types file.%s", "");
    } else {
        const char *carbonFilePath = argv[0];
        if (testFileExists(file, carbonFilePath, 1, 1, true) != true) {
            JAK_CONSOLE_OUTPUT_ON()
            JAK_CONSOLE_WRITELN(file, "Input file '%s' cannot be found. STOP", carbonFilePath);
            return false;
        }
        struct jak_error err;
        FILE *inputFile = fopen(carbonFilePath, "r");
        struct jak_memblock *stream;
        jak_archive_load(&stream, inputFile);
        if (!jak_archive_print(stdout, &err, stream)) {
            error_print_to_stderr(&err);
            error_drop(&err);
        }
        memblock_drop(stream);
        fclose(inputFile);

    }

    return true;
}

bool moduleInspectInvoke(int argc, char **argv, FILE *file, struct cmdopt_mgr *manager)
{
    JAK_UNUSED(manager);

    if (argc < 1) {
        JAK_CONSOLE_WRITE(file, "Require input file <input> as parameter for <args>.%s", "");
        JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
        JAK_CONSOLE_WRITELN(file, "Run '%s' to see an example on the usage.", "$ types inspect");
        return false;
    } else {
        const int filePathArgIdx = 0;
        const char *pathCarbonFileIn = argv[filePathArgIdx];

        JAK_CONSOLE_OUTPUT_OFF()
        if (testFileExists(file, pathCarbonFileIn, 1, 1, true) != true) {
            JAK_CONSOLE_OUTPUT_ON()
            JAK_CONSOLE_WRITELN(file, "Input file cannot be found. %s", "STOP.");
            return false;
        }
        JAK_CONSOLE_OUTPUT_ON()

        jak_archive archive;
        jak_archive_info info;
        if ((jak_archive_open(&archive, pathCarbonFileIn)) != true) {
            JAK_CONSOLE_WRITE(file, "Cannot open requested CARBON file: %s", pathCarbonFileIn);
            return false;
        } else {
            jak_archive_get_info(&info, &archive);
            FILE *f = fopen(pathCarbonFileIn, "r");
            fseek(f, 0, SEEK_END);
            jak_offset_t fileLength = ftell(f);
            fclose(f);
            printf("file:\t\t\t'%s'\n", pathCarbonFileIn);
            printf("file-size:\t\t%" PRIu64 " B\n", fileLength);
            printf("string-table-size:\t%zu B\n", info.string_table_size);
            printf("record-table-size:\t%zu B\n", info.record_table_size);
            printf("index-size:\t\t%zu B\n", info.string_id_index_size);
            printf("#-embedded-strings:\t%" PRIu32 "\n", info.num_embeddded_strings);
        }
    }

    return true;
}

bool moduleCab2JsInvoke(int argc, char **argv, FILE *file, struct cmdopt_mgr *manager)
{
    JAK_UNUSED(argc);
    JAK_UNUSED(argv);
    JAK_UNUSED(file);
    JAK_UNUSED(manager);

    if (argc != 1) {
        JAK_CONSOLE_WRITE(file, "Require exactly one <input> parameter for <args>.%s", "");
        JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
        JAK_CONSOLE_WRITELN(file, "Run '%s' to see an example on the usage.", "$ ark-carbon to_json");
        return false;
    } else {
        const int filePathArgIdx = 0;
        const char *pathCarbonFileIn = argv[filePathArgIdx];

        JAK_CONSOLE_OUTPUT_OFF()
        if (testFileExists(file, pathCarbonFileIn, 1, 1, true) != true) {
            JAK_CONSOLE_OUTPUT_ON()
            JAK_CONSOLE_WRITELN(file, "Input file cannot be found. %s", "STOP.");
            return false;
        }
        JAK_CONSOLE_OUTPUT_ON()

        jak_archive archive;
        int status;
        if ((status = jak_archive_open(&archive, pathCarbonFileIn))) {
            struct jak_encoded_doc_list collection;
            jak_archive_converter(&collection, &archive);
            encoded_doc_collection_print(stdout, &collection);
            printf("\n");
            encoded_doc_collection_drop(&collection);
        } else {
            error_print(archive.err.code);
        }

        jak_archive_close(&archive);

        return true;
    }
}

bool moduleListInvoke(int argc, char **argv, FILE *file, struct cmdopt_mgr *manager)
{
    JAK_UNUSED(manager);

    if (argc != 1) {
        JAK_CONSOLE_WRITE(file, "Require one constant for <args> parameter.%s", "");
        JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
        JAK_CONSOLE_WRITELN(file, "Run '%s' to see an example on the usage.", "$ types list");
        return false;
    } else {
        const char *constant = argv[0];
        if (strcmp(constant, "compressors") == 0) {
            for (size_t i = 0; i < JAK_ARRAY_LENGTH(compressor_strategy_register); i++) {
                JAK_CONSOLE_WRITELN(file, "%s", compressor_strategy_register[i].name);
            }
        } else {
            JAK_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
            JAK_CONSOLE_WRITELN(file, "Constant '%s' is not known.", constant);
            return false;
        }

        return true;
    }
}
