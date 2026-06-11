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
  \item{ext_dic}{external dictionary names: \code{sejong}, \code{woorimalsam}, or \code{insighter}}
  \item{category_dic_nms}{category names for \code{woorimalsam}; English aliases and Korean names are accepted}
  \item{user_dic}{user dictionary data.frame with term and KAIST tag columns}
  \item{replace_usr_dic}{replace existing user dictionary instead of appending}
  \item{verbose}{print detail progress}
}
\value{invisible data.frame of the resulting user dictionary}
\description{Build the writable HanNLP user dictionary from user-supplied entries and optional external Sejong/NIADic sources. External data is read from installed packages or paths specified by \code{HANNLP_SEJONG_HANDIC}, \code{HANNLP_NIADIC_RDATA}, or \code{HANNLP_NIADIC_DB}.}
