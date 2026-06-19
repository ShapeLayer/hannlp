\name{get_dictionary}
\alias{get_dictionary}
\title{Get Dictionary}
\usage{get_dictionary(dic_name)}
\arguments{\item{dic_name}{one of \code{user_dic}, \code{system_dic}, \code{analyzed_dic}, \code{sejong}, \code{woorimalsam}, or \code{insighter}. The \code{woorimalsam} and \code{insighter} dictionaries require the optional \code{NIADic} package to be installed and loaded (\code{library(NIADic)}) in the current session; HanNLP does not bundle NIADic data.}}
\value{data.frame with dictionary terms and tags}
\description{Return a HanNLP dictionary as a data.frame. The external Sejong dictionary requires an installed source package or a configured file path. The NIADic-based dictionaries require the loaded optional NIADic package.}
