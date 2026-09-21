\name{backupUsrDic}
\alias{backupUsrDic}
\title{Backup current user dictionary}
\usage{backupUsrDic(ask = TRUE)}
\arguments{\item{ask}{ask to confirm backup}}
\value{invisible logical}
\description{Backup the writable HanNLP user dictionary.}
\details{
Before writing dictionary files, explicitly set the environment variable
\code{HANNLP_USER_DATA_DIR} to the desired directory. There is no default
writable directory. Backups are stored in its \file{backup} subdirectory.
For temporary use, choose a directory under \code{tempdir()}.
}
