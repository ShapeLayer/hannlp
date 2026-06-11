\name{convertTag}
\alias{convertTag}
\title{Tag name converter}
\usage{convertTag(fromTag, toTag, tag)}
\arguments{
  \item{fromTag}{tag set name to convert from, either \code{K} or \code{S}}
  \item{toTag}{desired tag set name, either \code{K} or \code{S}}
  \item{tag}{tag name to search}
}
\value{converted tag vector}
\description{Convert tags between KAIST and Sejong tag sets.}
\examples{
convertTag("K", "S", "ncn")
}
