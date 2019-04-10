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

#ifndef CMDOPT_H
#define CMDOPT_H

#include "shared/common.h"
#include "stdx/vec.h"

typedef struct carbon_cmdopt_mgr carbon_cmdopt_mgr_t;

typedef struct
{
    char *opt_name;
    char *opt_desc;
    char *opt_manfile;
    int (*callback)(int argc, char **argv, FILE *file);
} carbon_cmdopt_t;

typedef struct
{
    vec_t ofType(carbon_cmdopt_t) cmd_options;
    char *desc;
} carbon_cmdopt_group_t;

typedef enum
{
    CARBON_MOD_ARG_REQUIRED,
    CARBON_MOD_ARG_NOT_REQUIRED,
    CARBON_MOD_ARG_MAYBE_REQUIRED,
} carbon_mod_arg_policy_e;

typedef struct carbon_cmdopt_mgr
{
    vec_t ofType(carbon_cmdopt_group_t) groups;
    carbon_mod_arg_policy_e policy;
    bool (*fallback)(int argc, char **argv, FILE *file, carbon_cmdopt_mgr_t *manager);

    char *module_name;
    char *module_desc;
} carbon_cmdopt_mgr_t;

CARBON_EXPORT(bool)
carbon_cmdopt_mgr_create(carbon_cmdopt_mgr_t *manager, char *module_name, char *module_desc,
                         carbon_mod_arg_policy_e policy, bool (*fallback)(int argc, char **argv, FILE *file,
                                                                          carbon_cmdopt_mgr_t *manager));
CARBON_EXPORT(bool)
carbon_cmdopt_mgr_drop(carbon_cmdopt_mgr_t *manager);

CARBON_EXPORT(bool)
carbon_cmdopt_mgr_process(carbon_cmdopt_mgr_t *manager, int argc, char **argv, FILE *file);

CARBON_EXPORT(bool)
carbon_cmdopt_mgr_create_group(carbon_cmdopt_group_t **group,
                               const char *desc,
                               carbon_cmdopt_mgr_t *manager);
CARBON_EXPORT(bool)
carbon_cmdopt_group_add_cmd(carbon_cmdopt_group_t *group, const char *opt_name, char *opt_desc, char *opt_manfile,
                            int (*callback)(int argc, char **argv, FILE *file));
CARBON_EXPORT(bool)
carbon_cmdopt_mgr_show_help(FILE *file, carbon_cmdopt_mgr_t *manager);

#endif
