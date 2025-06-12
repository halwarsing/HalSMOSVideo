#ifndef PRINTMODULE_H
#define PRINTMODULE_H

#include <efi.h>
#include <efilib.h>
#include <HalSM.h>

HalSMCModule* PrintModule_init(HalSMCompiler* hsmc);
HalSMVariable* PrintModule_printAt(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* PrintModule_clear(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* PrintModule_enableCursor(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* PrintModule_getAttr(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);

#endif