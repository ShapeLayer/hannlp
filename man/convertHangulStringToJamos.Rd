\name{convertHangulStringToJamos}
\alias{convertHangulStringToJamos}
\title{Convert Hangul string to Jamos}
\usage{convertHangulStringToJamos(hangul)}
\arguments{\item{hangul}{Hangul string}}
\value{Jamo sequences}
\description{Convert Hangul syllables to compatibility Jamo sequences.}
\examples{
hangul <- intToUtf8(c(0xd55c, 0xae00))
convertHangulStringToJamos(hangul)
}
