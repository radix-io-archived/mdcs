#ifndef __MDCS_ERROR_H
#define __MDCS_ERROR_H

#include <mdcs/mdcs.h>

extern mdcs_printer_f mdcs_print_error;
extern mdcs_printer_f mdcs_print_warning;

#define S(x) #x
#define S_(x) S(x)
#define S__LINE__ S_(__LINE__)

#define MDCS_PRINT_ERROR(msg) mdcs_print_error("[" __FILE__ ":" S__LINE__ "]" msg)

#define MDCS_PRINT_WARNING(msg) mdcs_print_warning("[" __FILE__ ":" S__LINE__ "]" msg)

#endif
