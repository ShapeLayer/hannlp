\name{useSystemDic}
\alias{useSystemDic}
\title{Use system default dictionary}
\usage{useSystemDic(backup = TRUE)}
\arguments{\item{backup}{backup current dictionary before switching}}
\value{invisible TRUE}
\description{Restore the writable user dictionary to HanNLP's bundled default user dictionary.}
\details{
Before writing dictionary files, explicitly set the environment variable
\code{HANNLP_USER_DATA_DIR} to the desired directory. There is no default
writable directory. Backups are stored in its \file{backup} subdirectory.
For temporary use, choose a directory under \code{tempdir()}.
}
