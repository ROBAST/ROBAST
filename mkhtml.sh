#!/bin/sh
root -l <<EOF
gSystem->Load("libROBAST")
THtml html
html.SetInputDir(".:src:include")
html.SetProductName("ROBAST")
html.MakeAll()
EOF