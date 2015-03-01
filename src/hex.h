#pragma once

//===	Includes

#include <stdint.h>

//===	Functions

void HexFromInt (int64_t i, uint8_t c[17]);
int64_t HexToInt (const uint8_t c[17]);
