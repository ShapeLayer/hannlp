\name{get_dictionary}
\alias{get_dictionary}
\title{Get Dictionary}
\usage{get_dictionary(dic_name)}
\arguments{\item{dic_name}{one of \code{user_dic}, \code{system_dic}, \code{analyzed_dic}, \code{sejong}, \code{woorimalsam}, or \code{insighter}}}
\value{data.frame with dictionary terms and tags}
\description{Return a HanNLP dictionary as a data.frame. External Sejong/NIADic dictionaries require installed source packages or configured file paths.}
