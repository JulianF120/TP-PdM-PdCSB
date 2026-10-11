/**
 * @file OLED_Draws.h
 * @brief Bitmaps of the digits and icons shown on the OLED display.
 *
 * Bitmaps use the SSD1306 page format: each byte is a column of 8 vertical
 * pixels (bit 0 is the top one), and bytes are stored page by page.
 *
 * @author Julian Ferreira
 * @date 04/10/2026
 */

#ifndef API_INC_OLED_DRAWS_H_
#define API_INC_OLED_DRAWS_H_

#include "general_def.h"

#define DIGIT_WIDTH		16		/**< @brief Width of a digit, in columns. */
#define DIGIT_PAGES		3		/**< @brief Height of a digit, in pages (24 px). */
#define DIGIT_SIZE		( DIGIT_WIDTH * DIGIT_PAGES )	/**< @brief Size of a digit bitmap, in bytes. */

#define COLON_WIDTH		8		/**< @brief Width of the colon, in columns. */
#define COLON_SIZE		( COLON_WIDTH * DIGIT_PAGES )	/**< @brief Size of the colon bitmap, in bytes. */

#define ICON_WIDTH		16		/**< @brief Width of the alarm icon, in columns. */
#define ICON_PAGES		3		/**< @brief Height of the alarm icon, in pages (24 px). */
#define ICON_SIZE		( ICON_WIDTH * ICON_PAGES )	/**< @brief Size of the alarm icon bitmap, in bytes. */

/**
 * @brief Monochrome bitmap in SSD1306 page format.
 */
typedef struct {
	uint8_t width;			/**< Width in columns. */
	uint8_t pages;			/**< Height in pages (each page is 8 pixels high). */
	const uint8_t *data;	/**< Bitmap data, width * pages bytes. */
} OLED_Bitmap_t;

/** @brief Returns the bitmap of the digit 1. */
OLED_Bitmap_t get_bm_num_1( void );
/** @brief Returns the bitmap of the digit 2. */
OLED_Bitmap_t get_bm_num_2( void );
/** @brief Returns the bitmap of the digit 3. */
OLED_Bitmap_t get_bm_num_3( void );
/** @brief Returns the bitmap of the digit 4. */
OLED_Bitmap_t get_bm_num_4( void );
/** @brief Returns the bitmap of the digit 5. */
OLED_Bitmap_t get_bm_num_5( void );
/** @brief Returns the bitmap of the digit 6. */
OLED_Bitmap_t get_bm_num_6( void );
/** @brief Returns the bitmap of the digit 7. */
OLED_Bitmap_t get_bm_num_7( void );
/** @brief Returns the bitmap of the digit 8. */
OLED_Bitmap_t get_bm_num_8( void );
/** @brief Returns the bitmap of the digit 9. */
OLED_Bitmap_t get_bm_num_9( void );
/** @brief Returns the bitmap of the digit 0. */
OLED_Bitmap_t get_bm_num_0( void );
/** @brief Returns the bitmap of the colon that separates hours and minutes. */
OLED_Bitmap_t get_bm_colon( void );
/** @brief Returns the bitmap of the alarm icon (bell with sound waves). */
OLED_Bitmap_t get_bm_alarm_ring( void );

/**
 * @brief Returns the bitmap of a decimal digit.
 *
 * @param[in] number Digit from 0 to 9.
 * @return Bitmap of the digit, or the bitmap of 0 if @p number is not valid.
 */
OLED_Bitmap_t getDigitBitmap( uint8_t number );

#endif /* API_INC_OLED_DRAWS_H_ */
