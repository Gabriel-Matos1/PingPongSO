// PingPongOS - PingPong Operating System
// Gerência básica do tempo.

#include "time.h"
#include "hardware/cpu.h"
#include "tcb.h"
#include "task.h"

static unsigned int tempo_sistema = 0;
static void tick_handler(int signum)
{
    tempo_sistema++;

    if (task_atual == NULL ||
        task_atual->tipo_user_ou_sistema == 0 ||
        task_atual->status != TASK_EXECUTANDO) {
        return;
    }

    if (task_atual->quantum <= 0)
        return;

    task_atual->quantum--;

    if (task_atual->quantum == 0)
        task_yield();
}
void time_init(){
    tempo_sistema = 0;

    hw_irq_handle(IRQ_TIMER, tick_handler);
    hw_timer(TICK, TICK);
}

void time_term(){
   
}

unsigned int time(){
    return tempo_sistema;
}