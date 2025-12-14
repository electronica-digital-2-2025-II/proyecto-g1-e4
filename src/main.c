#include "xparameters.h"
#include <stdint.h>

#define GPIO_BASE       XPAR_AXI_GPIO_0_BASEADDR

// Registros GPIO
#define GPIO_OUT_REG    (*(volatile uint32_t *)(GPIO_BASE + 0x0))  // canal 1: salidas
#define GPIO_IN_REG     (*(volatile uint32_t *)(GPIO_BASE + 0x8))  // canal 2: entradas

// -----------------
// Bits de salida
// -----------------
#define BUZ_A      (1u << 0)
#define BUZ_B      (1u << 1)
#define LED_A_R    (1u << 2)
#define LED_A_G    (1u << 3)
#define LED_B_R    (1u << 4)
#define LED_B_G    (1u << 5)
#define TRIG_A     (1u << 6)
#define TRIG_B     (1u << 7)
#define TRIG_BUS   (1u << 8)

// -----------------
// Bits de entrada (ECHOs)
// -----------------
#define ECHO_A     (1u << 6)   // gpio_in[6]
#define ECHO_B     (1u << 7)   // gpio_in[7]
#define ECHO_BUS   (1u << 8)   // gpio_in[8]

// -----------------
// Umbrales
// -----------------
// BUS: binario (cerca/no cerca)
#define TICKS_UMBRAL_BUS       800u

// PUERTAS: 3 niveles (ajústalo en campo)
#define TICKS_PUERTA_FULL     2000u   // "lleno" (muy cerca)
#define TICKS_PUERTA_MED      6000u   // "medio" (cerca pero no tan pegado)

// -----------------
// Delay bruto
// -----------------
static void delay_cycles(volatile uint32_t n)
{
    while (n--) {
        __asm__("nop");
    }
}

// -----------------
// Medición ultrasónica: devuelve ticks del pulso ECHO (0 si no hubo eco)
// -----------------
static uint32_t ultrasonic_ticks(uint32_t trig_mask, uint32_t echo_mask)
{
    // TRIG en 0
    GPIO_OUT_REG &= ~trig_mask;
    delay_cycles(1000);

    // Pulso TRIG
    GPIO_OUT_REG |= trig_mask;
    delay_cycles(2000);
    GPIO_OUT_REG &= ~trig_mask;

    // Esperar ECHO alto (timeout)
    uint32_t timeout = 200000;
    while (!(GPIO_IN_REG & echo_mask) && timeout--) {
        __asm__("nop");
    }
    if (timeout == 0) {
        return 0; // no llegó eco
    }

    // Contar duración de ECHO alto
    uint32_t ticks = 0;
    while ((GPIO_IN_REG & echo_mask) && (ticks < 200000)) {
        ticks++;
        __asm__("nop");
    }

    return ticks;
}

// -----------------
// BUS cerca: usa un umbral binario (como tu código original)
// Devuelve 1 si está cerca, 0 si no
// -----------------
static int ultrasonic_near(uint32_t trig_mask, uint32_t echo_mask, uint32_t ticks_threshold)
{
    uint32_t ticks = ultrasonic_ticks(trig_mask, echo_mask);
    if (ticks > 0 && ticks < ticks_threshold) return 1;
    return 0;
}

// -----------------
// Nivel de ocupación por puerta (según ticks)
// 0 = libre, 1 = medio, 2 = lleno
// -----------------
static int puerta_level(uint32_t ticks)
{
    if (ticks == 0) return 0; // sin eco => asumimos libre/lejano

    if (ticks < TICKS_PUERTA_FULL) return 2; // lleno
    if (ticks < TICKS_PUERTA_MED)  return 1; // medio
    return 0; // libre
}

int main(void)
{
    GPIO_OUT_REG = 0;

    while (1) {

        // 1) Sensor BUS: habilita o mata el sistema
        int bus_near = ultrasonic_near(TRIG_BUS, ECHO_BUS, TICKS_UMBRAL_BUS);

        uint32_t out = 0;

        if (bus_near) {

            // 2) Medimos ticks y sacamos nivel por puerta
            uint32_t ta = ultrasonic_ticks(TRIG_A, ECHO_A);
            uint32_t tb = ultrasonic_ticks(TRIG_B, ECHO_B);

            int la = puerta_level(ta); // 0 libre, 1 medio, 2 lleno
            int lb = puerta_level(tb);

            // ---------------------------------------------------------
            // REGLA DE COLORES:
            // - Verde: redirigir hacia la puerta más libre
            // - Rojo: la otra puerta está llena (no vaya)
            // - Amarillo (R+G): la otra puerta está medio llena
            // ---------------------------------------------------------

            // Caso: ambas libres
            if (la == 0 && lb == 0) {
                out |= BUZ_A | BUZ_B;
            }
            // A está mejor que B (A más libre)
            else if (la < lb) {
                if (la == 0) out |= BUZ_A;   // buzzer solo si está libre
                out |= LED_B_G;              // flecha en B hacia A (A es mejor)

                // En A muestro el estado de B (el "contrario")
                if (lb == 2) {
                    out |= LED_A_R;          // B lleno => rojo en A
                } else if (lb == 1) {
                    out |= (LED_A_R | LED_A_G); // B medio => amarillo en A
                }
            }
            // B está mejor que A (B más libre)
            else if (lb < la) {
                if (lb == 0) out |= BUZ_B;
                out |= LED_A_G;              // flecha en A hacia B (B es mejor)

                // En B muestro el estado de A (el "contrario")
                if (la == 2) {
                    out |= LED_B_R;          // A lleno => rojo en B
                } else if (la == 1) {
                    out |= (LED_B_R | LED_B_G); // A medio => amarillo en B
                }
            }
            // Iguales (ambas medio o ambas llenas)
            else {
                if (la == 1 && lb == 1) {
                    // Ambas medio: amarillo en ambas
                    out |= (LED_A_R | LED_A_G | LED_B_R | LED_B_G);
                } else {
                    // Ambas llenas: todo apagado (como antes)
                    // out = 0;
                }
            }
        }

        GPIO_OUT_REG = out;
        delay_cycles(100000);
    }

    return 0;
}
