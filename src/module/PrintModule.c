#include <PrintModule.h>

HalSMNull hPMnull;
HalSMVariable nullPM={&hPMnull,HalSMVariableType_HalSMNull};

HalSMCModule* PrintModule_init(HalSMCompiler* hsmc) {
    HalSMCModule* module=HalSMCModule_init(hsmc->memory,U8H("PrintModule"));
    
    //Functions
    PutDictElementToDict(hsmc,module->lfuncs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("printAt")),
        HalSMVariable_init(hsmc->memory,
            HalSMFunctionC_init(hsmc,PrintModule_printAt)
        ,HalSMVariableType_HalSMFunctionC)
    ));
    PutDictElementToDict(hsmc,module->lfuncs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("clear")),
        HalSMVariable_init(hsmc->memory,
            HalSMFunctionC_init(hsmc,PrintModule_clear)
            ,HalSMVariableType_HalSMFunctionC)
    ));
    PutDictElementToDict(hsmc,module->lfuncs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("enableCursor")),
        HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,
            PrintModule_enableCursor)
            ,HalSMVariableType_HalSMFunctionC)
    ));
    PutDictElementToDict(hsmc,module->lfuncs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("getAttr")),
        HalSMVariable_init(hsmc->memory,
            HalSMFunctionC_init(hsmc,PrintModule_getAttr),
            HalSMVariableType_HalSMFunctionC)
    ));

    //Variables
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("BLACK")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_BLACK))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("BLUE")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_BLUE))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("GREEN")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_GREEN))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("CYAN")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_CYAN))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("RED")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_RED))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("MAGENTA")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_MAGENTA))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("BROWN")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_BROWN))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("LIGHTGRAY")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_LIGHTGRAY))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("BRIGHT")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_BRIGHT))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("DARKGRAY")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_DARKGRAY))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("LIGHTBLUE")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_LIGHTBLUE))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("LIGHTGREEN")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_LIGHTGREEN))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("LIGHTCYAN")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_LIGHTCYAN))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("LIGHTRED")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_LIGHTRED))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("LIGHTMAGENTA")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_LIGHTMAGENTA))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("YELLOW")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_YELLOW))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("WHITE")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_WHITE))
    ));

    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("BGBLACK")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_BACKGROUND_BLACK))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("BGBLUE")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_BACKGROUND_BLUE))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("BGGREEN")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_BACKGROUND_GREEN))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("BGCYAN")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_BACKGROUND_CYAN))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("BGRED")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_BACKGROUND_RED))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("BGMAGENTA")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_BACKGROUND_MAGENTA))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("BGBROWN")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_BACKGROUND_BROWN))
    ));
    PutDictElementToDict(hsmc,module->vrs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("BGLIGHTGRAY")),
        HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_BACKGROUND_LIGHTGRAY))
    ));

    return module;
}

HalSMVariable* PrintModule_printAt(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    if(args->size>=4&&args->arr[1]->type==HalSMVariableType_int&&args->arr[2]->type==HalSMVariableType_int&&args->arr[3]->type==HalSMVariableType_str) {
        ui32 column,row;
        WCHAR* text;
        
        column=HalSMInteger_ToUnsignedInteger((HalSMInteger*)args->arr[1]->value);
        row=HalSMInteger_ToUnsignedInteger((HalSMInteger*)args->arr[2]->value);
        text=HU16(*(HalStringFormatStr*)args->arr[3]->value);
        gST->ConOut->SetCursorPosition(gST->ConOut,column,row);
        
        //Color
        if(args->size==5&&args->arr[4]->type==HalSMVariableType_int) {
            ui64 attr;
            attr=HalSMInteger_ToUnsignedLongLongInteger((HalSMInteger*)args->arr[4]->value);
            gST->ConOut->SetAttribute(gST->ConOut,attr);
        }

        gST->ConOut->OutputString(gST->ConOut,text);
        
        if(args->size==5&&args->arr[4]->type==HalSMVariableType_int) {
            //RETURN DEFAULT
            gST->ConOut->SetAttribute(gST->ConOut,EFI_TEXT_ATTR(EFI_LIGHTGRAY,EFI_BLACK));
        }
    }
    return &HALSMNULLVAR;
}

HalSMVariable* PrintModule_clear(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    gST->ConOut->ClearScreen(gST->ConOut);
    return &HALSMNULLVAR;
}

HalSMVariable* PrintModule_enableCursor(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    if(args->size==2&&args->arr[1]->type==HalSMVariableType_HalSMBool) {
        gST->ConOut->EnableCursor(gST->ConOut,*(ui8*)args->arr[1]->value);
    }
    return &HALSMNULLVAR;
}

HalSMVariable* PrintModule_getAttr(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    if(args->size==3&&args->arr[1]->type==HalSMVariableType_int&&args->arr[2]->type==HalSMVariableType_int) {
        ui32 f,b;
        f=HalSMInteger_ToUnsignedInteger((HalSMInteger*)args->arr[1]->value);
        b=HalSMInteger_ToUnsignedInteger((HalSMInteger*)args->arr[2]->value);
        return HalSMVariable_init_int(hsmc->memory,HalSMInteger_FromUnsignedInteger(hsmc->memory,EFI_TEXT_ATTR(f,b)));
    }
    return &HALSMNULLVAR;
}