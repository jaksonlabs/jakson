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

#ifndef OPT_H
#define OPT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jakson/stdinc.h>
#include <jakson/std/vector.h>

typedef struct command_opt {
        char *opt_name;
        char *opt_desc;
        char *opt_manfile;
        int (*callback)(int argc, char **argv, FILE *file);
} command_opt;

typedef struct command_opt_group {
        vector ofType(command_opt) cmd_options;
        char *desc;
} command_opt_group;

typedef enum module_arg_policy {
        MOD_ARG_REQUIRED, MOD_ARG_NOT_REQUIRED, MOD_ARG_MAYBE_REQUIRED,
} module_arg_policy;

typedef struct command_opt_mgr {
        vector ofType(command_opt_group) groups;
        module_arg_policy policy;
        bool (*fallback)(int argc, char **argv, FILE *file, command_opt_mgr *manager);
        char *module_name;
        char *module_desc;
} command_opt_mgr;

bool opt_manager_create(command_opt_mgr *manager, char *module_name, char *module_desc, module_arg_policy policy, bool (*fallback)(int argc, char **argv, FILE *file, command_opt_mgr *manager));
bool opt_manager_drop(command_opt_mgr *manager);

bool opt_manager_process(command_opt_mgr *manager, int argc, char **argv, FILE *file);
bool opt_manager_create_group(command_opt_group **group, const char *desc, command_opt_mgr *manager);
bool opt_group_add_cmd(command_opt_group *group, const char *opt_name, char *opt_desc, char *opt_manfile, int (*callback)(int argc, char **argv, FILE *file));
bool opt_manager_show_help(FILE *file, command_opt_mgr *manager);

#endif
