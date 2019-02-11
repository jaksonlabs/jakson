
#include <inttypes.h>

#include "carbon/carbon.h"

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
    carbon_json_parse_err_t error_desc;
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
    carbon_err_t err;
    status = carbon_jest_test_doc(&err, &jsonAst);
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
#define JS_2_CAB_OPTION_USE_COMPRESSOR "--compressor="
#define JS_2_CAB_OPTION_USE_COMPRESSOR_HUFFMAN "huffman"

bool moduleJs2CabInvoke(int argc, char **argv, FILE *file, carbon_cmdopt_mgr_t *manager)
{
    CARBON_UNUSED(manager);

    Js2CabContext cabContext;

    if (argc < 2) {
        CARBON_CONSOLE_WRITE(file, "Require at least <output> and <input> parameters for <args>.%s", "");
        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
        CARBON_CONSOLE_WRITELN(file, "Run '%s' to see an example on the usage.", "$ carbon convert");
        return false;
    } else {
        bool flagSizeOptimized = false;
        bool flagReadOptimized = false;
        bool flagForceOverwrite = false;
        carbon_archive_compressor_type_e compressor = CARBON_ARCHIVE_COMPRESSOR_TYPE_NONE;

        int outputIdx = 0, inputIdx = 1;
        int i;

        for (i = 0; i < argc; i++) {
            char *opt = argv[i];
            if (strncmp(opt, "--", 2) == 0) {
                if (strcmp(opt, JS_2_CAB_OPTION_SIZE_OPTIMIZED) == 0) {
                    flagSizeOptimized = true;
                    compressor = CARBON_ARCHIVE_COMPRESSOR_TYPE_HUFFMAN;
                } else if (strcmp(opt, JS_2_CAB_OPTION_READ_OPTIMIZED) == 0) {
                    flagReadOptimized = true;
                } else if (strcmp(opt, JS_2_CAB_OPTION_SILENT_OUTPUT) == 0) {
                    CARBON_CONSOLE_OUTPUT_OFF();
                } else if (strcmp(opt, JS_2_CAB_OPTION_FORCE_OVERWRITE) == 0) {
                    flagForceOverwrite = true;
                } else if (strncmp(opt, JS_2_CAB_OPTION_USE_COMPRESSOR, strlen(JS_2_CAB_OPTION_USE_COMPRESSOR)) == 0) {
                    const char *compressor_name = opt + strlen(JS_2_CAB_OPTION_USE_COMPRESSOR);
                    if (strcmp(compressor_name, JS_2_CAB_OPTION_USE_COMPRESSOR_HUFFMAN) == 0) {
                        compressor = CARBON_ARCHIVE_COMPRESSOR_TYPE_HUFFMAN;
                    } else {
                        CARBON_CONSOLE_WRITE(file, "unsupported compressor requested: '%s'",
                                             compressor_name);
                        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
                        CARBON_CONSOLE_WRITELN(file, "** ERROR ** unsupported operation requested: %s", opt);
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

        if (!flagSizeOptimized && compressor != CARBON_ARCHIVE_COMPRESSOR_TYPE_NONE) {
            CARBON_CONSOLE_WRITELN(file, "** WARNING ** a compressor was specified but will be ignored because size "
                "optimization is turned off. Use '--size-optimized' such that a compressor has any effect%s", "");
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

        if (convertJs2Model(&cabContext, file, flagReadOptimized, pathJsonFileIn, 1, 1) != true) {
            return false;
        }

        carbon_memblock_t *carbonFile;
        CARBON_CONSOLE_WRITE(file, "  - Convert partition into in-memory CARBON file%s", "");
        carbon_err_t err;
        if (!carbon_archive_from_model(&carbonFile, &err, cabContext.partitionMetaModel, compressor)) {
            carbon_error_print_and_abort(&err);
        }
        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");

        CARBON_CONSOLE_WRITE(file, "  - Write in-memory CARBON file to disk%s", "");
        FILE *outputFile = fopen(pathCarbonFileOut, "w");
        if (!outputFile) {
            CARBON_CONSOLE_WRITE(file, "Unable to open file for writing: '%s'", pathCarbonFileOut);
            CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
            return false;
        } else {
            if (carbon_archive_write(outputFile, carbonFile) != true) {
                CARBON_CONSOLE_WRITE(file, "Unable to write to file: '%s'", pathCarbonFileOut);
                CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
                return false;
            }
            fclose(outputFile);
        }
        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");

        CARBON_CONSOLE_WRITE(file, "  - Clean up in-memory CARBON file%s", "");
        carbon_memblock_drop(carbonFile);
        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "OK");

        cleanup(file, &cabContext);

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
        CARBON_CONSOLE_WRITE(file, "Require exactly one argument <carbon-file> that is a path to a carbon file.%s", "");
    } else {
        const char *carbonFilePath = argv[0];
        if (testFileExists(file, carbonFilePath, 1, 1, true) != true) {
            CARBON_CONSOLE_OUTPUT_ON()
            CARBON_CONSOLE_WRITELN(file, "Input file '%s' cannot be found. STOP", carbonFilePath);
            return false;
        }
        carbon_err_t err;
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
        CARBON_CONSOLE_WRITELN(file, "Run '%s' to see an example on the usage.", "$ carbon inspect");
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
        carbon_archive_info_t info;
        if ((carbon_archive_open(&archive, pathCarbonFileIn)) != true) {
            CARBON_CONSOLE_WRITE(file, "Cannot open requested CARBON file: %s", pathCarbonFileIn);
            return false;
        } else {
            carbon_archive_get_info(&info, &archive);
            FILE *f = fopen(pathCarbonFileIn, "r");
            fseek(f, 0, SEEK_END);
            carbon_off_t fileLength = ftell(f);
            fclose(f);
            printf("file;carbon_file_size;string_table_size;record_table_size\n");
            printf("\"%s\";%zu;%zu;%zu\n", pathCarbonFileIn, fileLength, info.string_table_size, info.record_table_size);
        }
    }

    return true;
}

bool moduleCab2JsInvoke(int argc, char **argv, FILE *file, carbon_cmdopt_mgr_t *manager)
{
    CARBON_UNUSED(manager);

    if (argc < 1) {
        CARBON_CONSOLE_WRITE(file, "Require at least <input> parameter for <args>.%s", "");
        CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
        CARBON_CONSOLE_WRITELN(file, "Run '%s' to see an example on the usage.", "$ carbon to_json");
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
        if ((status = carbon_archive_open(&archive, pathCarbonFileIn)) != true) {
            carbon_err_t err;
            carbon_archive_get_error(&err, &archive);
            CARBON_CONSOLE_WRITE(file, "Cannot open requested CARBON file: %s", "");
            CARBON_CONSOLE_WRITE_CONT(file, "[%s]\n", "ERROR");
            switch (err.code) {
            case CARBON_ERR_FOPEN_FAILED:
                CARBON_CONSOLE_WRITELN(file, "   Reason: file cannot be opened%s", "");
                break;
            case CARBON_ERR_IO:
                CARBON_CONSOLE_WRITELN(file, "   Reason: file I/O error%s", "");
                break;
            case CARBON_ERR_FORMATVERERR:
                CARBON_CONSOLE_WRITELN(file, "   Reason: not a CARBON file, or unsupported format version%s", "");
                break;
            case CARBON_ERR_CORRUPTED:
                CARBON_CONSOLE_WRITELN(file, "   Reason: file is corrupted%s", "");
                break;
            default:
                CARBON_CONSOLE_WRITELN(file, "   Reason: no further details or internal error%s", "");
                break;
            }

            CARBON_CONSOLE_WRITELN(file, "Input file '%s' cannot be converted. STOP", pathCarbonFileIn);
            return false;
        }

        carbon_archive_object_t root;
        size_t num_keys;
        const carbon_string_id_t *keys;

        carbon_archive_record(&root, &archive);

        keys = carbon_archive_object_keys_to_type(&num_keys, CARBON_TYPE_OBJECT, &root);
        for (size_t i = 0; i < num_keys; i++) {
            printf("key %"PRIu64"\n", keys[i]);

            carbon_archive_object_t nested;
            carbon_archive_object_values_object(&nested, i, &root);

            //----------------------------------------------------------------------------------------------------------
            size_t numNestedKeys;
            const carbon_string_id_t *keyNames = carbon_archive_object_keys_to_type(&numNestedKeys, CARBON_TYPE_OBJECT, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                printf("  key '%"PRIu64"' maps to obj\n", keyNames[j]);
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_type(&numNestedKeys, CARBON_TYPE_INT8, &nested);
            const carbon_int8_t* valueInt8 = carbon_archive_object_values_int8(NULL, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                printf("  key '%"PRIu64"' maps to int8s: %d\n", keyNames[j], valueInt8[j]);
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_type(&numNestedKeys, CARBON_TYPE_INT16, &nested);
            const carbon_int16_t* valueInt16 = carbon_archive_object_values_int16s(NULL, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                printf("  key '%"PRIu64"' maps to int16s: %d\n", keyNames[j], valueInt16[j]);
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_type(&numNestedKeys, CARBON_TYPE_INT32, &nested);
            const carbon_int32_t* valueInt32 = carbon_archive_object_values_int32(NULL, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                printf("  key '%"PRIu64"' maps to int32s: %"PRIi32"\n", keyNames[j], valueInt32[j]);
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_type(&numNestedKeys, CARBON_TYPE_INT64, &nested);
            const carbon_int64_t* valueInt64 = carbon_archive_object_values_int64s(NULL, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                printf("  key '%"PRIu64"' maps to int64: %"PRIi64"\n", keyNames[j], valueInt64[j]);
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_type(&numNestedKeys, CARBON_TYPE_UINT8, &nested);
            const carbon_uint8_t* valueUInt8 = carbon_archive_object_values_uint8s(NULL, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                printf("  key '%"PRIu64"' maps to uint8s: %d\n", keyNames[j], valueUInt8[j]);
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_type(&numNestedKeys, CARBON_TYPE_UINT16, &nested);
            const carbon_uint16_t* valueUInt16 = carbon_archive_object_values_uin16(NULL, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                printf("  key '%"PRIu64"' maps to uint16s: %d\n", keyNames[j], valueUInt16[j]);
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_type(&numNestedKeys, CARBON_TYPE_UINT32, &nested);
            const carbon_uin32_t* valueUInt32 = carbon_archive_object_values_uint32(NULL, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                printf("  key '%"PRIu64"' maps to uint32s: %d\n", keyNames[j], valueUInt32[j]);
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_type(&numNestedKeys, CARBON_TYPE_UINT64, &nested);
            const carbon_uin64_t* valueUInt64 = carbon_archive_object_values_uint64(NULL, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                printf("  key '%"PRIu64"' maps to uint64s: %"PRIu64"\n", keyNames[j], valueUInt64[j]);
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_type(&numNestedKeys, CARBON_TYPE_BOOL, &nested);
            const carbon_bool_t *valueBoolean = carbon_archive_object_values_bool(NULL, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                printf("  key '%"PRIu64"' maps to booleans: %d\n", keyNames[j], valueBoolean[j]);
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_type(&numNestedKeys,CARBON_TYPE_FLOAT,  &nested);
            const carbon_float_t *valueFloat = carbon_archive_object_values_float(NULL, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                printf("  key '%"PRIu64"' maps to floats: %f\n", keyNames[j], valueFloat[j]);
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_type(&numNestedKeys, CARBON_TYPE_STRING, &nested);
            const carbon_string_id_t *valueStr = carbon_archive_object_values_strings(NULL, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                printf("  key '%"PRIu64"' maps to strings: %"PRIu64"\n", keyNames[j], valueStr[j]);
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_type(&numNestedKeys, CARBON_TYPE_VOID, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                printf("  key '%"PRIu64"' maps to nulls\n", keyNames[j]);
            }

            ////////////////////////////////////////////////////////////////////////////////////////////////////////////

            //----------------------------------------------------------------------------------------------------------
            uint32_t length;

            keyNames = carbon_archive_object_keys_to_array(&numNestedKeys, CARBON_TYPE_INT8, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                const carbon_int8_t *values = carbon_archive_object_values_int8_arrays(&length, j, &nested);
                printf("  key '%"PRIu64"' maps to int8 array\n\t -> [", keyNames[j]);
                for (size_t k = 0; k < length; k++) {
                    printf("%d ", values[k]);
                }
                printf("]\n");
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_array(&numNestedKeys, CARBON_TYPE_INT16, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                const carbon_int16_t *values = carbon_archive_object_values_int16_arrays(&length, j, &nested);
                printf("  key '%"PRIu64"' maps to int16 array\n\t -> [", keyNames[j]);
                for (size_t k = 0; k < length; k++) {
                    printf("%d ", values[k]);
                }
                printf("]\n");
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_array(&numNestedKeys, CARBON_TYPE_INT32, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                const carbon_int32_t *values = carbon_archive_object_values_int32_arrays(&length, j, &nested);
                printf("  key '%"PRIu64"' maps to int32 array\n\t -> [", keyNames[j]);
                for (size_t k = 0; k < length; k++) {
                    printf("%d ", values[k]);
                }
                printf("]\n");
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_array(&numNestedKeys, CARBON_TYPE_INT64, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                const carbon_int64_t *values = carbon_archive_object_values_int64_arrays(&length, j, &nested);
                printf("  key '%"PRIu64"' maps to int64 array\n\t -> [", keyNames[j]);
                for (size_t k = 0; k < length; k++) {
                    printf("%"PRIi64" ", values[k]);
                }
                printf("]\n");
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_array(&numNestedKeys, CARBON_TYPE_UINT8, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                const carbon_uint8_t *values = carbon_archive_object_values_uint8_arrays(&length, j, &nested);
                printf("  key '%"PRIu64"' maps to uint8 array\n\t -> [", keyNames[j]);
                for (size_t k = 0; k < length; k++) {
                    printf("%d ", values[k]);
                }
                printf("]\n");
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_array(&numNestedKeys, CARBON_TYPE_UINT16, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                const carbon_uint16_t *values = carbon_archive_object_values_uint16_arrays(&length, j, &nested);
                printf("  key '%"PRIu64"' maps to uint16 array\n\t -> [", keyNames[j]);
                for (size_t k = 0; k < length; k++) {
                    printf("%d ", values[k]);
                }
                printf("]\n");
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_array(&numNestedKeys, CARBON_TYPE_UINT32, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                const carbon_uin32_t *values = carbon_archive_object_values_uint32_arrays(&length, j, &nested);
                printf("  key '%"PRIu64"' maps to uint32 array\n\t -> [", keyNames[j]);
                for (size_t k = 0; k < length; k++) {
                    printf("%d ", values[k]);
                }
                printf("]\n");
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_array(&numNestedKeys, CARBON_TYPE_UINT64, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                const carbon_uin64_t *values = carbon_archive_object_values_uint64_arrays(&length, j, &nested);
                printf("  key '%"PRIu64"' maps to uint64 array\n\t -> [", keyNames[j]);
                for (size_t k = 0; k < length; k++) {
                    printf("%"PRIu64" ", values[k]);
                }
                printf("]\n");
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_array(&numNestedKeys, CARBON_TYPE_BOOL, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                const carbon_bool_t *values = carbon_archive_object_values_bool_arrays(&length, j, &nested);
                printf("  key '%"PRIu64"' maps to boolean array\n\t -> [", keyNames[j]);
                for (size_t k = 0; k < length; k++) {
                    printf("%d ", values[k]);
                }
                printf("]\n");
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_array(&numNestedKeys, CARBON_TYPE_FLOAT, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                const carbon_float_t *values = carbon_archive_object_values_float_arrays(&length, j, &nested);
                printf("  key '%"PRIu64"' maps to float array\n\t -> [", keyNames[j]);
                for (size_t k = 0; k < length; k++) {
                    printf("%f ", values[k]);
                }
                printf("]\n");
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_array(&numNestedKeys, CARBON_TYPE_STRING, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                const carbon_string_id_t *values = carbon_archive_object_values_string_arrays(&length, j, &nested);
                printf("  key '%"PRIu64"' maps to string array\n\t -> [", keyNames[j]);
                for (size_t k = 0; k < length; k++) {
                    printf("%"PRIu64" ", values[k]);
                }
                printf("]\n");
            }
            //----------------------------------------------------------------------------------------------------------
            keyNames = carbon_archive_object_keys_to_array(&numNestedKeys, CARBON_TYPE_VOID, &nested);
            for (size_t j = 0; j < numNestedKeys; j++) {
                carbon_archive_object_values_null_array_lengths(&length, j, &nested);
                printf("  key '%"PRIu64"' maps to nulls -> %ux\n", keyNames[j], length);
            }


            ////////////////////////////////////////////////////////////////////////////////////////////////////////////

            carbon_archive_table_t table;
            int status = carbon_archive_table_open(&table, &nested);
            for (size_t j = 0; status == true && j < table.ngroups; j++) {
                printf("  key '%"PRIu64"' maps to table\n", table.keys[j]);
                carbon_column_group_t group;
                carbon_archive_table_column_group(&group, j, &table);
                printf("  \tnum cols: %zu\n", group.ncolumns);
                for (size_t k = 0; k < group.ncolumns; k++) {
                    carbon_column_t col;
                    carbon_archive_table_column(&col, k, &group);
                    printf("   \ttype: %d, # entries: %zu\n", col.type, col.nelems);
                    for (size_t l = 0; l < col.nelems; l++) {
                        printf("   \t\t# %zu -> ", l);
                        carbon_field_t field;
                        carbon_archive_table_field_get(&field, l, &col);
                        carbon_field_type_e type;
                        carbon_archive_table_field_type(&type, &field);
                        uint32_t len;
                        switch(type) {
                            case carbon_field_type_null: {
                                carbon_archive_table_field_get_null_array_lengths(&len, &field);
                                printf("%d", len);
                            } break;
                            case carbon_field_type_bool: {
                                const carbon_bool_t *vals = carbon_archive_table_field_get_bool_array(&len, &field);
                                for (size_t x = 0; x < len; x++) {
                                    printf("%d ", vals[x]);
                                }
                            } break;
                            case carbon_field_type_int8: {
                                const carbon_int8_t *vals = carbon_archive_table_field_get_int8_array(&len, &field);
                                for (size_t x = 0; x < len; x++) {
                                    printf("%d ", vals[x]);
                                }
                            } break;
                            case carbon_field_type_int16: {
                                const carbon_int16_t *vals = carbon_archive_table_field_get_int16_array(&len, &field);
                                for (size_t x = 0; x < len; x++) {
                                    printf("%d ", vals[x]);
                                }
                            } break;
                            case carbon_field_type_int32: {
                                const carbon_int32_t *vals = carbon_archive_table_field_get_int32_array(&len, &field);
                                for (size_t x = 0; x < len; x++) {
                                    printf("%d ", vals[x]);
                                }
                            } break;
                            case carbon_field_type_int64: {
                                const carbon_int64_t *vals = carbon_archive_table_field_get_int64_array(&len, &field);
                                for (size_t x = 0; x < len; x++) {
                                    printf("%"PRIi64" ", vals[x]);
                                }
                            } break;
                            case carbon_field_type_uint8: {
                                const carbon_uint8_t *vals = carbon_archive_table_field_get_uint8_array(&len, &field);
                                for (size_t x = 0; x < len; x++) {
                                    printf("%d ", vals[x]);
                                }
                            } break;
                            case carbon_field_type_uint16: {
                                const carbon_uint16_t *vals = carbon_archive_table_field_get_uint16_array(&len, &field);
                                for (size_t x = 0; x < len; x++) {
                                    printf("%d ", vals[x]);
                                }
                            } break;
                            case carbon_field_type_uint32: {
                                const carbon_uin32_t *vals = carbon_archive_table_field_get_uint32_array(&len, &field);
                                for (size_t x = 0; x < len; x++) {
                                    printf("%d ", vals[x]);
                                }
                            } break;
                            case carbon_field_type_uint64: {
                                const carbon_uin64_t *vals = carbon_archive_table_field_get_uint64_array(&len, &field);
                                for (size_t x = 0; x < len; x++) {
                                    printf("%"PRIu64"", vals[x]);
                                }
                            } break;
                            case carbon_field_type_float: {
                                const carbon_float_t *vals = carbon_archive_table_field_get_float_array(&len, &field);
                                for (size_t x = 0; x < len; x++) {
                                    printf("%f ", vals[x]);
                                }
                            } break;
                            case carbon_field_type_string: {
                                const carbon_string_id_t *vals =
                                    carbon_archive_table_field_get_string_array(&len, &field);
                                for (size_t x = 0; x < len; x++) {
                                    printf("%"PRIu64" ", vals[x]);
                                }
                            } break;
                            case carbon_field_type_object: {
                                carbon_object_cursor_t cursor;
                                carbon_archive_table_field_object_cursor_open(&cursor, &field);
                                carbon_archive_object_t *arrayObj;
                                while(carbon_archive_table_field_object_cursor_next(&arrayObj, &cursor)) {
                                    printf("<< object >>");
                                }
                            } break;
                        default: {
                            CARBON_PRINT_ERROR_AND_DIE(CARBON_ERR_INTERNALERR);
                            }
                        }
                        printf("\n");
                    }
                }
            }


            printf("  --> # nested %zu\n", numNestedKeys);
        }


        carbon_archive_close(&archive);

        CARBON_CONSOLE_WRITELN(file, "DEBUG: XXX Okay%s", "");

        return true;
    }
}
