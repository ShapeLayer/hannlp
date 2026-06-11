root <- normalizePath(".", winslash = "/", mustWork = TRUE)

copy_dir <- function(from, to) {
  if (!dir.exists(from)) {
    stop("required directory does not exist: ", from, call. = FALSE)
  }
  if (dir.exists(to)) {
    unlink(to, recursive = TRUE, force = TRUE)
  }
  dir.create(dirname(to), recursive = TRUE, showWarnings = FALSE)
  ok <- file.copy(from, dirname(to), recursive = TRUE, copy.date = TRUE)
  if (!isTRUE(ok)) {
    stop("failed to stage directory: ", from, call. = FALSE)
  }
}

inst_modules <- file.path(root, "inst", "modules")
unlink(inst_modules, recursive = TRUE, force = TRUE)

copy_dir(
  file.path(root, "modules", "hannanum", "inst", "hannanum-data"),
  file.path(inst_modules, "hannanum", "inst", "hannanum-data")
)

copy_dir(
  file.path(root, "modules", "niadic", "NIADic", "inst"),
  file.path(inst_modules, "niadic", "NIADic", "inst")
)
