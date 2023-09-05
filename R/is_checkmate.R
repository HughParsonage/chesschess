#' Is this checkmate?
#' @param x Positions of white, given as a character vector
#' @param y Positions of black
#' @param start Either 0 meaning a blank board, plus the positions given by \code{x, y}
#' or \code{1} meaning the starting positions plus \code{x, y}
#' @param white_to_move Is white to move (i.e. is white in checkmate?)
#' @export

is_checkmate <- function(x, y, start = 0L, white_to_move = TRUE) {
  .Call("C_isCheckmate", x, y, start, white_to_move, PACKAGE = packageName())
}
