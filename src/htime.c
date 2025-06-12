#include <htime.h>

// -----------------------------------------------------------------------------
// Глобальный «тиковый» счётчик. Аналог BIOS-переменной tick.
STATIC
VOLATILE
UINT64  UefiTick = 0;

// -----------------------------------------------------------------------------
// Колбэк-функция, вызываемая каждый раз, когда срабатывает таймер.
// Просто инкрементируем UefiTick.
STATIC
VOID
EFIAPI
TimerTickCallback(
  IN EFI_EVENT  Event,
  IN VOID      *Context
  )
{
  UefiTick++;
  (VOID)Event;
  (VOID)Context;
}

// -----------------------------------------------------------------------------
// Инициализация «тикера» с частотой freq тиков в секунду.
// freq — сколько «тиков» будет за 1 секунду.
// Возвращает EFI_SUCCESS или ошибку Boot Services.
EFI_STATUS
InitUefiTicker(
  IN UINTN  Freq
  )
{
  EFI_STATUS  Status;
  EFI_EVENT   TimerEvent;
  UINT64      Period100ns;

  if (Freq == 0) {
    return EFI_INVALID_PARAMETER;
  }

  // 1 секунда = 10 000 000 единиц по 100 нс.
  // Чтобы получить freq тиков/сек, период (в 100нс) = 10 000 000 / freq.
  Period100ns = 10000000ULL / Freq;

  // 1) Создаём событие с типом EVT_TIMER + EVT_NOTIFY_SIGNAL,
  //    чтобы в колбэке вызывался TimerTickCallback.
  Status = gBS->CreateEvent(
             EVT_TIMER | EVT_NOTIFY_SIGNAL, // тип события
             TPL_CALLBACK,                  // уровень TPL для колбэка
             TimerTickCallback,             // указатель на функцию-обработчик
             NULL,                          // контекст (не используем)
             &TimerEvent                    // адрес, куда запишут дескриптор события
           );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  // 2) Устанавливаем периодическое срабатывание:
  //    каждый Period100ns * 100 нс таймер «звенит» и вызывает колбэк.
  /*Status = gBS->SetTimer(
             TimerEvent,
             TimerPeriodic,      // флаг «повторяющийся таймер»
             Period100ns         // период в единицах по 100 нс
           );*/
    Status = gBS->SetTimer(
             TimerEvent,
             TimerPeriodic,      // флаг «повторяющийся таймер»
             0         // период в единицах по 100 нс
           );
  if (EFI_ERROR(Status)) {
    // Если не удалось задать таймер, закрываем событие, чтобы не утек ресурс
    gBS->CloseEvent(TimerEvent);
    return Status;
  }

  return EFI_SUCCESS;
}

// -----------------------------------------------------------------------------
// Возвращает текущее значение «тиков» (начиная от 0, с момента InitUefiTicker).
STATIC
UINT64
GetUefiTicks(
  VOID
  )
{
  return UefiTick;
}

// -----------------------------------------------------------------------------
// Аналог BIOS-функции sleep: ждем, пока не пройдёт Timesleep тиков.
// Если Timesleep=5 и мы инициализировали ticker с Freq=1000,
// то 1 тик = 1 мс, и SleepTicks(5) «спит» примерно 5 мс.
STATIC
VOID
SleepTicks(
  IN UINT64  Timesleep
  )
{
  UINT64  Start = GetUefiTicks();
  while ((GetUefiTicks() - Start) < Timesleep) {
    // Опционально вставляем CpuPause(), чтобы снизить энергопотребление и
    // дать чипу понять, что мы в «busy-loop». CpuPause определяется в BaseLib.
    //CpuPause();
  }
}