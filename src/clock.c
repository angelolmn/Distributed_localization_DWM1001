/**
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "clock.h"
#define CHPRINTF_USE_FLOAT TRUE
#define CHPRINTF_USE_LONG_LONG TRUE
#include "chprintf.h"

static virtual_timer_t vt_time;             // temporizador virtual
static binary_semaphore_t sem_time;         // semáforo binario para sincronizar ISR e hilo CLOCK
const uint32_t period = TIME_MS2I(1);       // periodo del tick lógico: 1 ms

// Callback del temporizador virtual. Se ejecuta en contexto de ISR
static void timer_cb(void *arg) {
    (void)arg;
    chSysLockFromISR();
    chBSemSignalI(&sem_time);                    // libera el hilo CLOCK en cada tick
    chVTSetI(&vt_time, period, timer_cb, NULL);  // rearma el temporizador 
    chSysUnlockFromISR();
}

// Hilo del reloj
THD_FUNCTION(CLOCK, arg) {
    (void)arg;
    chThdSleepMilliseconds(1);

    const int32_t skew = 1000;                   // velocidad del reloj
    chVTObjectInit(&vt_time);                    // inicializa el temporizador virtual
    chBSemObjectInit(&sem_time, TRUE);           // inicializa el semáforo
    chVTSet(&vt_time, period, timer_cb, NULL);   // arranca el temporizador periódico

    BaseSequentialStream* const chp = (BaseSequentialStream*)&SD1;  

    while (true) {
        chBSemWait(&sem_time);              // espera al siguiente tick
        chSysLock();
        clock_time += skew + control;       // actualiza el reloj
        if(control != 0) control = 0;       // reestablece el control
        chSysUnlock();
        cont++;                             // contador de ticks procesados
    }
}