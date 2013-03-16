
#ifndef GP_INCLUDE_GP_H
#define GP_INCLUDE_GP_H

#include "common.h"
#include "world.h"
#include "program.h"

void        gp_mutate           (GpWorld *, GpProgram *);
void        gp_cross_homologous (GpProgram *, GpProgram *, GpProgram *, GpProgram *);
void        gp_cross_twopoint   (GpWorld *, GpProgram *, GpProgram *, GpProgram *);

#endif
