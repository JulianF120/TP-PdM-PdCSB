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
OLED_Bitmap_t get_bm_num_2();
OLED_Bitmap_t get_bm_num_3();
OLED_Bitmap_t get_bm_num_4();
OLED_Bitmap_t get_bm_num_5();
OLED_Bitmap_t get_bm_num_6();
OLED_Bitmap_t get_bm_num_7();
OLED_Bitmap_t get_bm_num_8();
OLED_Bitmap_t get_bm_num_9();
OLED_Bitmap_t get_bm_num_0();
OLED_Bitmap_t get_bm_num_doble_dot();
OLED_Bitmap_t get_bm_alrm2();
OLED_Bitmap_t getNumberDraw( uint8_t number );

#endif /* API_INC_LCD_DRAWS_H_ */
