

game2outcome <- function(x, y) {
  .Call("C_game2outcome", x, y, PACKAGE = packageName())
}
