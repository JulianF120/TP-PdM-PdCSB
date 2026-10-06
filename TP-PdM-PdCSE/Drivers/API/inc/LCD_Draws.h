/*
 * LCD_Draws.h
 *
 *  Created on: 4 oct. 2026
 *      Author: julia
 */

#ifndef API_INC_LCD_DRAWS_H_
#define API_INC_LCD_DRAWS_H_

typedef struct {
    uint8_t width;        // Ancho en columnas (eje X)
    uint8_t pages;        // Alto en páginas (cada página = 8 píxeles verticales)
    const uint8_t *data;  // Puntero al mapa de bits
} OLED_Bitmap_t;

OLED_Bitmap_t get_bm_num_1();
OLED_Bitmap_t get_bm_alrm2();

#endif /* API_INC_LCD_DRAWS_H_ */
