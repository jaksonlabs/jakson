#include "information.h"

void system_info(const char* path, context *system_context, size_t batch_size) {
  getsysteminfo(path);
  slog_info(0, "System processors: %zu, Load Average: %.2f, Batch Size: %zu, Complete File Size: %zu", TOTAL_PROC_NUM, (float) _system_information.loads[0] / (float)(1 << SI_LOAD_SHIFT), batch_size, _file_stat.st_size);

  system_context->load = (float) _system_information.loads[0] / (float)(1 << SI_LOAD_SHIFT);
  system_context->processor_number = TOTAL_PROC_NUM;
  system_context->batch_size = batch_size/1000.0f/1000.0f;
  system_context->file_size = _file_stat.st_size/1000.0f/1000.0f;
}
