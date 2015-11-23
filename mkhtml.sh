#!/bin/sh
root -l <<EOF
gSystem->Load("libROBAST")
THtml html
html.SetInputDir(".:include")
html.SetHomepage("http://robast.github.io/")
html.SetProductName("ROBAST")
html.MakeAll()
EOF



