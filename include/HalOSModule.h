#ifndef HALOSMODULE_H
#define HALOSMODULE_H

#include <efi.h>
#include <efilib.h>
#include <HalSM.h>
#include <print.h>

HalSMCModule* HalOSModule_init(HalSMCompiler* hsmc);
HalSMVariable* HalOSModule_printSystemInfo(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);

#endif