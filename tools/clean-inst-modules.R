# R CMD build invokes cleanup before creating the source tarball.  The staged
# modules are package runtime data, so removing them here would produce a
# tarball that installs successfully but cannot run the analyzer.  Only the
# staging marker is transient.
marker <- file.path("inst", "modules", ".hannlp-generated")
if (file.exists(marker)) {
  unlink(marker, force = TRUE)
}
