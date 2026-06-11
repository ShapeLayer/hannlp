\name{HangulAutomata}
\alias{HangulAutomata}
\title{Compose Hangul syllables from Jamo or keystrokes}
\usage{HangulAutomata(input, isKeystroke = FALSE, isForceConv = FALSE)}
\arguments{
  \item{input}{input Jamo sequences or two-beolsik keystrokes}
  \item{isKeystroke}{whether input is two-beolsik keystrokes instead of Jamo}
  \item{isForceConv}{force conversion for incomplete sequences}
}
\value{complete Hangul syllable string}
\description{Compose complete Hangul syllables from compatibility Jamo or two-beolsik keystroke sequences.}
