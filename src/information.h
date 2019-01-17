// file: information.h

#ifndef NG5_SYSINFO
#define NG5_SYSINFO

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N C L U D E S
//
// ---------------------------------------------------------------------------------------------------------------------
#include <slog.h>
#include "common.h"

NG5_BEGIN_DECL

// ---------------------------------------------------------------------------------------------------------------------
//
//  M A C R O S
//
// ---------------------------------------------------------------------------------------------------------------------

#ifdef __unix__
#include <sys/sysinfo.h>
#include <sys/stat.h>

#define TOTAL_PROC_NUM sysconf(_SC_NPROCESSORS_ONLN)

struct sysinfo _system_information;
struct stat _file_stat;

#define getsysteminfo(path) \
{ \
  if (0 != sysinfo(&_system_information)) { \
    slog_panic(0, "Could not read in system information"); \
    exit(1); \
  } \
  stat(path, &_file_stat); \
}

#else
// TODO Test for MacOS -- @marcus

#define getsysteminfo(path) \
  { \
  slog_panic(0, "NOT IMPLEMENTED"); \
  exit(1); \
  }

#endif

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E S
//
// ---------------------------------------------------------------------------------------------------------------------

typedef struct {
  float load;
  unsigned long processor_number;
  unsigned long batch_size;
  unsigned long file_size;
} context;

// ---------------------------------------------------------------------------------------------------------------------
//
//  I N T E R F A C E
//
// ---------------------------------------------------------------------------------------------------------------------

void system_info(const char* path, context *system_context, size_t batch_size);

NG5_END_DECL

#endif
