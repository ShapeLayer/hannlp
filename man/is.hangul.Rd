\name{is.hangul}
\alias{is.hangul}
\title{Check if sentence is all Hangul}
\usage{is.hangul(sentence)}
\arguments{\item{sentence}{input characters}}
\value{logical vector}
\description{Check whether each input string contains only Hangul syllables or compatibility Jamo.}
\examples{
hangul <- intToUtf8(c(0xd55c, 0xae00))
is.hangul(c(hangul, "abc"))
}
