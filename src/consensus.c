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

#define CHPRINTF_USE_LONG_LONG TRUE
#include <stdint.h>
#include "consensus.h"
#include "chprintf.h"


// Serializa un int32 a 4 bytes en orden little-endian
void int32_to_bytes(uint8_t out[4], const int32_t value) {
    out[0] = (uint8_t)((uint32_t)value & 0xFF);
    out[1] = (uint8_t)(((uint32_t)value >> 8) & 0xFF);
    out[2] = (uint8_t)(((uint32_t)value >> 16) & 0xFF);
    out[3] = (uint8_t)(((uint32_t)value >> 24) & 0xFF);
}

// Reconstruye un int32 a partir de 4 bytes little-endian
int32_t bytes_to_int32(const uint8_t in[4]) {
    uint32_t temp = ((uint32_t)in[3] << 24) |
                    ((uint32_t)in[2] << 16) |
                    ((uint32_t)in[1] << 8)  |
                    ((uint32_t)in[0]);
    return (int32_t)temp;
}


// Hilo de consenso
THD_FUNCTION(CONSENSUS, arg) {
    (void)arg;

    chThdSleepMilliseconds(200);

    bool  first = false;        // Bloquea el control hasta recibir el primer dato válido
    int comunication = 0;

    const dw_addr_t self_addr = dw_get_addr();     // dirección UWB de este nodo
    const dw_addr_t FIRST     = 3213;              // dirección del nodo que habla primero
    const dw_addr_t SECOND    = 18;                // dirección del nodo que habla segundo

    dw_recv_info_t rcv;

    const sysinterval_t tmo_op = TIME_MS2I(100);   // timeout de las operaciones send/recv

    uint8_t N_neighbors = 1;

    int32_t time = 0;                       // reloj local x_{ii}
    int32_t time_neighbors[N_neighbors];    // relojes de los vecinos x_{ij}
    int32_t integral_control  = 0;          // estado integral w(k)

    int32_t neighbordiff_curr_time = 0;     // diferencia de reloj actual x(k)
    int32_t neighbordiff_last_time = 0;     // diferencia de reloj previa x(k-1)

    uint32_t last_cont = 0;                 // valor de 'cont' en la iteración anterior

    for(size_t i = 0; i < N_neighbors; i++)
        time_neighbors[i] = 0;

    // Ganancias del control PI
    float Ki = 0.0001f;
    float Kp = 0.5f;

    BaseSequentialStream* const chp = (BaseSequentialStream*)&SD1; 

    static uint8_t rxbuf[4] = {0,0,0,0};
    const size_t RXBUF_SIZE = sizeof rxbuf;

    static uint8_t txbuf[4] = {0,0,0,0};
    const size_t TXBUF_SIZE = sizeof txbuf; 


    while (true) {
        // Lectura del reloj local
        chSysLock();
        time = clock_time;
        chSysUnlock();

        int32_to_bytes(txbuf, time);    // prepara el timestamp local para enviar

        // Comunicación UWB
        if(self_addr == FIRST) {
            (void)dw_send_tmo(SECOND, txbuf, sizeof(time), tmo_op);

            rcv = dw_recv_tmo(&SECOND, rxbuf, RXBUF_SIZE, tmo_op);
            if (rcv.state != DW_RECV_OK) {     
                neighbordiff_last_time = neighbordiff_curr_time;
                chThdSleepMilliseconds(20);
                continue;
            }
        }
        else if (self_addr == SECOND) {
            rcv = dw_recv_tmo(&FIRST, rxbuf, RXBUF_SIZE, tmo_op);
            if (rcv.state != DW_RECV_OK) {
                neighbordiff_last_time = neighbordiff_curr_time;
                chThdSleepMilliseconds(20);
                continue;
            }

            (void)dw_send_tmo(FIRST, txbuf, sizeof(time), tmo_op);
        }

        // Lectura del contador de ticks del reloj
        chSysLock();
        uint32_t local_cont = cont;
        chSysUnlock();

        // Ticks transcurridos desde la última iteración
        uint32_t elapsed_ticks = local_cont - last_cont;
        last_cont = local_cont;

        // Reloj del vecino recibido
        time_neighbors[0] = bytes_to_int32(rxbuf);

        // Activa el control solo tras el primer timestamp válido del vecino
        if(!first && time_neighbors[0]!=0){
            first = true;
            comunication = 1;
        }

        neighbordiff_curr_time = time - time_neighbors[0]; // x(k)

        // Estado integral: w(k) = w(k-1) + Ki * x(k-1) * elapsed_ticks
        if(first){
            float temp = Ki * (float)neighbordiff_last_time * (float)elapsed_ticks;
            integral_control += (int32_t)temp;
        }

        // Ley de control PI: u(k) = -Kp * x(k) - w(k)
        int32_t copy_control = 0;
        if(first){
            copy_control = (int32_t)(-Kp * (float)neighbordiff_curr_time) - integral_control;
        }

        // Actualización del control
        chSysLock();
        control = copy_control;
        chSysUnlock();

        // Log por UART: time, vecino, x(k), u(k), w(k), cont
        chprintf(chp,
            "%d %d %d %d %d %d \r\n",
            time,
            time_neighbors[0],
            neighbordiff_curr_time,
            copy_control,
            integral_control,
            local_cont
        );

        neighbordiff_last_time = neighbordiff_curr_time;  // actualiza x(k) -> x(k-1)

        chThdSleepMilliseconds(5);
    }
}