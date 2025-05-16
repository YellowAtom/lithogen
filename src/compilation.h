#pragma once

#include "config.h"
#include "image.h"
#include "model.h"

void CompileModel(Model& model, const Config* config, const Image& image);
void WriteModel(const char* filePath, const Model& model);