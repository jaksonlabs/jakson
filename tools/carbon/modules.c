
#include <inttypes.h>
#include "core/pack/pack.h"
#include "core/carbon/archive_query.h"
#include "core/carbon/archive_int.h"
#include "core/carbon.h"

#include "modules.h"

typedef struct
{
    carbon_strdic_t dictionary;
    carbon_doc_bulk_t context;
    carbon_doc_entries_t *partition;
    carbon_columndoc_t *partitionMetaModel;
    char *jsonContent;

} Js2CabContext;

static int convertJs2Model(Js2CabContext *context, FILE *file, bool optimizeForReads, const char *fileName,
                           size_t fileNum, size_t fileMax)
{

    CARBON_CONSOLE_WRITELN(file, "** Process file %zu of %zu **", fileNum, fileMax);
    CARBON_CONSOLE_WRITELN(file, "%s", fileName);

    CARBON_CONSOLE_WRITE(file, "  - Read contents into memory%s", "");

    FILE *f = fopen(fileName, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    context->jsonContent = malloc(fsize + 1);
    size_t nread = fread(context->jsonContent, fsize, 1, f);
    CARBON_UNUSED(nread);
    fclose(f);
    context->jsonContent[fsize] = 0;

    CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");


    CARBON_CONSOLE_WRITE(file, "  - Setup string dictionary%s", "");

    carbon_strdic_create_async(&context->dictionary, 1000, 1000, 1000, 8, NULL);
    CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");

    CARBON_CONSOLE_WRITE(file, "  - Parse JSON file%s", "");
    carbon_json_parser_t parser;
    carbon_json_parse_err error_desc;
    carbon_json_t jsonAst;
    carbon_json_parser_create(&parser, &context->context);
    int status = carbon_json_parse(&jsonAst, &error_desc, &parser, context->jsonContent);
    if (!status) {
        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
        if (error_desc.token) {
            CARBON_CONSOLE_WRITELN(file, "** ERROR ** Parsing failed: %s\nBut token %s was found in line %u column %u",
                                   error_desc.msg, error_desc.token_type_str, error_desc.token->line,
                                   error_desc.token->column);
        } else {
            CARBON_CONSOLE_WRITELN(file, "** ERROR ** Parsing failed: %s", error_desc.msg);
        }
        return false;
    } else {
        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");
    }

    CARBON_CONSOLE_WRITE(file, "  - Test document restrictions%s", "");
    struct err err;
    status = carbon_json_test_doc(&err, &jsonAst);
    if (!status) {
        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
        carbon_error_print(&err);
        return false;
    } else {
        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");
    }

    CARBON_CONSOLE_WRITE(file, "  - Create bulk insertion bulk%s", "");
    carbon_doc_bulk_create(&context->context, &context->dictionary);
    CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");

    context->partition = carbon_doc_bulk_new_entries(&context->context);

    CARBON_CONSOLE_WRITE(file, "  - Add file to new partition%s", "");
    carbon_doc_bulk_add_json(context->partition, &jsonAst);
    carbon_json_drop(&jsonAst);
    CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");

    CARBON_CONSOLE_WRITE(file, "  - Cleanup reserved memory%s", "");
    carbon_doc_bulk_shrink(&context->context);
    CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");

    CARBON_CONSOLE_WRITE(file, "  - Finalize partition%s", "");
    context->partitionMetaModel =
        carbon_doc_entries_to_columndoc(&context->context, context->partition, optimizeForReads);
    CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");

    return true;
}

static void cleanup(FILE *file, Js2CabContext *context)
{
    CARBON_CONSOLE_WRITE(file, "  - Perform cleanup operations%s", "");
    carbon_strdic_drop(&context->dictionary);
    carbon_doc_bulk_Drop(&context->context);
    carbon_doc_entries_drop(context->partition);
    carbon_columndoc_free(context->partitionMetaModel);
    free(context->jsonContent);
    free(context->partitionMetaModel);
    CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");
}

static int testFileExists(FILE *file, const char *fileName, size_t fileNum, size_t fileMax, bool requireExistence)
{
    CARBON_CONSOLE_WRITE(file, "Check file %zu of %zu", fileNum, fileMax);
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
    CARBON_CONSOLE_WRITE_CONT(file, "[%s]", "ERROR");
    CARBON_CONSOLE_WRITE_ENDL(file);
    CARBON_CONSOLE_WRITELN(file, "** ERROR ** file I/O error for file '%s'", fileName);
    return false;

success:
    CARBON_CONSOLE_WRITE_CONT(file, "[%s]", "OK");
    CARBON_CONSOLE_WRITE_ENDL(file);
    return true;

}

bool moduleCheckJsInvoke(int argc, char **argv, FILE *file, carbon_cmdopt_mgr_t *manager)
{
    CARBON_UNUSED(manager);

    Js2CabContext cabContext;

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

    CARBON_CONSOLE_WRITELN(file, "Input files passed tests%s", "");

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
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Create from model started");
}

static void tracker_end_create_from_model()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Create from model finished");
}

