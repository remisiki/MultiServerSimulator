/**
* Module implementing policies (or queueing algorithms)
* Possible policies:
* - fcfsLocal: First Come First Serve, all jobs are served locally (jobs from
*   region A will not be served in region B)
*/
#ifndef _POLICY_H
#define _POLICY_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <gsl/gsl_rng.h>
#include "job.h"
#include "queue.h"
#include "server.h"
#include "region.h"
#include "param.h"

uint32_t fcfsLocal(Server** servers);

#endif
