#include <gtest/gtest.h>

#include <fstream>
#include <unordered_map>

#include <test-constants.h>
#include "carbon/carbon.h"

static std::string const SAMPLE_JSON_PATH = TEST_CONSTANT_ASSETS_PATH "/sample.json";

static std::string const json_content(std::string const & filename) {
    std::ifstream file(filename);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return content;
}

static std::string const encode_decode_content(std::string const name, carbon_compressor_type_e compressor, std::unordered_map<std::string, std::string> options) {
    carbon_archive_t archive_encoding;
    carbon_err_t err;
    carbon_hashmap_t opts = carbon_hashmap_new();

    for(auto const &it : options) {
        carbon_hashmap_put(opts, it.first.data(), strdup(it.second.data()));
    }

    carbon_archive_callback_t cb = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

    std::string const carbon_output_path = SAMPLE_JSON_PATH + "-" + name + ".carbon";
    std::string const json_output_path   = SAMPLE_JSON_PATH + "-" + name + ".decoded.json";
    std::string const json_data          = json_content(SAMPLE_JSON_PATH);
    if (!carbon_archive_from_json(
                &archive_encoding, carbon_output_path.data(),
                &err, json_data.data(), compressor, opts,
                CARBON_STRDIC_TYPE_SYNC, 0, false, false, &cb
        )) {
        carbon_error_print_and_abort(&err);
    } else {
        carbon_archive_close(&archive_encoding);
    }


    carbon_archive_t archive_decoding;
    int status;
    if ((status = carbon_archive_open(&archive_decoding, carbon_output_path.data()))) {
        carbon_encoded_doc_collection_t collection;
        carbon_archive_converter(&collection, &archive_decoding);

        FILE *file = fopen(json_output_path.data(), "w");
        carbon_encoded_doc_collection_print(file, &collection);
        fclose(file);

        carbon_encoded_doc_collection_drop(&collection);
    } else {
        CARBON_PRINT_ERROR(archive_decoding.err.code);
    }

    carbon_archive_close(&archive_decoding);

    std::string const decoded_content = json_content(json_output_path);

    remove(json_output_path.data());
    remove(carbon_output_path.data());

    carbon_hashmap_drop(opts);

    return decoded_content;
}

std::string const compressor_none_content() {
    static std::string content = "";
    if (content.empty()) {
        content = encode_decode_content("none", CARBON_COMPRESSOR_NONE, {});
    }

    return content;
}

TEST(AutoCompressor, TestBruteForceWithSampling) {
    std::string const compressor_auto_content = encode_decode_content("auto-brute-force-with-sampling", CARBON_COMPRESSOR_AUTO, {{ "sampling.enabled", "true" }, { "selector", "brute-force" }});

    EXPECT_EQ(compressor_none_content(), compressor_auto_content);
}

TEST(AutoCompressor, TestBruteForceWithoutSampling) {
    std::string const compressor_auto_content = encode_decode_content("auto-brute-force-without-sampling", CARBON_COMPRESSOR_AUTO, {{ "sampling.enabled", "false" }, { "selector", "brute-force" }});

    EXPECT_EQ(compressor_none_content(), compressor_auto_content);
}

TEST(AutoCompressor, TestCostModelWithSampling) {
    std::string const compressor_auto_content = encode_decode_content("auto-cost-model-with-sampling", CARBON_COMPRESSOR_AUTO, {{ "sampling.enabled", "true" }, { "selector", "cost-model" }});

    EXPECT_EQ(compressor_none_content(), compressor_auto_content);
}

TEST(AutoCompressor, TestCostModelWithoutSampling) {
    std::string const compressor_auto_content = encode_decode_content("auto-cost-model-without-sampling", CARBON_COMPRESSOR_AUTO, {{ "sampling.enabled", "false" }, { "selector", "cost-model" }});

    EXPECT_EQ(compressor_none_content(), compressor_auto_content);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
