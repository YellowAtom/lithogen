// SPDX-License-Identifier: GPL-3.0
#pragma once

#include "config.h"
#include "structures.h"

void CompileModel(Model& model, const Config* config, const Image& image);
void WriteModel(const char* filePath, const Model& model);