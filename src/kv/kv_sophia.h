#pragma once

//===	Includes

#include <stdlib.h>
#include <stdio.h>
#include <alloca.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#include <ftw.h>
#include <unistd.h>

#include <sophia/sophia.h>

#include "backend.h"
#include "../xm.h"

//===	Types

typedef struct {
	void	*env;
	void	*ctl;
	void	*db;
} kv_sophia;
