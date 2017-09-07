#ifndef __MOCHI_MDCS_H
#define __MOCHI_MDCS_H

#define MDCS_SUCCESS 0
#define MDCS_ERROR  (-1)
#define MDCS_TRUE    1
#define MDCS_FALSE   0

/**
 * Type of a printer function, used by mdcs_set_error_printer
 * and mdcs_set_warning_printer.
 */
typedef void (*mdcs_printer_f)(const char*);

/**
 * Initializes the MDCS service by registering the proper
 * RPCs using the provided margo instance.
 *
 * \param[in] mid Initialized margo instance.
 * \param[in] listening MDCS_TRUE if we are listening for queries.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_init(margo_instance_id mid, int listening);

/**
 * Checks if the MDCS service is initialized.
 *
 * \param[out] flag MDCS_TRUE if the service is initialize, MDCS_FALSE otherwise.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_initialized(int* flag);

/**
 * Finalizes the MDCS service.
 *
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_finalize();

/**
 * Sets the function to use to print errors inside MDCS.
 * 
 * \param[in] fun Function to call to print an error message.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_set_error_printer(mdcs_printer_f fun);

/**
 * Sets the function to use to print warnings inside MDCS.
 * 
 * \param[in] fun Function to call to print a warning message.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_set_warning_printer(mdcs_printer_f fun);

#endif