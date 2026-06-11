.hannlp_state <- new.env(parent = emptyenv())
.hannlp_state$user_data_dir <- NULL

.hannlp_package_data_dir <- function() {
  package_data <- system.file("hannanum-data", package = "HanNLP")
  if (nzchar(package_data) && dir.exists(package_data)) {
    return(package_data)
  }
  local_data <- file.path(getwd(), "HanNLP", "inst", "hannanum-data")
  if (dir.exists(local_data)) {
    return(local_data)
  }
  local_data <- file.path(getwd(), "inst", "hannanum-data")
  if (dir.exists(local_data)) {
    return(local_data)
  }
  stop("HanNLP bundled dictionary data was not found.", call. = FALSE)
}

.hannlp_user_root <- function() {
  override <- Sys.getenv("HANNLP_USER_DATA_DIR", unset = "")
  if (nzchar(override)) {
    return(override)
  }
  file.path(tools::R_user_dir("HanNLP", "data"), "hannanum-data")
}

.hannlp_optional_namespace <- function(pkg) {
  tryCatch(loadNamespace(pkg), error = function(e) NULL)
}

.hannlp_user_data_dir <- function(create = TRUE) {
  if (!is.null(.hannlp_state$user_data_dir)) {
    return(.hannlp_state$user_data_dir)
  }
  target <- .hannlp_user_root()
  if (!dir.exists(target) && isTRUE(create)) {
    source <- .hannlp_package_data_dir()
    if (!dir.exists(target)) {
      dir.create(target, recursive = TRUE, showWarnings = FALSE)
    }
    entries <- list.files(source, all.files = TRUE, no.. = TRUE, full.names = TRUE)
    ok <- file.copy(entries, target, recursive = TRUE, overwrite = TRUE, copy.date = TRUE)
    if (!all(ok)) {
      stop("failed to initialize HanNLP user dictionary data.", call. = FALSE)
    }
  }
  .hannlp_state$user_data_dir <- target
  target
}

.hannlp_user_dic_path <- function(create = TRUE) {
  file.path(.hannlp_user_data_dir(create = create), "kE", "dic_user.txt")
}

.hannlp_backup_dic_path <- function() {
  file.path(dirname(.hannlp_user_data_dir(create = TRUE)), "backup", "dic_user.txt")
}

.hannlp_read_dic <- function(path) {
  if (!file.exists(path)) {
    stop("dictionary file does not exist: ", path, call. = FALSE)
  }
  if (file.size(path) == 0L) {
    return(data.frame(term = character(), tag = character(), stringsAsFactors = FALSE))
  }
  out <- utils::read.delim(
    path,
    header = FALSE,
    sep = "\t",
    quote = "",
    comment.char = "",
    fileEncoding = "UTF-8",
    stringsAsFactors = FALSE,
    colClasses = "character"
  )
  if (ncol(out) < 2L) {
    stop("invalid dictionary file: ", path, call. = FALSE)
  }
  out <- out[, 1:2, drop = FALSE]
  names(out) <- c("term", "tag")
  Encoding(out$term) <- "UTF-8"
  out
}

