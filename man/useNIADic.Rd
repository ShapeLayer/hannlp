\name{useNIADic}
\alias{useNIADic}
\title{Use NIA dictionary}
\usage{useNIADic(which_dic = c("woorimalsam", "insighter"), category_dic_nms = "all", backup = TRUE)}
\arguments{
  \item{which_dic}{retained for KoNLP API compatibility}
  \item{category_dic_nms}{retained for KoNLP API compatibility}
  \item{backup}{backup current dictionary before switching}
}
\value{invisible data.frame of the resulting user dictionary}
\description{Replace the writable user dictionary with NIADic entries from an installed NIADic package, \code{HANNLP_NIADIC_RDATA}, or \code{HANNLP_NIADIC_DB}.}
