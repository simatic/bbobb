/**
 * @brief Functions used to signal errors in a given file at a given line in a more
 *  portable way than error_at_line function (which is gcc-specific). Moreover, not
 *  using a macro like in errorTrains.h makes it possible to overwrite these error
 *  functions when doing unitary tests.
 * @file bb_error.h
 * @author May CATTANT - Quentin MONNOT - Victor ROUQUETTE - Michel SIMATIC
 * @date 23 february 2016
 */
 
#ifndef _BB_ERROR_H
#define _BB_ERROR_H

/** 
 * @brief Function used to signal errors in a given file at a given
 *  line in a more portable way than error_at_line function (which is
 *  gcc-specific). NB: if @a status has a nonzero value, then error()
 *  calls abort(3) to terminate the program (@a status is unused).
 */
void bbErrorAtLine(int status, int errnum, char *filename, int linenum, char *format,...);

/** 
 * @brief Same as @a bbErrorAtLine but without having to specify an
 * errnum
 */
void bbErrorAtLineWithoutErrnum(int status, char *filename, int linenum, char *format,...);

#endif /* _BB_ERROR_H */