static void tracker_begin_create_from_json()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Create from json started");
}

static void tracker_end_create_from_json()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Create from json finished");
}

static void tracker_begin_archive_stream_from_json()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Create stream from json started");
}

static void tracker_end_archive_stream_from_json()

{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Create stream from json finished");
}
static void tracker_begin_write_archive_file_to_disk()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Write archive to disk started");
}

static void tracker_end_write_archive_file_to_disk()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Write archive to disk finished");
}

static void tracker_begin_load_archive()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Load archive from disk started");
}

static void tracker_end_load_archive()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Load archive from disk finished");
}

static void tracker_begin_setup_string_dictionary()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Setup string dictionary started");
}

static void tracker_end_setup_string_dictionary()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Setup string dictionary finished");
}

static void tracker_begin_parse_json()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Parsing json started");
}

static void tracker_end_parse_json()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Parsing json finished");
}

static void tracker_begin_test_json()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Test document for restrictions started");
}

static void tracker_end_test_json()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Test document for restrictions finished");
}

static void tracker_begin_import_json()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Import document started");
}

static void tracker_end_import_json()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Import document finished");
}

static void tracker_begin_cleanup()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Cleanup started");
}

static void tracker_end_cleanup()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Cleanup finished");
}

static void tracker_begin_write_string_table()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Writing string table started");
}

static void tracker_end_write_string_table()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Writing string table finished");
}

static void tracker_begin_write_record_table()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Writing record table started");
}

static void tracker_end_write_record_table()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Writing record table finished");
}

static void tracker_skip_string_id_index_baking()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Backing indexes skipped");
}

static void tracker_begin_string_id_index_baking()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Backing string id to offset index started");
}

static void tracker_end_string_id_index_baking()
{
    CARBON_CONSOLE_WRITELN(stdout, "%s", "  - Backing string id to offset index finished");
}


