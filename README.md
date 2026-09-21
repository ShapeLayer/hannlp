# HanNLP

This package offers POS tagging and morphological analysis for Korean linguistics research. It comes packed with specialized utilities—including a keystroke converter, Hangul automata, and tools for calculating Mutual Information. You can also customize your workflow by selectively adding or editing entries in the morphological dictionary.

<br />

_derived from [KoNLP](https://github.com/haven-jeon/KoNLP)_

Since the Hannanum analyzer, which KoNLP depends on, is written in Java, KoNLP requires rJava and a Java environment to function. However, configuring the Java environment for rJava involves several manual steps, often making the installation process of KoNLP quite cumbersome.

HanNLP is a package designed to eliminate the dependency on rJava and the Java environment by porting the core implementations of KoNLP and the Hannanum analyzer to C.

## Installation

```R
install.packages('devtools')
devtools::install_github('ShapeLayer/HanNLP')
```

Alternatively, you can download this repository and install it by entering the following command in your terminal:

```sh
R CMD INSTALL path_of_HanNLP

# Example: (git required)
# git clone https://github.com/shapelayer/HanNLP.git
# R CMD INSTALL HanNLP
```

## User dictionary storage

Analysis and dictionary lookup use the bundled data without creating files. Before modifying or backing up a dictionary, explicitly set `HANNLP_USER_DATA_DIR` to a destination directory; no writable path is selected by default. Backups live in the `backup` subdirectory of that destination. For temporary use:

```r
local({
  old <- Sys.getenv("HANNLP_USER_DATA_DIR", unset = NA_character_)
  path <- tempfile("hannlp-dictionary-")
  on.exit({
    if (is.na(old)) Sys.unsetenv("HANNLP_USER_DATA_DIR") else
      Sys.setenv(HANNLP_USER_DATA_DIR = old)
    unlink(path, recursive = TRUE)
  })
  Sys.setenv(HANNLP_USER_DATA_DIR = path)
  mergeUserDic(data.frame(term = "HanNLP", tag = "ncn"), append = FALSE)
})
```
