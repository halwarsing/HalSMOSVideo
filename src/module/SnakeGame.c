#include <SnakeGame.h>
#include <htime.h>

HalSMCModule* SnakeGame_init(HalSMCompiler* hsmc) {
    HalSMCModule* module=HalSMCModule_init(hsmc->memory,U8H("SnakeGame"));
    
    //Functions
    PutDictElementToDict(hsmc,module->lfuncs,DictElementInit(hsmc->memory,
        HalSMVariable_init_str(hsmc->memory,U8H("start")),
        HalSMVariable_init(hsmc->memory,HalSMFunctionC_init(hsmc,SnakeGame_start),HalSMVariableType_HalSMFunctionC)
    ));

    return module;
}

static unsigned short snakeBody[2000];

void SnakeGame_printAt(CHAR16* str,ui32 c,ui32 r,ui32 attr)
{
    gST->ConOut->SetCursorPosition(gST->ConOut,c,r);
    gST->ConOut->SetAttribute(gST->ConOut,attr);
    gST->ConOut->OutputString(gST->ConOut,str);
}

HalSMVariable* SnakeGame_start(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars) {
    EFI_STATUS    Status;
    EFI_INPUT_KEY Key;

    unsigned char posSnakeX=40;
    unsigned char posSnakeY=13;
    unsigned char directionSnake=1;
    UINTN startSnakeTicks=0;
    UINTN MAX_COLS, MAX_ROWS, MAX_LEN;

    ui32 ATTR_HEAD, ATTR_BODY, ATTR_FRUIT;

    ATTR_HEAD=EFI_TEXT_ATTR(EFI_YELLOW,EFI_BLACK);
    ATTR_BODY=EFI_TEXT_ATTR(EFI_GREEN,EFI_BLACK);
    ATTR_FRUIT=EFI_TEXT_ATTR(EFI_RED,EFI_BLACK);

    gST->ConOut->EnableCursor(gST->ConOut,FALSE);

    snakeBody[0]=*(unsigned short*)((unsigned char[]){39,13});
    snakeBody[1]=*(unsigned short*)((unsigned char[]){38,13});
    snakeBody[2]=*(unsigned short*)((unsigned char[]){37,13});
    unsigned short sizeSnakeBody=3,indexSnakeBody;

    gST->ConOut->QueryMode(gST->ConOut,gST->ConOut->Mode->Mode,&MAX_COLS,&MAX_ROWS);

    MAX_LEN=MAX_COLS*MAX_ROWS;

    unsigned short posFruit=GetUefiTicks()%MAX_LEN;

    while (1) {
        Status=gST->ConIn->ReadKeyStroke(gST->ConIn,&Key);

        if ((Key.ScanCode==SCAN_LEFT||Key.UnicodeChar==L'a')&&directionSnake!=1) {
            directionSnake=0;
        }
        if ((Key.ScanCode==SCAN_RIGHT||Key.UnicodeChar==L'd')&&directionSnake!=0) {
            directionSnake=1;
        }
        if ((Key.ScanCode==SCAN_UP||Key.UnicodeChar==L'w')&&directionSnake!=3) {
            directionSnake=2;
        }
        if ((Key.ScanCode==SCAN_DOWN||Key.UnicodeChar==L's')&&directionSnake!=2) {
            directionSnake=3;
        }

        if (GetUefiTicks()-startSnakeTicks>=10) {
            for (indexSnakeBody=sizeSnakeBody-1;indexSnakeBody>0;indexSnakeBody--) {
                snakeBody[indexSnakeBody]=snakeBody[indexSnakeBody-1];
            }
            snakeBody[0]=*(unsigned short*)((unsigned char[]){posSnakeX,posSnakeY});

            if (directionSnake==0) {
                if (posSnakeX==0) {
                    posSnakeX=MAX_COLS;
                }
                posSnakeX--;
            } else if (directionSnake==1) {
                posSnakeX++;
                if (posSnakeX>=MAX_COLS) {
                    posSnakeX=0;
                }
            } else if (directionSnake==2) {
                if (posSnakeY==0) {
                    posSnakeY=MAX_ROWS;
                }
                posSnakeY--;
            } else if (directionSnake==3) {
                posSnakeY++;
                if (posSnakeY>=MAX_ROWS) {
                    posSnakeY=0;
                }
            }

            if (posSnakeX==posFruit%MAX_COLS&&posSnakeY==posFruit/MAX_COLS) {
                posFruit=GetUefiTicks()%MAX_LEN;
                snakeBody[sizeSnakeBody]=snakeBody[sizeSnakeBody-1];
                sizeSnakeBody++;
            }
            gST->ConOut->ClearScreen(gST->ConOut);

            for (indexSnakeBody=0;indexSnakeBody<sizeSnakeBody;indexSnakeBody++) {
                SnakeGame_printAt(L"@",snakeBody[indexSnakeBody]&0xff,(snakeBody[indexSnakeBody]>>8)&0xff,ATTR_BODY);
                if ((posSnakeX==(snakeBody[indexSnakeBody]&0xff))&&(posSnakeY==((snakeBody[indexSnakeBody]>>8)&0xff))) {
                    /*clear_screen();
                    enable_cursor(14,15);*/

                    gST->ConOut->ClearScreen(gST->ConOut);
                    gST->ConOut->EnableCursor(gST->ConOut,TRUE);
                    gST->ConOut->SetCursorPosition(gST->ConOut,0,0);
                    gST->ConOut->SetAttribute(gST->ConOut,EFI_TEXT_ATTR(EFI_LIGHTGRAY,EFI_BLACK));
                    return &HALSMNULLVAR;
                }
            }

            SnakeGame_printAt(L"@",posSnakeX,posSnakeY,ATTR_HEAD);

            SnakeGame_printAt(L"%",posFruit%MAX_COLS,posFruit/MAX_COLS,ATTR_FRUIT);

            startSnakeTicks=GetUefiTicks();
        }
    }

    return &HALSMNULLVAR;
}