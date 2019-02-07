#include "carbon/carbon-error.h"
#include "cmdopt/cmdopt.h"

#include "modules.h"

#define DESC_CHECK_JS "Test whether JSON input files given as args are suitable for CARBON."
#define DESC_CHECK_JS_USAGE "Test if input files given via <args> parameter are suitable for CARBON conversion.\n" \
                            "\nEXAMPLE\n\t$ carbon checkjs myjson1.json myjson2.json"

#define DESC_JS2CAB "Convert single JSON file into CARBON format"
#define DESC_JS2CAB_USAGE "The parameter <args> is split into two parts <output> and <input>.\n" \
                          "This command converts a single JSON file <input> into CARBON formatted file <output>.\n\n" \
                          "Optionally, the following options are available that must be defined before <output>:\n\n" \
                          "   --size-optimized           Compress the embedded string dictionary using a particular compressor\n" \
                          "   --compressor=<compressor>  Use <compressor> as compression technique for size optimization.\n" \
                          "                              Available values for <compressor>:\n" \
                          "                                 huffman   Use huffman-code compressor (default)\n" \
                          "   --read-optimized           Sort keys and values during pre-processing for efficient reads\n" \
                          "   --force-overwrite          Overwrite the output file if this file already exists\n" \
                          "   --silent                   Suppresses all outputs to stdout\n" \
                          "\nEXAMPLE\n" \
                          "   $ carbon convert output.carbon input.json\n" \
                          "   $ carbon convert --size-optimized --read-optimized output.cab input.json" \

#define DESC_CAB2JS_INFO  "Convert single CARBON file into JSON and print it to stdout"
#define DESC_CAB2JS_USAGE "The parameter <args> is a path to a CARBON file that is converted JSON and printed on stdout.\n" \
                          "\nEXAMPLE\n" \
                          "   $ carbon to_json inthewoods.carbon\n" \

#define DESC_CAB_VIEW "Print CARBON file in human readable form to stdout"
#define DESC_CAB_INFO "Display information about a CARBON file to stdout"

#define DEFINE_MODULE(moduleName, moduleCommand, desc, invokeFunc)                                              \
static int module##moduleName##Entry(int argc, char **argv, FILE *file)                                         \
{                                                                                                               \
    carbon_cmdopt_mgr_t manager;                                                                        \
    carbon_cmdopt_mgr_create(&manager, moduleCommand, desc, CARBON_MOD_ARG_REQUIRED, invokeFunc);            \
    int status = carbon_cmdopt_mgr_process(&manager, argc, argv, file);                                       \
    carbon_cmdopt_mgr_drop(&manager);                                                                         \
    return status;                                                                                              \
}

DEFINE_MODULE(CheckJs, "checkjs", DESC_CHECK_JS_USAGE, moduleCheckJsInvoke);
DEFINE_MODULE(Js2Cab, "convert", DESC_JS2CAB_USAGE, moduleJs2CabInvoke);

DEFINE_MODULE(ViewCab, "view", DESC_CAB_VIEW, moduleViewCabInvoke);
DEFINE_MODULE(Inspect, "inspect", DESC_CAB_INFO, moduleInspectInvoke);
DEFINE_MODULE(Cab2Js, "to_json", DESC_CAB2JS_USAGE, moduleCab2JsInvoke);

static bool showHelp(int argc, char **argv, FILE *file, carbon_cmdopt_mgr_t *manager);

int main (int argc, char **argv)
{
    CARBON_CONSOLE_OUTPUT_ON();

    carbon_cmdopt_mgr_t manager;
    carbon_cmdopt_group_t *group;

    carbon_cmdopt_mgr_create(&manager, "carbon-tool", "A tool to work with CARBON files.\n"
                                 "Copyright (c) 2018-2019 Marcus Pinnecke (pinnecke@ovgu.de)", CARBON_MOD_ARG_MAYBE_REQUIRED,
                             showHelp);

    carbon_cmdopt_mgr_create_group(&group, "work with JSON files", &manager);
    carbon_cmdopt_group_add_cmd(group,
                                "checkjs", DESC_CHECK_JS,
                                "manpages/carbon/checkjs",
                                moduleCheckJsEntry);
    carbon_cmdopt_group_add_cmd(group,
                                "convert", DESC_JS2CAB,
                                "manpages/carbon/convert",
                                moduleJs2CabEntry);

    carbon_cmdopt_mgr_create_group(&group, "work with CARBON files", &manager);
    carbon_cmdopt_group_add_cmd(group,
                                "view", DESC_CAB_VIEW,
                                "manpages/carbon/view",
                                moduleViewCabEntry);
    carbon_cmdopt_group_add_cmd(group,
                                "inspect", DESC_CAB_INFO,
                                "manpages/carbon/inspect",
                                moduleInspectEntry);
    carbon_cmdopt_group_add_cmd(group,
                                "to_json", DESC_CAB2JS_INFO,
                                "manpages/carbon/to_json",
                                moduleCab2JsEntry);

    int status = carbon_cmdopt_mgr_process(&manager, argc - 1, argv + 1, stdout);
    carbon_cmdopt_mgr_drop(&manager);
    return status ? EXIT_SUCCESS : EXIT_FAILURE;
}

static bool showHelp(int argc, char **argv, FILE *file, carbon_cmdopt_mgr_t *manager)
{
    CARBON_UNUSED(argc);
    CARBON_UNUSED(argv);
    carbon_cmdopt_mgr_show_help(file, manager);
    return true;
}