.hannlp_category_aliases <- c(
  general = "\uc77c\ubc18",
  chemical = "\ud654\ud559",
  language = "\uc5b8\uc5b4",
  music = "\uc74c\uc545",
  history = "\uc5ed\uc0ac",
  education = "\uad50\uc721",
  "society in general" = "\uc0ac\ud68c \uc77c\ubc18",
  life = "\uc0dd\ud65c",
  physical = "\uccb4\uc721",
  "information and communication" = "\uc815\ubcf4\ud1b5\uc2e0",
  medicine = "\uc758\ud559",
  earth = "\uc9c0\uad6c",
  construction = "\uac74\uc124",
  "veterinary science" = "\uc218\uc758\ud559",
  business = "\uacbd\uc601",
  law = "\ubc95\ub960",
  plant = "\uc2dd\ubb3c",
  buddhism = "\ubd88\uad50",
  "engineering general" = "\uacf5\ud559 \uc77c\ubc18",
  folk = "\ubbfc\uc18d",
  administration = "\ud589\uc815",
  economic = "\uacbd\uc81c",
  math = "\uc218\ud559",
  "korean medicine" = "\ud55c\uc758\ud559",
  military = "\uad70\uc0ac",
  literature = "\ubb38\ud559",
  clothes = "\ubcf5\uc2dd",
  "religion normal" = "\uc885\uad50 \uc77c\ubc18",
  animal = "\ub3d9\ubb3c",
  agriculture = "\ub18d\uc5c5",
  astronomy = "\ucc9c\ubb38",
  transport = "\uad50\ud1b5",
  "natural plain" = "\uc790\uc5f0 \uc77c\ubc18",
  industry = "\uc0b0\uc5c5",
  medium = "\ub9e4\uccb4",
  political = "\uc815\uce58",
  geography = "\uc9c0\ub9ac",
  mining = "\uad11\uc5c5",
  hearing = "\uccad\uac01",
  fishing = "\uc218\uc0b0\uc5c5",
  machinery = "\uae30\uacc4",
  catholic = "\uac00\ud1a8\ub9ad",
  "book title" = "\ucc45\uba85",
  named = "\uace0\uc720\uba85 \uc77c\ubc18",
  "electrical and electronic" = "\uc804\uae30\u00b7\uc804\uc790",
  pharmacy = "\uc57d\ud559",
  "art, music and physical" = "\uc608\uccb4\ub2a5 \uc77c\ubc18",
  useless = "\ubb34\uc6a9",
  ocean = "\ud574\uc591",
  forestry = "\uc784\uc5c5",
  christian = "\uae30\ub3c5\uad50",
  craft = "\uacf5\uc608",
  service = "\uc11c\ube44\uc2a4\uc5c5",
  sports = "\uc6b4\ub3d9",
  food = "\uc2dd\ud488",
  art = "\ubbf8\uc220",
  environment = "\ud658\uacbd",
  video = "\uc601\uc0c1",
  "natural resources" = "\ucc9c\uc5f0\uc790\uc6d0",
  "industry general" = "\uc0b0\uc5c5 \uc77c\ubc18",
  smoke = "\uc5f0\uae30",
  philosophy = "\ucca0\ud559",
  "health general" = "\ubcf4\uac74 \uc77c\ubc18",
  "proper names general" = "\uace0\uc720\uba85 \uc77c\ubc18",
  welfare = "\ubcf5\uc9c0",
  material = "\uc7ac\ub8cc",
  "humanities general" = "\uc778\ubb38 \uc77c\ubc18"
)

.hannlp_normalize_category_names <- function(category_dic_nms) {
  if (length(category_dic_nms) == 0L) {
    return(character())
  }
  out <- as.character(category_dic_nms)
  mapped <- .hannlp_category_aliases[out]
  out[!is.na(mapped)] <- unname(mapped[!is.na(mapped)])
  out
}

.hannlp_read_zip_dic <- function(zip_path, dic_path) {
  if (!file.exists(zip_path)) {
    stop("dictionary zip file does not exist: ", zip_path, call. = FALSE)
  }
  con <- unz(zip_path, dic_path, open = "r", encoding = "UTF-8")
  on.exit(close(con), add = TRUE)
  out <- utils::read.delim(
    con,
    header = FALSE,
    sep = "\t",
    quote = "",
    comment.char = "",
    stringsAsFactors = FALSE,
    colClasses = "character"
  )
  if (ncol(out) < 2L) {
    stop("invalid dictionary inside zip: ", dic_path, call. = FALSE)
  }
  out <- out[, 1:2, drop = FALSE]
  names(out) <- c("term", "tag")
  Encoding(out$term) <- "UTF-8"
  out
}

