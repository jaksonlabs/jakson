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

#ifndef JAK_OPT_H
#define JAK_OPT_H

// ---------------------------------------------------------------------------------------------------------------------
//  includes
// ---------------------------------------------------------------------------------------------------------------------

#include <jak_stdinc.h>
#include <jak_vector.h>

typedef struct jak_command_opt {
        char *opt_name;
        char *opt_desc;
        char *opt_manfile;
        int (*callback)(int argc, char **argv, FILE *file);
} jak_command_opt;

typedef struct jak_command_opt_group {
        struct jak_vector ofType(jak_command_opt) cmd_options;
        char *desc;
} jak_command_opt_group;

typedef enum jak_module_arg_policy {
        JAK_MOD_ARG_REQUIRED, JAK_MOD_ARG_NOT_REQUIRED, JAK_MOD_ARG_MAYBE_REQUIRED,
} jak_module_arg_policy;

typedef struct jak_command_opt_mgr {
        struct jak_vector ofType(jak_command_opt_group) groups;
        jak_module_arg_policy policy;
        bool (*fallback)(int argc, char **argv, FILE *file, jak_command_opt_mgr *manager);
        char *module_name;
        char *module_desc;
} jak_command_opt_mgr;

bool jak_opt_manager_create(jak_command_opt_mgr *manager, char *module_name, char *module_desc, jak_module_arg_policy policy, bool (*fallback)(int argc, char **argv, FILE *file, jak_command_opt_mgr *manager));
bool jak_opt_manager_drop(jak_command_opt_mgr *manager);

bool jak_opt_manager_process(jak_command_opt_mgr *manager, int argc, char **argv, FILE *file);
bool jak_opt_manager_create_group(jak_command_opt_group **group, const char *desc, jak_command_opt_mgr *manager);
bool opt_group_add_cmd(jak_command_opt_group *group, const char *opt_name, char *opt_desc, char *opt_manfile, int (*callback)(int argc, char **argv, FILE *file));
bool jak_opt_manager_show_help(FILE *file, jak_command_opt_mgr *manager);

#endif
