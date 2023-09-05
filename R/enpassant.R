#' Enpassant
#' @param x,y,start,white_to_move,last_move See \code{\link{is_checkmate}}.
#' @return
#' An integer: 0 if no enpassant possible; otherwise the column from which it is
#' possible, negative if the capture is to the left, positive if to the right.
#' @export

enpassant <- function(x, y, start = 0L, white_to_move = TRUE, last_move = ifelse(white_to_move, y[1], x[1])) {
  .Call("C_canEnPassant", x, y, start, white_to_move, last_move, PACKAGE = packageName())
}

