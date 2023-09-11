library(chesschess)
# Placeholder with simple test
expect_equal(1 + 1, 2)

expect_true(chesschess:::is_checkmate(c("Kc1", "Rh7", "Ra8"), "Kc8", white_to_move = FALSE, last_move = "Ra4a8"))

# Complex position, only if not en passant
expect_true(is_checkmate(c("Kb4", "b5", "Qa3", "Bf4", "Rh6", "Qd8"),
                         c("Kb7", "Bg7", "b6", "c5", "Rd5", "Nb2", "Bc2"),
                         white_to_move = TRUE,
                         last_move = "c6c5"))

expect_false(is_checkmate(c("Kb4", "b5", "Qa3", "Bf4", "Rh6", "Qd8"),
                          c("Kb7", "Bg7", "b6", "c5", "Rd5", "Nb2", "Bc2"),
                          white_to_move = TRUE,
                          last_move = "c7c5"))

# Test Queen pinned
expect_true(is_checkmate(c("Qf8", "Qa8", "Qa5", "Ka4"),
                         c("Qc8", "Qc7", "d7", "Kd8"),
                         start = 0L,
                         white_to_move = FALSE,
                         last_move = "Qf1f8"))

expect_false(is_checkmate(c("Kc1", "Rd2", "Rf3"),
                          c("Kc8", "Bd4", "Be4", "Bf4", "Bg4", "Rf1"),
                          white_to_move = TRUE,
                          start = 0L,
                          last_move = "Rf2f1"))

expect_false(is_checkmate(c("Kd1"),
                         c("Bb3", "Bc3", "Ke3"),
                         white_to_move = TRUE,
                         start = 0L,
                         last_move = "Ba2b3"))

expect_true(is_checkmate("Ka1",
                         c("Bc3", "Bc4", "Kc1"),
                         last_move = "Bb4c3"))
expect_false(is_checkmate(c("Ka1", "Na4"),
                          c("Bc3", "Bc4", "Kc1"),
                          last_move = "Bb4c3"))
expect_true(is_checkmate(c("Ka1", "Na4"),
                         c("Bc3", "Bc4", "Kc1", "Ra7"),
                         last_move = "Bb4c3"))
expect_true(is_checkmate(c("Kf7", "Bg7"), c("Kh8", "Nh7"), white_to_move = FALSE, last_move = "Bh6h7"))
expect_true(is_checkmate(c("Ka1", "Rd8"), c("Kg8", "f7", "g7", "h7"), white_to_move = FALSE, last_move = "Rd7d8"))


expect_equal(enpassant(c("e5", "Ke1"), c("f5", "Ke7"), start = 0L, last_move = "f7f5"),
             5)
expect_equal(enpassant(c("e5", "Ke1"), c("f5", "Ke7", "d5"), start = 0L, last_move = "f7f5"),
             5)
expect_equal(enpassant(c("e5", "Ke1"), c("f5", "Ke7", "d5"), start = 0L, last_move = "d7d5"),
             -5)
expect_equal(enpassant(c("a5", "Ke1"), c("f5", "Ke7", "b5"), start = 0L, last_move = "b7b5"),
             1)
expect_equal(enpassant(c("h5", "Ke1"), c("f5", "Ke7", "g5"), start = 0L, last_move = "g7g5"),
             -8)

g2o <- chesschess:::game2outcome
expect_equal(g2o("e4", "e5"), 0L)
expect_equal(g2o(c("f3", "g4"), c("e6", "Qh4")), -1, info = "fools mate")

expect_equal(g2o(c("e4", "Bc4", "Qf3", "Qf7"),
                 c("e5", "Nc6", "d6")),
             1L,
             info = "scholars mate")

# Castling rights
expect_equal(g2o(c("e4", "Nf3", "Bb5", "O-O"),
                 c("e5", "Nc6", "Nf6")),
             0L)
expect_error(g2o(c("e4", "Nf3", "Bb5", "O-O-O"),
                 c("e5", "Nc6", "Nf6")))