.hannlp_sejong_zip_path <- function() {
  override <- Sys.getenv("HANNLP_SEJONG_HANDIC", unset = "")
  if (nzchar(override) && file.exists(override)) {
    return(override)
  }
  sejong_pkg <- system.file(package = "Sejong")
  if (nzchar(sejong_pkg)) {
    candidate <- file.path(sejong_pkg, "dics", "handic.zip")
    if (file.exists(candidate)) {
      return(candidate)
    }
  }
  ""
}

.hannlp_read_sejong_dic <- function() {
  zip_path <- .hannlp_sejong_zip_path()
  if (!nzchar(zip_path)) {
    stop("Sejong dictionary not found. Install the Sejong package or set HANNLP_SEJONG_HANDIC to handic.zip.", call. = FALSE)
  }
  .hannlp_read_zip_dic(zip_path, file.path("data", "kE", "dic_user2.txt"))
}

.hannlp_niadic_db_path <- function() {
  override <- Sys.getenv("HANNLP_NIADIC_DB", unset = "")
  if (nzchar(override) && file.exists(override)) {
    return(override)
  }
  niadic_pkg <- system.file(package = "NIADic")
  if (nzchar(niadic_pkg)) {
    candidate <- file.path(niadic_pkg, "hangul.db")
    if (file.exists(candidate)) {
      return(candidate)
    }
  }
  ""
}

.hannlp_niadic_rdata_path <- function() {
  override <- Sys.getenv("HANNLP_NIADIC_RDATA", unset = "")
  if (nzchar(override) && file.exists(override)) {
    return(override)
  }
  niadic_pkg <- system.file(package = "NIADic")
  if (nzchar(niadic_pkg)) {
    candidate <- file.path(niadic_pkg, "dics.RData")
    if (file.exists(candidate)) {
      return(candidate)
    }
  }
  ""
}

.hannlp_read_niadic_table <- function(dic_name, category_dic_nms = "") {
  niadic_ns <- .hannlp_optional_namespace("NIADic")
  if (!is.null(niadic_ns) && exists("get_dic", envir = niadic_ns, inherits = FALSE)) {
    dic <- get("get_dic", envir = niadic_ns)(dic_name)
  } else {
    db_path <- .hannlp_niadic_db_path()
    rdata_path <- .hannlp_niadic_rdata_path()
    if (nzchar(rdata_path)) {
      e <- new.env(parent = emptyenv())
      load(rdata_path, envir = e)
      if (!exists(dic_name, envir = e, inherits = FALSE)) {
        stop(sprintf("NIADic RData does not contain '%s' dictionary!", dic_name), call. = FALSE)
      }
      dic <- get(dic_name, envir = e, inherits = FALSE)
    } else {
      if (!nzchar(db_path)) {
        stop("NIADic dictionary not found. Install NIADic, set HANNLP_NIADIC_RDATA to dics.RData, or set HANNLP_NIADIC_DB to hangul.db.", call. = FALSE)
      }
      rsqlite_ns <- .hannlp_optional_namespace("RSQLite")
      if (is.null(rsqlite_ns)) {
        stop("RSQLite is required to import NIADic hangul.db.", call. = FALSE)
      }
      conn <- get("dbConnect", envir = rsqlite_ns)(get("SQLite", envir = rsqlite_ns)(), db_path)
      on.exit(get("dbDisconnect", envir = rsqlite_ns)(conn), add = TRUE)
      tables <- get("dbListTables", envir = rsqlite_ns)(conn)
      if (!(dic_name %in% tables)) {
        stop(sprintf("NIADic does not contain '%s' dictionary!", dic_name), call. = FALSE)
      }
      dic <- get("dbGetQuery", envir = rsqlite_ns)(conn, sprintf("select * from %s", dic_name))
    }
  }
  if (!all(c("term", "tag") %in% names(dic))) {
    stop("NIADic table does not contain term/tag columns.", call. = FALSE)
  }
  if (identical(dic_name, "woorimalsam") && "category" %in% names(dic)) {
    if (length(category_dic_nms) > 0L && !identical(category_dic_nms, "") && !any(category_dic_nms %in% "all")) {
      categories <- .hannlp_normalize_category_names(category_dic_nms)
      eng_match <- if ("eng_cate" %in% names(dic)) dic$eng_cate %in% category_dic_nms else rep(FALSE, nrow(dic))
      dic <- dic[dic$category %in% categories | eng_match, , drop = FALSE]
    } else if (length(category_dic_nms) == 0L || identical(category_dic_nms, "")) {
      if ("eng_cate" %in% names(dic)) {
        eng_general <- dic$eng_cate %in% "general"
        if (any(eng_general)) {
          dic <- dic[eng_general, , drop = FALSE]
        } else {
          dic <- dic[dic$category %in% .hannlp_category_aliases[["general"]], , drop = FALSE]
        }
      }
    }
  }
  out <- data.frame(term = as.character(dic$term), tag = as.character(dic$tag), stringsAsFactors = FALSE)
  Encoding(out$term) <- "UTF-8"
  out
}

