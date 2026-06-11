marker <- file.path("inst", "modules", ".hannlp-generated")
if (file.exists(marker)) {
  unlink(file.path("inst", "modules"), recursive = TRUE, force = TRUE)
}
