//
// Created by marten on 3/30/19.
//

#ifndef NG5_SLICE_MEASURE_H

#define NG5_SLICE_MEASURE_H

// Time elapsed spend on scanning slices
extern _Atomic u_int32_t timeElapsedScanning;

// Time elapsed spend on sealing slices
extern _Atomic u_int32_t  timeElapsedSealing;

#define ENABLE_MEASURE_TIME_SCANNED 1

#define ENABLE_MEASURE_TIME_SEALED 1



#endif //NG5_SLICE_MEASURE_H
