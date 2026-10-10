/*
 * OLED_Draws.h
 *
 *  Created on: 4 oct. 2026
 *      Author: Julian Ferreira
 */

#ifndef API_INC_OLED_DRAWS_H_
#define API_INC_OLED_DRAWS_H_

#include "general_def.h"

#define DIGIT_WIDTH		16		// columnas de cada dígito
#define DIGIT_PAGES		3		// 24 px de alto
#define DIGIT_SIZE		( DIGIT_WIDTH * DIGIT_PAGES )

#define COLON_WIDTH		8		// columnas de los dos puntos
#define COLON_SIZE		( COLON_WIDTH * DIGIT_PAGES )

#define ICON_WIDTH		16		// columnas del ícono de alarma
#define ICON_PAGES		3
#define ICON_SIZE		( ICON_WIDTH * ICON_PAGES )

typedef struct {
    uint8_t width;        // Ancho en columnas (eje X)
    uint8_t pages;        // Alto en páginas (cada página = 8 píxeles verticales)
    const uint8_t *data;  // Puntero al mapa de bits
} OLED_Bitmap_t;

OLED_Bitmap_t get_bm_num_1( void );
OLED_Bitmap_t get_bm_num_2( void );
OLED_Bitmap_t get_bm_num_3( void );
OLED_Bitmap_t get_bm_num_4( void );
OLED_Bitmap_t get_bm_num_5( void );
OLED_Bitmap_t get_bm_num_6( void );
OLED_Bitmap_t get_bm_num_7( void );
OLED_Bitmap_t get_bm_num_8( void );
OLED_Bitmap_t get_bm_num_9( void );
OLED_Bitmap_t get_bm_num_0( void );
OLED_Bitmap_t get_bm_colon( void );
OLED_Bitmap_t get_bm_alarm_ring( void );
OLED_Bitmap_t getDigitBitmap( uint8_t number );

#endif /* API_INC_OLED_DRAWS_H_ */
