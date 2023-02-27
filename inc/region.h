/**
* Module including constants for regions
* It may be hard to parse all parameters for the regions in commandline,
* especially if region number goes to very large, so hardcode here.
*/
#ifndef _REGION_H
#define _REGION_H

#include <stdint.h>

// Region count
extern const uint32_t REGION_CNT;

// A matrix with shape nxn, where n is the region count. Entry (i,j) includes
// an integer that represents the mean service time for server in region i to
// serve the job that comes from region j. The service time follows an
// exponential distribution given the mean.
extern const uint32_t MEAN_SERVICE_TIME[2][2];

#endif