.hannlp_collect_external_dics <- function(ext_dic, category_dic_nms) {
  out <- data.frame(term = character(), tag = character(), stringsAsFactors = FALSE)
  for (dic in unique(ext_dic[nzchar(ext_dic)])) {
    part <- switch(
      dic,
      sejong = .hannlp_read_sejong_dic(),
      insighter = .hannlp_read_niadic_table("insighter", category_dic_nms),
      woorimalsam = .hannlp_read_niadic_table("woorimalsam", category_dic_nms),
      stop(sprintf("No %s dictionary!", dic), call. = FALSE)
    )
    out <- rbind(out, part[, c("term", "tag"), drop = FALSE])
  }
  unique(out)
}

.hannlp_write_user_dic <- function(dic) {
  path <- .hannlp_user_dic_path(create = TRUE)
  if (!dir.exists(dirname(path))) {
    dir.create(dirname(path), recursive = TRUE, showWarnings = FALSE)
  }
  if (nrow(dic) == 0L) {
    file.create(path)
  } else {
    utils::write.table(
      dic[, c("term", "tag"), drop = FALSE],
      file = path,
      quote = FALSE,
      row.names = FALSE,
      col.names = FALSE,
      sep = "\t",
      fileEncoding = "UTF-8"
    )
  }
  invisible(path)
}

.hannlp_normalize_user_dic <- function(user_dic) {
  if (!is.data.frame(user_dic) || ncol(user_dic) != 2L || nrow(user_dic) == 0L) {
    stop("user_dic must be a non-empty two-column data.frame.", call. = FALSE)
  }
  out <- user_dic[, 1:2, drop = FALSE]
  names(out) <- c("term", "tag")
  out$term <- as.character(out$term)
  out$tag <- as.character(out$tag)
  Encoding(out$term) <- "UTF-8"
  invalid_tags <- out$tag[is.na(tags[out$tag])]
  if (length(invalid_tags) > 0L) {
    stop(sprintf("Unsupported tag names: %s", paste(unique(invalid_tags), collapse = ", ")), call. = FALSE)
  }
  out
}

#' Tag name converter
#'
#' @param fromTag tag set name to convert from, either `K` or `S`
#' @param toTag desired tag set name, either `K` or `S`
#' @param tag tag name to search
#' @return converted tag vector
#' @export
convertTag <- function(fromTag, toTag, tag) {
  if (identical(fromTag, toTag) || !(fromTag %in% c("K", "S")) || !(toTag %in% c("K", "S"))) {
    stop("check input parameter!", call. = FALSE)
  }
  dic <- if (identical(fromTag, "K") && identical(toTag, "S")) KtoS else StoK
  unname(dic[tag])
}

#' Reload all dictionaries
#'
#' HanNLP loads dictionaries per analysis call, so this function is retained for KoNLP API compatibility.
#' @export
reloadAllDic <- function() {
  invisible(TRUE)
}

