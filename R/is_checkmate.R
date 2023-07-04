

is_checkmate <- function(x, y, start = 0L) {
  .Call("C_isCheckmate", x, y, start, PACKAGE = packageName())
}
