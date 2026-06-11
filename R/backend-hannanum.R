.hannlp_hannanum_data_dir <- function() {
  user_data <- .hannlp_user_data_dir(create = FALSE)
  if (dir.exists(user_data)) {
    return(user_data)
  }

  package_data <- system.file("modules", "hannanum", "inst", "hannanum-data", package = "HanNLP")
  if (nzchar(package_data) && dir.exists(package_data)) {
    return(package_data)
  }

  root_data <- file.path(
    normalizePath(file.path(getwd(), ".."), mustWork = FALSE),
    "JHanNanum-0.8.4-en", "JHanNanum", "data"
  )
  if (dir.exists(root_data)) {
    return(root_data)
  }

  local_data <- file.path(getwd(), "JHanNanum-0.8.4-en", "JHanNanum", "data")
  if (dir.exists(local_data)) {
    return(local_data)
  }

  source_data <- file.path(getwd(), "src", "modules", "hannanum", "inst", "hannanum-data")
  if (dir.exists(source_data)) {
    return(source_data)
  }

  stop("HanNanum data directory was not found.", call. = FALSE)
}

.hannlp_preprocess <- function(inputs) {
  if (!is.character(inputs)) {
    warning("Input must be legitimate character!", call. = FALSE)
    return(FALSE)
  }
  new_input <- gsub("[[:space:]]", " ", inputs)
  new_input <- gsub("[[:space:]]+$", "", new_input)
  gsub("^[[:space:]]+", "", new_input)
}

.hannlp_hannanum_analyze_one <- function(sentence, mode, autoSpacing) {
  sentence_pre <- .hannlp_preprocess(sentence)
  if (is.na(sentence_pre) || identical(sentence_pre, FALSE) || identical(sentence_pre, "")) {
    return(sentence)
  }
  if (isTRUE(autoSpacing)) {
    warning("autoSpacing is not implemented in the native HanNanum backend yet.", call. = FALSE)
  }
  .Call("hannlp_hannanum_analyze", sentence_pre, .hannlp_hannanum_data_dir(), mode, PACKAGE = "HanNLP")
}