#' Reload user dictionaries for specific functions
#'
#' @param whichDics character vector of dictionary consumers
#' @export
reloadUserDic <- function(whichDics) {
  if (!is.character(whichDics)) {
    stop("'whichDics' must be character!", call. = FALSE)
  }
  invisible(TRUE)
}

#' Backup current user dictionary
#'
#' @param ask ask to confirm backup
#' @export
backupUsrDic <- function(ask = TRUE) {
  response <- "Y"
  if (isTRUE(ask)) {
    response <- readline("Would you backup your current 'dic_user.txt' file to backup directory? (Y/n): ")
  }
  if (substr(response, 1L, 1L) == "Y") {
    source <- .hannlp_user_dic_path(create = TRUE)
    target <- .hannlp_backup_dic_path()
    if (!dir.exists(dirname(target))) {
      dir.create(dirname(target), recursive = TRUE, showWarnings = FALSE)
    }
    if (!file.copy(source, target, overwrite = TRUE)) {
      warning(sprintf("Could not copy %s", source), call. = FALSE)
      return(invisible(FALSE))
    }
  }
  invisible(TRUE)
}

#' Restore backed up user dictionary
#'
#' @param ask ask to confirm restore
#' @export
restoreUsrDic <- function(ask = TRUE) {
  backup <- .hannlp_backup_dic_path()
  if (!file.exists(backup)) {
    stop("There is no backuped dic_user.txt to restore!", call. = FALSE)
  }
  response <- "Y"
  if (isTRUE(ask)) {
    response <- readline("Would you restore your backuped 'dic_user.txt' file to current dictionary directory? (Y/n): ")
  }
  if (substr(response, 1L, 1L) == "Y") {
    target <- .hannlp_user_dic_path(create = TRUE)
    if (!file.copy(backup, target, overwrite = TRUE)) {
      warning(sprintf("Could not copy %s", backup), call. = FALSE)
      return(invisible(FALSE))
    }
  }
  reloadAllDic()
  invisible(TRUE)
}

#' Append or replace the HanNLP user dictionary
#'
#' @param newUserDic new user dictionary as data.frame
#' @param append append to existing dictionary or replace it
#' @param verbose retained for KoNLP API compatibility
#' @param ask ask to backup current dictionary
#' @export
mergeUserDic <- function(newUserDic, append = TRUE, verbose = FALSE, ask = FALSE) {
  if (isTRUE(ask)) {
    backupUsrDic(ask = FALSE)
  }
  new_dic <- .hannlp_normalize_user_dic(newUserDic)
  old_dic <- if (isTRUE(append) && file.exists(.hannlp_user_dic_path(create = TRUE))) {
    .hannlp_read_dic(.hannlp_user_dic_path(create = TRUE))
  } else {
    data.frame(term = character(), tag = character(), stringsAsFactors = FALSE)
  }
  out <- unique(rbind(old_dic, new_dic))
  .hannlp_write_user_dic(out)
  message(sprintf("%s words were added to dic_user.txt.", nrow(new_dic)))
  reloadAllDic()
  invisible(out)
}

