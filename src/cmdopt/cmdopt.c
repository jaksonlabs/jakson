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

#include "cmdopt/cmdopt.h"

static carbon_cmdopt_t *findOptionByName(carbon_cmdopt_mgr_t *manager, const char *name);

bool carbon_cmdopt_mgr_create(carbon_cmdopt_mgr_t *manager, char *moduleName, char *moduleDesc,
                              carbon_mod_arg_policy_e policy, bool (*fallback)(int argc, char **argv, FILE *file,
                                                                               carbon_cmdopt_mgr_t *manager))
{
    CARBON_NON_NULL_OR_ERROR(manager)
    CARBON_NON_NULL_OR_ERROR(moduleName)
    CARBON_NON_NULL_OR_ERROR(fallback)
    manager->module_name = strdup(moduleName);
    manager->module_desc = moduleDesc ? strdup(moduleDesc) : NULL;
    manager->policy = policy;
    manager->fallback = fallback;
    CARBON_CHECK_SUCCESS(VectorCreate(&manager->groups, NULL, sizeof(carbon_cmdopt_group_t), 5));
    return true;
}

bool carbon_cmdopt_mgr_drop(carbon_cmdopt_mgr_t *manager)
{
    CARBON_NON_NULL_OR_ERROR(manager);
    for (size_t i = 0; i < manager->groups.numElems; i++) {
        carbon_cmdopt_group_t *cmdGroup = VECTOR_GET(&manager->groups, i, carbon_cmdopt_group_t);
        for (size_t j = 0; j < cmdGroup->cmd_options.numElems; j++) {
            carbon_cmdopt_t *option = VECTOR_GET(&cmdGroup->cmd_options, j, carbon_cmdopt_t);
            free(option->opt_name);
            free(option->opt_desc);
            free(option->opt_manfile);
        }
        CARBON_CHECK_SUCCESS(VectorDrop(&cmdGroup->cmd_options));
        free(cmdGroup->desc);
    }

    CARBON_CHECK_SUCCESS(VectorDrop(&manager->groups));
    free(manager->module_name);
    if (manager->module_desc) {
        free(manager->module_desc);
    }
    return true;
}

bool carbon_cmdopt_mgr_process(carbon_cmdopt_mgr_t *manager, int argc, char **argv, FILE *file)
{
    CARBON_NON_NULL_OR_ERROR(manager)
    CARBON_NON_NULL_OR_ERROR(argv)
    CARBON_NON_NULL_OR_ERROR(file)

    if (argc == 0) {
        if (manager->policy == CARBON_MOD_ARG_REQUIRED) {
            carbon_cmdopt_mgr_show_help(file, manager);
        } else {
            return manager->fallback(argc, argv, file, manager);
        }
    } else {
        const char *arg = argv[0];
        carbon_cmdopt_t *option = findOptionByName(manager, arg);
        if (option) {
            return option->callback(argc - 1, argv + 1, file);
        } else {
            return manager->fallback(argc, argv, file, manager);
        }
    }

    return true;
}

bool carbon_cmdopt_mgr_create_group(carbon_cmdopt_group_t **group,
                                    const char *desc,
                                    carbon_cmdopt_mgr_t *manager)
{
    CARBON_NON_NULL_OR_ERROR(group)
    CARBON_NON_NULL_OR_ERROR(desc)
    CARBON_NON_NULL_OR_ERROR(manager)
    carbon_cmdopt_group_t *cmdGroup = VECTOR_NEW_AND_GET(&manager->groups, carbon_cmdopt_group_t);
    cmdGroup->desc = strdup(desc);
    CARBON_CHECK_SUCCESS(VectorCreate(&cmdGroup->cmd_options, NULL, sizeof(carbon_cmdopt_t), 10));
    *group = cmdGroup;
    return true;
}

bool carbon_cmdopt_group_add_cmd(carbon_cmdopt_group_t *group, const char *opt_name, char *opt_desc, char *opt_manfile,
                                 int (*callback)(int argc, char **argv, FILE *file))
{
    CARBON_NON_NULL_OR_ERROR(group)
    CARBON_NON_NULL_OR_ERROR(opt_name)
    CARBON_NON_NULL_OR_ERROR(opt_desc)
    CARBON_NON_NULL_OR_ERROR(opt_manfile)
    CARBON_NON_NULL_OR_ERROR(callback)

    carbon_cmdopt_t *command = VECTOR_NEW_AND_GET(&group->cmd_options, carbon_cmdopt_t);
    command->opt_desc = strdup(opt_desc);
    command->opt_manfile = strdup(opt_manfile);
    command->opt_name = strdup(opt_name);
    command->callback = callback;

    return true;
}

bool carbon_cmdopt_mgr_show_help(FILE *file, carbon_cmdopt_mgr_t *manager)
{
    CARBON_NON_NULL_OR_ERROR(file)
    CARBON_NON_NULL_OR_ERROR(manager)

    if (manager->groups.numElems > 0) {
        fprintf(file, "usage: %s <command> %s\n\n", manager->module_name,
                (manager->policy == CARBON_MOD_ARG_REQUIRED ? "<args>" :
                 manager->policy == CARBON_MOD_ARG_MAYBE_REQUIRED ? "[<args>]":
                 ""));

        if (manager->module_desc) {
            fprintf(file, "%s\n\n", manager->module_desc);
        }
        fprintf(file, "These are common commands used in various situations:\n\n");
        for (size_t i = 0; i < manager->groups.numElems; i++) {
            carbon_cmdopt_group_t *cmdGroup = VECTOR_GET(&manager->groups, i, carbon_cmdopt_group_t);
            fprintf(file, "%s\n", cmdGroup->desc);
            for (size_t j = 0; j < cmdGroup->cmd_options.numElems; j++) {
                carbon_cmdopt_t *option = VECTOR_GET(&cmdGroup->cmd_options, j, carbon_cmdopt_t);
                fprintf(file, "   %-15s%s\n", option->opt_name, option->opt_desc);
            }
            fprintf(file, "\n");
        }
        fprintf(file, "\n'%s help' show this help, and '%s help <command>' open manpage of\nthe specific command.\n",
                manager->module_name, manager->module_name);
    } else {
        fprintf(file, "usage: %s %s\n\n", manager->module_name,
                (manager->policy == CARBON_MOD_ARG_REQUIRED ? "<args>" :
                 manager->policy == CARBON_MOD_ARG_MAYBE_REQUIRED ? "[<args>]":
                 ""));

        fprintf(file, "%s\n\n", manager->module_desc);
    }

    return true;
}

static carbon_cmdopt_t *findOptionByName(carbon_cmdopt_mgr_t *manager, const char *name)
{
    for (size_t i = 0; i < manager->groups.numElems; i++) {
        carbon_cmdopt_group_t *cmdGroup = VECTOR_GET(&manager->groups, i, carbon_cmdopt_group_t);
        for (size_t j = 0; j < cmdGroup->cmd_options.numElems; j++) {
            carbon_cmdopt_t *option = VECTOR_GET(&cmdGroup->cmd_options, j, carbon_cmdopt_t);
            if (strcmp(option->opt_name, name) == 0) {
                return option;
            }
        }
    }
    return NULL;
}
