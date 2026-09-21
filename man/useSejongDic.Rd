\name{useSejongDic}
\alias{useSejongDic}
\title{Use bundled Sejong-style dictionary}
\usage{useSejongDic(backup = TRUE)}
\arguments{\item{backup}{backup current dictionary before switching}}
\value{invisible result}
\description{Replace the writable user dictionary with Sejong dictionary entries from an installed Sejong package or \code{HANNLP_SEJONG_HANDIC}.}
\details{
Before writing dictionary files, explicitly set the environment variable
\code{HANNLP_USER_DATA_DIR} to the desired directory. There is no default
writable directory. Backups are stored in its \file{backup} subdirectory.
For temporary use, choose a directory under \code{tempdir()}.
}
