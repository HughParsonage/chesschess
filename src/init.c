#include <R.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>

/* FIXME: 
   Check these declarations against the C/Fortran source code.
*/

/* .Call calls */
extern SEXP C_canEnPassant(SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP C_game2outcome(SEXP, SEXP);
extern SEXP C_isCheckmate(SEXP, SEXP, SEXP, SEXP, SEXP);

static const R_CallMethodDef CallEntries[] = {
    {"C_canEnPassant", (DL_FUNC) &C_canEnPassant, 5},
    {"C_game2outcome", (DL_FUNC) &C_game2outcome, 2},
    {"C_isCheckmate",  (DL_FUNC) &C_isCheckmate,  5},
    {NULL, NULL, 0}
};

void R_init_chesschess(DllInfo *dll)
{
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
