#ifndef DEBUG_MACROS_H
#define DEBUG_MACROS_H

#define GET_VAR_NAME_STR(var) #var
#define DEBUG_PRINT printf("%s:%s:%i still allive ############\n", __FILE__, __func__, __LINE__)

#endif /* DEBUG_MACROS_H */
