// file: descent.c

#include "descent.h"

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E S
//
// ----------------------------------n-----------------------------------------------------------------------------------

typedef struct {
  double load_weight;
  double processor_weight;
  double batch_weight;
  double file_weight;
} weights;

weights weight;

void DescentInit() {
  weight = (weights) {0.1, 0.1, 0.1, 0.1};
}

size_t DescentCalculate(context system_context) {

  return system_context.processor_number;
}