bool moduleJs2CabInvoke(int argc, char **argv, FILE *file, carbon_cmdopt_mgr_t *manager)
{
    CARBON_UNUSED(manager);

    if (argc < 2) {
        CARBON_CONSOLE_WRITE(file, "Require at least <output> and <input> parameters for <args>.%s", "");
        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
        CARBON_CONSOLE_WRITELN(file, "Run '%s' to see an example on the usage.", "$ types convert");
        return false;
    } else {
        bool flagSizeOptimized = false;
        bool flagReadOptimized = false;
        bool flagForceOverwrite = false;
        bool flagBakeStringIdIndex = true;
        carbon_compressor_type_e compressor = CARBON_COMPRESSOR_NONE;
        carbon_strdic_type_e dic_type = CARBON_STRDIC_TYPE_ASYNC;
        int string_dic_async_nthreads = 8;

        int outputIdx = 0, inputIdx = 1;
        int i;

        for (i = 0; i < argc; i++) {
            char *opt = argv[i];
            if (strncmp(opt, "--", 2) == 0) {
                if (strcmp(opt, JS_2_CAB_OPTION_SIZE_OPTIMIZED) == 0) {
                    flagSizeOptimized = true;
                    compressor = CARBON_COMPRESSOR_HUFFMAN;
                } else if (strcmp(opt, JS_2_CAB_OPTION_READ_OPTIMIZED) == 0) {
                    flagReadOptimized = true;
                } else if (strcmp(opt, JS_2_CAB_OPTION_NO_STRING_ID_INDEX) == 0) {
                    flagBakeStringIdIndex = false;
                } else if (strcmp(opt, JS_2_CAB_OPTION_SILENT_OUTPUT) == 0) {
                    CARBON_CONSOLE_OUTPUT_OFF();
                } else if (strcmp(opt, JS_2_CAB_OPTION_FORCE_OVERWRITE) == 0) {
                    flagForceOverwrite = true;
                } else if (strcmp(opt, JS_2_CAB_OPTION_USE_COMPRESSOR) == 0 && i++ < argc) {
                    const char *compressor_name = argv[i];
                    if (!carbon_compressor_by_name(&compressor, compressor_name)) {
                        CARBON_CONSOLE_WRITE(file, "unsupported pack requested: '%s'",
                                             compressor_name);
                        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
                        CARBON_CONSOLE_WRITELN(file, "** ERROR ** unsupported operation requested: %s", opt);
                        return false;
                    }
                } else if (strcmp(opt, JS_2_CAB_OPTION_DIC_TYPE) == 0 && i++ < argc) {
                    const char *dic_type_name = argv[i];
                    if (strcmp(dic_type_name, "async") == 0) {
                        dic_type = CARBON_STRDIC_TYPE_ASYNC;
                    } else if (strcmp(dic_type_name, "sync") == 0) {
                        dic_type = CARBON_STRDIC_TYPE_SYNC;
                    } else {
                        CARBON_CONSOLE_WRITE(file, "unsupported dictionary type requested: '%s'",
                                             dic_type_name);
                        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
                        CARBON_CONSOLE_WRITELN(file, "** ERROR ** unsupported operation requested: %s", opt);
                        return false;
                    }
                } else if (strcmp(opt, JS_2_CAB_OPTION_DIC_NTHREADS) == 0 && i++ < argc) {
                    const char *nthreads_str = argv[i];
                    int nthreads_atoid = atoi(nthreads_str);
                    if (nthreads_atoid > 0) {
                        string_dic_async_nthreads = nthreads_atoid;
                    } else {
                        CARBON_CONSOLE_WRITE(file, "not a number or zero threads assigned: '%s'",
                                             nthreads_str);
                        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
                        CARBON_CONSOLE_WRITELN(file, "** ERROR ** thread setting cannot be applied: %s", opt);
                        return false;
                    }
                } else {
                    CARBON_CONSOLE_WRITELN(file, "** ERROR ** unrecognized option '%s'", opt);
                    return false;
                }
            } else {
                break;
            }
        }

        if (!flagSizeOptimized && compressor != CARBON_COMPRESSOR_NONE) {
            CARBON_CONSOLE_WRITELN(file, "** WARNING ** a pack was specified but will be ignored because size "
                "optimization is turned off. Use '--size-optimized' such that a pack has any effect%s", "");
        }

        if (i + 1 >= argc) {
            CARBON_CONSOLE_WRITELN(file, "** ERROR ** require <output> and <input> parameter: %d remain", argc);
            return false;
        } else if (argc - (i + 1) != 1) {
            CARBON_CONSOLE_WRITELN(file, "** ERROR ** unsupported number of arguments: %d provided beyond <input>", argc - (i + 1));
            return false;
        } else {
            outputIdx = i;
            inputIdx = i + 1;
        }

        const char *pathCarbonFileOut = argv[outputIdx];
        const char *pathJsonFileIn = argv[inputIdx];

        if (!flagForceOverwrite && (testFileExists(file, pathCarbonFileOut, 1, 1, false) != true)) {
            CARBON_CONSOLE_WRITELN(file, "Output file already exists. Remove it first, or use --force-overwrite. %s", "STOP.");
            return false;
        }
        if (testFileExists(file, pathJsonFileIn, 1, 1, true) != true) {
            CARBON_CONSOLE_WRITELN(file, "Input file cannot be found. %s", "STOP.");
            return false;
        }

        CARBON_CONSOLE_WRITELN(file, "  - Read contents into memory%s", "");

        FILE *f = fopen(pathJsonFileIn, "rb");
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        char *jsonContent = malloc(fsize + 1);
        size_t nread = fread(jsonContent, fsize, 1, f);
        CARBON_UNUSED(nread);
        fclose(f);
        jsonContent[fsize] = 0;

        carbon_archive_t archive;
        struct err err;

        carbon_archive_callback_t progress_tracker = { 0 };
        progress_tracker.begin_create_from_model = tracker_begin_create_from_model;
        progress_tracker.end_create_from_model = tracker_end_create_from_model;
        progress_tracker.begin_create_from_json = tracker_begin_create_from_json;
        progress_tracker.end_create_from_json = tracker_end_create_from_json;
        progress_tracker.begin_archive_stream_from_json = tracker_begin_archive_stream_from_json;
        progress_tracker.end_archive_stream_from_json = tracker_end_archive_stream_from_json;
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

        if (!carbon_archive_from_json(&archive, pathCarbonFileOut, &err, jsonContent,
                                      compressor, dic_type, string_dic_async_nthreads, flagReadOptimized,
                                      flagBakeStringIdIndex, &progress_tracker)) {
            carbon_error_print_and_abort(&err);
        } else {
            carbon_archive_close(&archive);
        }


        free(jsonContent);

//        carbon_memblock_t *carbonFile;
//        CARBON_CONSOLE_WRITE(file, "  - Convert partition into in-memory CARBON file%s", "");
//        struct err err;
//        if (!carbon_archive_from_model(&carbonFile, &err, cabContext.partitionMetaModel, pack, flagBakeStringIdIndex)) {
//            carbon_error_print_and_abort(&err);
//        }
//        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");
//
//        CARBON_CONSOLE_WRITE(file, "  - Write in-memory CARBON file to disk%s", "");
//        FILE *outputFile = fopen(pathCarbonFileOut, "w");
//        if (!outputFile) {
//            CARBON_CONSOLE_WRITE(file, "Unable to open file for writing: '%s'", pathCarbonFileOut);
//            CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
//            return false;
//        } else {
//            if (carbon_archive_write(outputFile, carbonFile) != true) {
//                CARBON_CONSOLE_WRITE(file, "Unable to write to file: '%s'", pathCarbonFileOut);
//                CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
//                return false;
//            }
//            fclose(outputFile);
//        }
//        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");
//
//        CARBON_CONSOLE_WRITE(file, "  - Clean up in-memory CARBON file%s", "");
//        carbon_memblock_drop(carbonFile);
//        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");
//
//        cleanup(file, &cabContext);

        CARBON_CONSOLE_WRITELN(file, "Conversion successfull%s", "");

        return true;
    }
}

