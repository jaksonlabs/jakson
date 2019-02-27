//
// Created by Marcus Pinnecke on 25.02.19.
//

#ifndef LIBCARBON_CLI_H
#define LIBCARBON_CLI_H

#include "../../include/cmdopt/cmdopt.h"

bool moduleCliInvoke(int argc, char **argv, FILE *file, carbon_cmdopt_mgr_t *manager);

#endif //LIBCARBON_CLI_H
