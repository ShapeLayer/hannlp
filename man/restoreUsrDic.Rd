\name{restoreUsrDic}
\alias{restoreUsrDic}
\title{Restore backed up user dictionary}
\usage{restoreUsrDic(ask = TRUE)}
\arguments{\item{ask}{ask to confirm restore}}
\value{invisible logical}
\description{Restore the writable HanNLP user dictionary from backup.}
\details{
Before writing dictionary files, explicitly set the environment variable
\code{HANNLP_USER_DATA_DIR} to the desired directory. There is no default
writable directory. Backups are stored in its \file{backup} subdirectory.
For temporary use, choose a directory under \code{tempdir()}.
}
