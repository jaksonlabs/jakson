// file: descent.c

#include "descent.h"

// ---------------------------------------------------------------------------------------------------------------------
//
//  T Y P E S
//
// ----------------------------------n-----------------------------------------------------------------------------------

typedef struct {
  float load_weight;
  float processor_weight;
  float batch_weight;
  float file_weight;
} weights;

weights weight;
float previous_result;
float learning_rate;

void DescentInit() {
  weight = (weights) {0.01, 1, 0.0, 0.0};
  previous_result = 0;
  learning_rate = 0.0001;
}

size_t DescentCalculate(context system_context) {
  return weight.load_weight * system_context.load + weight.processor_weight * system_context.processor_number + weight.batch_weight * system_context.batch_size + weight.file_weight * system_context.file_size;
}

float delta_weight(float current_value, float performance) {
  return learning_rate * (2 * (float)current_value * ((float)previous_result - (float)performance));
}

void DescentTrain(context system_context, float performance) {
  if (previous_result > 0) {
    weight.load_weight -= delta_weight(system_context.load, performance);
    weight.processor_weight -= delta_weight(system_context.processor_number, performance);
    weight.batch_weight -= delta_weight(system_context.batch_size, performance);
    weight.file_weight -= delta_weight(system_context.file_size, performance);
  }
  previous_result = performance;
}
