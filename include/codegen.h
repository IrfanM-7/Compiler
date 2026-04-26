#ifndef NOVA_CODEGEN_H
#define NOVA_CODEGEN_H

#include "ast.h"

void generateCode(Program* program, const char* outputFilename);
void generateCCode(Program* program, const char* outputFilename);

#endif