bool moduleViewCabInvoke(int argc, char **argv, FILE *file, carbon_cmdopt_mgr_t *manager)
{
    CARBON_UNUSED(argc);
    CARBON_UNUSED(argv);
    CARBON_UNUSED(file);
    CARBON_UNUSED(manager);

    CARBON_CONSOLE_OUTPUT_OFF()

    if (argc != 1) {
        CARBON_CONSOLE_WRITE(file, "Require exactly one argument <types-file> that is a path to a types file.%s", "");
    } else {
        const char *carbonFilePath = argv[0];
        if (testFileExists(file, carbonFilePath, 1, 1, true) != true) {
            CARBON_CONSOLE_OUTPUT_ON()
            CARBON_CONSOLE_WRITELN(file, "Input file '%s' cannot be found. STOP", carbonFilePath);
            return false;
        }
        struct err err;
        FILE *inputFile = fopen(carbonFilePath, "r");
        carbon_memblock_t *stream;
        carbon_archive_load(&stream, inputFile);
        if (!carbon_archive_print(stdout, &err, stream)) {
            carbon_error_print(&err);
            carbon_error_drop(&err);
        }
        carbon_memblock_drop(stream);
        fclose(inputFile);

    }

    return true;
}

bool moduleInspectInvoke(int argc, char **argv, FILE *file, carbon_cmdopt_mgr_t *manager)
{
    CARBON_UNUSED(manager);

    if (argc < 1) {
        CARBON_CONSOLE_WRITE(file, "Require input file <input> as parameter for <args>.%s", "");
        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
        CARBON_CONSOLE_WRITELN(file, "Run '%s' to see an example on the usage.", "$ types inspect");
        return false;
    } else {
        const int filePathArgIdx = 0;
        const char *pathCarbonFileIn = argv[filePathArgIdx];

        CARBON_CONSOLE_OUTPUT_OFF()
        if (testFileExists(file, pathCarbonFileIn, 1, 1, true) != true) {
            CARBON_CONSOLE_OUTPUT_ON()
            CARBON_CONSOLE_WRITELN(file, "Input file cannot be found. %s", "STOP.");
            return false;
        }
        CARBON_CONSOLE_OUTPUT_ON()

        carbon_archive_t archive;
        struct archive_info info;
        if ((carbon_archive_open(&archive, pathCarbonFileIn)) != true) {
            CARBON_CONSOLE_WRITE(file, "Cannot open requested CARBON file: %s", pathCarbonFileIn);
            return false;
        } else {
            carbon_archive_get_info(&info, &archive);
            FILE *f = fopen(pathCarbonFileIn, "r");
            fseek(f, 0, SEEK_END);
            offset_t fileLength = ftell(f);
            fclose(f);
            printf("file:\t\t\t'%s'\n", pathCarbonFileIn);
            printf("file-size:\t\t%zu B\n", fileLength);
            printf("string-table-size:\t%zu B\n", info.string_table_size);
            printf("record-table-size:\t%zu B\n", info.record_table_size);
            printf("index-size:\t\t%zu B\n", info.string_id_index_size);
            printf("#-embedded-strings:\t%" PRIu32 "\n", info.num_embeddded_strings);
        }
    }

    return true;
}