#' Build HanNLP user dictionary
#'
#' @param ext_dic retained for KoNLP API compatibility; external dictionaries are not bundled yet
#' @param category_dic_nms retained for KoNLP API compatibility
#' @param user_dic user dictionary data.frame with term and KAIST tag columns
#' @param replace_usr_dic replace existing user dictionary instead of appending
#' @param verbose print detail progress
#' @export
buildDictionary <- function(ext_dic = "", category_dic_nms = "", user_dic = data.frame(), replace_usr_dic = FALSE, verbose = FALSE) {
  external_dic <- if (length(ext_dic) > 0L && any(nzchar(ext_dic))) {
    .hannlp_collect_external_dics(as.character(ext_dic), category_dic_nms)
  } else {
    data.frame(term = character(), tag = character(), stringsAsFactors = FALSE)
  }
  user_part <- if (is.data.frame(user_dic) && nrow(user_dic) > 0L) {
    .hannlp_normalize_user_dic(user_dic)
  } else {
    data.frame(term = character(), tag = character(), stringsAsFactors = FALSE)
  }
  incoming <- unique(rbind(external_dic, user_part))
  if (is.data.frame(user_dic) && nrow(user_dic) > 0L) {
    out <- mergeUserDic(incoming, append = !isTRUE(replace_usr_dic), verbose = verbose, ask = FALSE)
  } else if (nrow(external_dic) > 0L) {
    out <- mergeUserDic(external_dic, append = !isTRUE(replace_usr_dic), verbose = verbose, ask = FALSE)
  } else if (isTRUE(replace_usr_dic)) {
    out <- data.frame(term = character(), tag = character(), stringsAsFactors = FALSE)
    .hannlp_write_user_dic(out)
  } else {
    out <- .hannlp_read_dic(.hannlp_user_dic_path(create = TRUE))
  }
  message(sprintf("%s words dictionary was built.", nrow(out)))
  reloadAllDic()
  invisible(out)
}

#' Summary of dictionaries
#'
#' @param which one of `current` or `backup`
#' @param n number of rows for head and tail
#' @export
statDic <- function(which = "current", n = 6) {
  path <- switch(
    which,
    current = .hannlp_user_dic_path(create = TRUE),
    backup = .hannlp_backup_dic_path(),
    stop("No dictionary to summary!", call. = FALSE)
  )
  dic <- .hannlp_read_dic(path)
  dic$tag <- as.factor(dic$tag)
  list(summary = summary(dic), head = utils::head(dic, n = n), tail = utils::tail(dic, n = n))
}

#' Get Dictionary
#'
#' @param dic_name one of `user_dic`, `system_dic`, or `analyzed_dic`
#' @return data.frame with dictionary terms and tags
#' @export
get_dictionary <- function(dic_name) {
  if (!is.character(dic_name) || length(dic_name) != 1L) {
    stop("dic_name must be a character scalar", call. = FALSE)
  }
  path <- switch(
    dic_name,
    user_dic = .hannlp_user_dic_path(create = TRUE),
    system_dic = file.path(.hannlp_hannanum_data_dir(), "kE", "dic_system.txt"),
    analyzed_dic = file.path(.hannlp_hannanum_data_dir(), "kE", "dic_analyzed.txt"),
    sejong = return(.hannlp_read_sejong_dic()),
    insighter = return(.hannlp_read_niadic_table("insighter")),
    woorimalsam = return(.hannlp_read_niadic_table("woorimalsam", "all")),
    stop(sprintf("HanNLP does not contain '%s' dictionary!", dic_name), call. = FALSE)
  )
  .hannlp_read_dic(path)
}

#' Use bundled Sejong-style dictionary
#'
#' @param backup backup current dictionary before switching
#' @export
useSejongDic <- function(backup = TRUE) {
  if (isTRUE(backup)) backupUsrDic(ask = FALSE)
  buildDictionary(ext_dic = "sejong", replace_usr_dic = TRUE)
}

#' Use system default dictionary
#'
#' @param backup backup current dictionary before switching
#' @export
useSystemDic <- function(backup = TRUE) {
  if (isTRUE(backup)) backupUsrDic(ask = FALSE)
  bundled <- file.path(.hannlp_package_data_dir(), "kE", "dic_user.txt")
  .hannlp_write_user_dic(.hannlp_read_dic(bundled))
  reloadAllDic()
  invisible(TRUE)
}

#' Use NIA dictionary
#'
#' @param which_dic retained for KoNLP API compatibility
#' @param category_dic_nms retained for KoNLP API compatibility
#' @param backup backup current dictionary before switching
#' @export
useNIADic <- function(which_dic = c("woorimalsam", "insighter"), category_dic_nms = "all", backup = TRUE) {
  if (isTRUE(backup)) backupUsrDic(ask = FALSE)
  buildDictionary(ext_dic = which_dic, category_dic_nms = category_dic_nms, replace_usr_dic = TRUE)
}
