\name{mergeUserDic}
\alias{mergeUserDic}
\title{Append or replace the HanNLP user dictionary}
\usage{mergeUserDic(newUserDic, append = TRUE, verbose = FALSE, ask = FALSE)}
\arguments{
  \item{newUserDic}{new user dictionary as data.frame}
  \item{append}{append to existing dictionary or replace it}
  \item{verbose}{retained for KoNLP API compatibility}
  \item{ask}{ask to backup current dictionary}
}
\value{invisible data.frame of the resulting user dictionary}
\description{Append to or replace the writable HanNLP user dictionary.}
\details{
Before writing dictionary files, explicitly set the environment variable
\code{HANNLP_USER_DATA_DIR} to the desired directory. There is no default
writable directory. Backups are stored in its \file{backup} subdirectory.
For temporary use, choose a directory under \code{tempdir()}.
}