bool moduleCab2JsInvoke(int argc, char **argv, FILE *file, carbon_cmdopt_mgr_t *manager)
{
    CARBON_UNUSED(argc);
    CARBON_UNUSED(argv);
    CARBON_UNUSED(file);
    CARBON_UNUSED(manager);

    if (argc != 1) {
        CARBON_CONSOLE_WRITE(file, "Require exactly one <input> parameter for <args>.%s", "");
        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
        CARBON_CONSOLE_WRITELN(file, "Run '%s' to see an example on the usage.", "$ types-tool to_json");
        return false;
    } else {
        const int filePathArgIdx = 0;
        const char *pathCarbonFileIn = argv[filePathArgIdx];

        CARBON_CONSOLE_OUTPUT_OFF()
        if (testFileExists(file, pathCarbonFileIn, 1, 1, true) != true) {
            CARBON_CONSOLE_OUTPUT_ON()
            CARBON_CONSOLE_WRITELN(file, "Input file cannot be found. %s", "STOP.");
            return false;
        }
        CARBON_CONSOLE_OUTPUT_ON()

        carbon_archive_t archive;
        int status;
        if ((status = carbon_archive_open(&archive, pathCarbonFileIn))) {
            carbon_encoded_doc_collection_t collection;
            carbon_archive_converter(&collection, &archive);
            carbon_encoded_doc_collection_print(stdout, &collection);
            printf("\n");
            carbon_encoded_doc_collection_drop(&collection);
        } else {
            CARBON_PRINT_ERROR(archive.err.code);
        }

        carbon_archive_close(&archive);

        return true;
    }
}

bool moduleListInvoke(int argc, char **argv, FILE *file, carbon_cmdopt_mgr_t *manager)
{
    CARBON_UNUSED(manager);

    if (argc != 1) {
        CARBON_CONSOLE_WRITE(file, "Require one constant for <args> parameter.%s", "");
        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
        CARBON_CONSOLE_WRITELN(file, "Run '%s' to see an example on the usage.", "$ types list");
        return false;
    } else {
        const char *constant = argv[0];
        if (strcmp(constant, "compressors") == 0) {
            for (size_t i = 0; i < CARBON_ARRAY_LENGTH(carbon_compressor_strategy_register); i++) {
                CARBON_CONSOLE_WRITELN(file, "%s", carbon_compressor_strategy_register[i].name);
            }
        } else {
            CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
            CARBON_CONSOLE_WRITELN(file, "Constant '%s' is not known.", constant);
            return false;
        }

        return true;
    }
}
