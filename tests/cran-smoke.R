user_data <- file.path(tempdir(), "hannlp-user-data")
Sys.setenv(HANNLP_USER_DATA_DIR = user_data)
unlink(user_data, recursive = TRUE, force = TRUE)

library(HanNLP)

stopifnot(identical(is.hangul("한글"), TRUE))
stopifnot(identical(is.ascii("abc"), TRUE))
stopifnot(identical(HangulAutomata("gksrmf", isKeystroke = TRUE), "한글"))

nouns <- extractNoun("한글 형태소 분석을 테스트합니다")
stopifnot(is.character(nouns))

pos <- SimplePos09("한글 형태소 분석을 테스트합니다")
stopifnot(is.list(pos), length(pos) > 0L)

tmp <- tempfile(fileext = ".txt")
writeLines("한글 형태소 분석", tmp, useBytes = TRUE)
stopifnot(length(concordance_file(tmp, "형태소", encoding = "UTF-8")) == 1L)

mi <- mutualinformation(c("한글 분석", "한글 테스트", "분석 테스트"))
stopifnot(is.numeric(mi))

dic <- data.frame(term = "테스트", tag = "ncn", stringsAsFactors = FALSE)
merged <- mergeUserDic(dic, append = FALSE)
stopifnot(is.data.frame(merged), identical(merged$term, "테스트"))

unlink(user_data, recursive = TRUE, force = TRUE)
