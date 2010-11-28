#!/bin/sh
root -l <<EOF
gSystem->Load("libROBAST")
THtml html
html.SetProductName("ROBAST")
html.MakeAll()
EOF