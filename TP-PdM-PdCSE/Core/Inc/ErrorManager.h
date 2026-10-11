/**
 * @file ErrorManager.h
 * @brief Error handling of the application.
 * @author Julian Ferreira
 * @date 09/10/2026
 */

#ifndef INC_ERRORMANAGER_H_
#define INC_ERRORMANAGER_H_

/**
 * @brief Handles an unrecoverable error by resetting the microcontroller.
 *
 * This function does not return.
 */
void Error_Handler(void);

#endif /* INC_ERRORMANAGER_H_ */
