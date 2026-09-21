library(HanNLP)

local({
  old <- Sys.getenv("HANNLP_USER_DATA_DIR", unset = NA_character_)
  user_data <- tempfile("hannlp-user-data-")
  second_data <- tempfile("hannlp-second-data-")
  tmp <- tempfile(fileext = ".txt")
  on.exit({
    if (is.na(old)) Sys.unsetenv("HANNLP_USER_DATA_DIR") else
      Sys.setenv(HANNLP_USER_DATA_DIR = old)
    unlink(c(user_data, second_data, tmp), recursive = TRUE)
  })
  Sys.unsetenv("HANNLP_USER_DATA_DIR")
  stopifnot(is.data.frame(get_dictionary("user_dic")))
  stopifnot(is.list(statDic()))
  entry <- data.frame(term = "HanNLP", tag = "ncn")
  for (action in list(
    function() mergeUserDic(entry, append = FALSE),
    function() buildDictionary(user_dic = entry),
    function() useSystemDic(backup = FALSE),
    function() backupUsrDic(ask = FALSE)
  )) {
    err <- tryCatch(action(), error = identity)
    stopifnot(inherits(err, "error"), grepl("HANNLP_USER_DATA_DIR", conditionMessage(err)))
  }
  Sys.setenv(HANNLP_USER_DATA_DIR = user_data)
  # Analysis before the first write must not prevent later initialization.

  stopifnot(identical(is.hangul("한글"), TRUE))
  stopifnot(identical(is.ascii("abc"), TRUE))
  stopifnot(identical(HangulAutomata("gksrmf", isKeystroke = TRUE), "한글"))

  nouns <- extractNoun("한글 형태소 분석을 테스트합니다")
  stopifnot(is.character(nouns))

  pos <- SimplePos09("한글 형태소 분석을 테스트합니다")
  stopifnot(is.list(pos), length(pos) > 0L)

  # Multiple eojeols exercise destruction of candidate-only morphology results.
  text <- intToUtf8(c(0xd55c, 0xae00, 0x20, 0xd615, 0xd0dc, 0xc18c))
  morph <- MorphAnalyzer(text)
  stopifnot(
    is.list(morph),
    identical(names(morph), strsplit(text, " ", fixed = TRUE)[[1L]]),
    all(lengths(morph) > 0L),
    all(vapply(morph, is.character, logical(1L))),
    all(grepl("/", unlist(morph), fixed = TRUE)),
    identical(MorphAnalyzer(text), morph)
  )

  writeLines("한글 형태소 분석", tmp, useBytes = TRUE)
  stopifnot(length(concordance_file(tmp, "형태소", encoding = "UTF-8")) == 1L)

  mi <- mutualinformation(c("한글 분석", "한글 테스트", "분석 테스트"))
  stopifnot(is.numeric(mi))

  stopifnot(!dir.exists(user_data))

  dic <- data.frame(term = "테스트", tag = "ncn", stringsAsFactors = FALSE)
  merged <- mergeUserDic(dic, append = FALSE)
  stopifnot(is.data.frame(merged), identical(merged$term, "테스트"))

  stopifnot(dir.exists(user_data))
  backupUsrDic(ask = FALSE)
  stopifnot(file.exists(file.path(user_data, "backup", "dic_user.txt")))
  mergeUserDic(entry, append = FALSE)
  restoreUsrDic(ask = FALSE)
  stopifnot(identical(get_dictionary("user_dic")$term, dic$term))
  Sys.setenv(HANNLP_USER_DATA_DIR = second_data)
  mergeUserDic(entry, append = FALSE)
  stopifnot(identical(get_dictionary("user_dic")$term, entry$term))
  Sys.setenv(HANNLP_USER_DATA_DIR = user_data)
  stopifnot(identical(get_dictionary("user_dic")$term, dic$term))
})
