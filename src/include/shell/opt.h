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
#include "std/vec.h"

struct carbon_cmdopt
{
    char *opt_name;
    char *opt_desc;
    char *opt_manfile;
    int (*callback)(int argc, char **argv, FILE *file);
};

struct carbon_cmdopt_group
{
    struct vector ofType(struct carbon_cmdopt) cmd_options;
    char *desc;
};

enum carbon_mod_arg_policy
{
    NG5_MOD_ARG_REQUIRED,
    NG5_MOD_ARG_NOT_REQUIRED,
    NG5_MOD_ARG_MAYBE_REQUIRED,
};

struct carbon_cmdopt_mgr
{
    struct vector ofType(struct carbon_cmdopt_group) groups;
    enum carbon_mod_arg_policy policy;
    bool (*fallback)(int argc, char **argv, FILE *file, struct carbon_cmdopt_mgr *manager);

    char *module_name;
    char *module_desc;
};

NG5_EXPORT(bool)
carbon_cmdopt_mgr_create(struct carbon_cmdopt_mgr *manager, char *module_name, char *module_desc,
                         enum carbon_mod_arg_policy policy, bool (*fallback)(int argc, char **argv, FILE *file,
                                                                          struct carbon_cmdopt_mgr *manager));
NG5_EXPORT(bool)
carbon_cmdopt_mgr_drop(struct carbon_cmdopt_mgr *manager);

NG5_EXPORT(bool)
carbon_cmdopt_mgr_process(struct carbon_cmdopt_mgr *manager, int argc, char **argv, FILE *file);

NG5_EXPORT(bool)
carbon_cmdopt_mgr_create_group(struct carbon_cmdopt_group **group,
                               const char *desc,
                               struct carbon_cmdopt_mgr *manager);
NG5_EXPORT(bool)
carbon_cmdopt_group_add_cmd(struct carbon_cmdopt_group *group, const char *opt_name, char *opt_desc, char *opt_manfile,
                            int (*callback)(int argc, char **argv, FILE *file));
NG5_EXPORT(bool)
carbon_cmdopt_mgr_show_help(FILE *file, struct carbon_cmdopt_mgr *manager);

#endif
