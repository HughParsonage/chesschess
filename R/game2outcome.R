

game2outcome <- function(x, y) {
  stopifnot(is.character(x), is.character(y))
  x <- gsub("[x#+]", "", x)
  y <- gsub("[x#+]", "", y)
  .Call("C_game2outcome", x, y, PACKAGE = packageName())
}
