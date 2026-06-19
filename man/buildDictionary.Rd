\name{buildDictionary}
\alias{buildDictionary}
\title{Build HanNLP user dictionary}
\usage{
buildDictionary(
  ext_dic = "",
  category_dic_nms = "",
  user_dic = data.frame(),
  replace_usr_dic = FALSE,
  verbose = FALSE
)
}
\arguments{
  \item{ext_dic}{optional external dictionary names: \code{sejong}, \code{woorimalsam}, or \code{insighter}}
  \item{category_dic_nms}{retained for KoNLP API compatibility}
  \item{user_dic}{user dictionary data.frame with term and KAIST tag columns}
  \item{replace_usr_dic}{replace existing user dictionary instead of appending}
  \item{verbose}{print detail progress}
}
\value{invisible data.frame of the resulting user dictionary}
\description{Build the writable HanNLP user dictionary from user-supplied entries and optional external sources. The Sejong dictionary is read from the installed Sejong package or the path specified by \code{HANNLP_SEJONG_HANDIC}. The \code{woorimalsam} and \code{insighter} dictionaries require the optional \code{NIADic} package to be installed and loaded (\code{library(NIADic)}) in the current session; HanNLP does not bundle NIADic data.}
