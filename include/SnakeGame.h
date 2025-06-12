#ifndef SNAKEGAME_H
#define SNAKEGAME_H

#include <efi.h>
#include <efilib.h>
#include <HalSM.h>

HalSMCModule* SnakeGame_init(HalSMCompiler* hsmc);
HalSMVariable* SnakeGame_start(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);

#endif