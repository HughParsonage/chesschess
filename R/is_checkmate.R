#' Is this checkmate?
#' @param x Positions of white, given as a character vector
#' @param y Positions of black
#' @param start Either 0 meaning a blank board, plus the positions given by \code{x, y}
#' or \code{1} meaning the starting positions plus \code{x, y}
#' @param white_to_move Is white to move (i.e. is white in checkmate?)
#' @param last_move A string in quasi-algebraic notation, differing in that both
#' the destination and origin positions must be given, e.g. e2e4, Qa1a7. Defaults
#' to the first position of x and y but unlikely to be correct in general.
#' @export

is_checkmate <- function(x, y, start = 0L, white_to_move = TRUE, last_move = ifelse(white_to_move, y[1], x[1])) {
  .Call("C_isCheckmate", x, y, start, white_to_move, last_move, PACKAGE = packageName())
}
