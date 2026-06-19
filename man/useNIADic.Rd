\name{useNIADic}
\alias{useNIADic}
\title{Use NIA dictionary}
\usage{
useNIADic(
  which_dic = c("woorimalsam", "insighter"),
  category_dic_nms = "all",
  backup = TRUE
)
}
\arguments{
  \item{which_dic}{NIADic dictionaries to load: \code{woorimalsam}, \code{insighter}, or both}
  \item{category_dic_nms}{retained for KoNLP API compatibility}
  \item{backup}{backup current dictionary before switching}
}
\value{invisible data.frame of the resulting user dictionary}
\description{Replace the writable user dictionary with NIADic entries. HanNLP does not bundle or distribute NIADic data because it is released under CC BY-SA, which is incompatible with the GPL. This function works only when the optional \code{NIADic} package is installed and its namespace is already loaded in the current R session (e.g. after \code{library(NIADic)}); otherwise it errors.}
