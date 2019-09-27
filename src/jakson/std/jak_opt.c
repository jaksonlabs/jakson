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

#include <jakson/std/jak_opt.h>

static jak_command_opt *option_by_name(jak_command_opt_mgr *manager, const char *name);

bool jak_opt_manager_create(jak_command_opt_mgr *manager, char *module_name, char *module_desc, jak_module_arg_policy policy,
                    bool (*fallback)(int argc, char **argv, FILE *file, jak_command_opt_mgr *manager))
{
        JAK_ERROR_IF_NULL(manager)
        JAK_ERROR_IF_NULL(module_name)
        JAK_ERROR_IF_NULL(fallback)
        manager->module_name = strdup(module_name);
        manager->module_desc = module_desc ? strdup(module_desc) : NULL;
        manager->policy = policy;
        manager->fallback = fallback;
        JAK_CHECK_SUCCESS(jak_vector_create(&manager->groups, NULL, sizeof(jak_command_opt_group), 5));
        return true;
}

bool jak_opt_manager_drop(jak_command_opt_mgr *manager)
{
        JAK_ERROR_IF_NULL(manager);
        for (size_t i = 0; i < manager->groups.num_elems; i++) {
                jak_command_opt_group *cmdGroup = JAK_VECTOR_GET(&manager->groups, i, jak_command_opt_group);
                for (size_t j = 0; j < cmdGroup->cmd_options.num_elems; j++) {
                        jak_command_opt *option = JAK_VECTOR_GET(&cmdGroup->cmd_options, j, jak_command_opt);
                        free(option->opt_name);
                        free(option->opt_desc);
                        free(option->opt_manfile);
                }
                JAK_CHECK_SUCCESS(jak_vector_drop(&cmdGroup->cmd_options));
                free(cmdGroup->desc);
        }

        JAK_CHECK_SUCCESS(jak_vector_drop(&manager->groups));
        free(manager->module_name);
        if (manager->module_desc) {
                free(manager->module_desc);
        }
        return true;
}

bool jak_opt_manager_process(jak_command_opt_mgr *manager, int argc, char **argv, FILE *file)
{
        JAK_ERROR_IF_NULL(manager)
        JAK_ERROR_IF_NULL(argv)
        JAK_ERROR_IF_NULL(file)

        if (argc == 0) {
                if (manager->policy == JAK_MOD_ARG_REQUIRED) {
                        jak_opt_manager_show_help(file, manager);
                } else {
                        return manager->fallback(argc, argv, file, manager);
                }
        } else {
                const char *arg = argv[0];
                jak_command_opt *option = option_by_name(manager, arg);
                if (option) {
                        return option->callback(argc - 1, argv + 1, file);
                } else {
                        return manager->fallback(argc, argv, file, manager);
                }
        }

        return true;
}

bool jak_opt_manager_create_group(jak_command_opt_group **group, const char *desc, jak_command_opt_mgr *manager)
{
        JAK_ERROR_IF_NULL(group)
        JAK_ERROR_IF_NULL(desc)
        JAK_ERROR_IF_NULL(manager)
        jak_command_opt_group *cmdGroup = JAK_VECTOR_NEW_AND_GET(&manager->groups, jak_command_opt_group);
        cmdGroup->desc = strdup(desc);
        JAK_CHECK_SUCCESS(jak_vector_create(&cmdGroup->cmd_options, NULL, sizeof(jak_command_opt), 10));
        *group = cmdGroup;
        return true;
}

bool opt_group_add_cmd(jak_command_opt_group *group, const char *opt_name, char *opt_desc, char *opt_manfile,
                       int (*callback)(int argc, char **argv, FILE *file))
{
        JAK_ERROR_IF_NULL(group)
        JAK_ERROR_IF_NULL(opt_name)
        JAK_ERROR_IF_NULL(opt_desc)
        JAK_ERROR_IF_NULL(opt_manfile)
        JAK_ERROR_IF_NULL(callback)

        jak_command_opt *command = JAK_VECTOR_NEW_AND_GET(&group->cmd_options, jak_command_opt);
        command->opt_desc = strdup(opt_desc);
        command->opt_manfile = strdup(opt_manfile);
        command->opt_name = strdup(opt_name);
        command->callback = callback;

        return true;
}

bool jak_opt_manager_show_help(FILE *file, jak_command_opt_mgr *manager)
{
        JAK_ERROR_IF_NULL(file)
        JAK_ERROR_IF_NULL(manager)

        if (manager->groups.num_elems > 0) {
                fprintf(file,
                        "usage: %s <command> %s\n\n",
                        manager->module_name,
                        (manager->policy == JAK_MOD_ARG_REQUIRED ? "<args>" : manager->policy
                                                                              == JAK_MOD_ARG_MAYBE_REQUIRED
                                                                              ? "[<args>]" : ""));

                if (manager->module_desc) {
                        fprintf(file, "%s\n\n", manager->module_desc);
                }
                fprintf(file, "These are common commands used in various situations:\n\n");
                for (size_t i = 0; i < manager->groups.num_elems; i++) {
                        jak_command_opt_group *cmdGroup = JAK_VECTOR_GET(&manager->groups, i, jak_command_opt_group);
                        fprintf(file, "%s\n", cmdGroup->desc);
                        for (size_t j = 0; j < cmdGroup->cmd_options.num_elems; j++) {
                                jak_command_opt *option = JAK_VECTOR_GET(&cmdGroup->cmd_options, j, jak_command_opt);
                                fprintf(file, "   %-15s%s\n", option->opt_name, option->opt_desc);
                        }
                        fprintf(file, "\n");
                }
                fprintf(file,
                        "\n'%s help' show this help, and '%s help <command>' to open \nmanpage of specific command.\n",
                        manager->module_name,
                        manager->module_name);
        } else {
                fprintf(file,
                        "usage: %s %s\n\n",
                        manager->module_name,
                        (manager->policy == JAK_MOD_ARG_REQUIRED ? "<args>" : manager->policy
                                                                              == JAK_MOD_ARG_MAYBE_REQUIRED
                                                                              ? "[<args>]" : ""));

                fprintf(file, "%s\n\n", manager->module_desc);
        }

        return true;
}

static jak_command_opt *option_by_name(jak_command_opt_mgr *manager, const char *name)
{
        for (size_t i = 0; i < manager->groups.num_elems; i++) {
                jak_command_opt_group *cmdGroup = JAK_VECTOR_GET(&manager->groups, i, jak_command_opt_group);
                for (size_t j = 0; j < cmdGroup->cmd_options.num_elems; j++) {
                        jak_command_opt *option = JAK_VECTOR_GET(&cmdGroup->cmd_options, j, jak_command_opt);
                        if (strcmp(option->opt_name, name) == 0) {
                                return option;
                        }
                }
        }
        return NULL;
}
