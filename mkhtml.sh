#!/bin/sh
root -l <<EOF
gSystem->Load("libROBAST")
THtml html
html.SetHeader("misc/header.html")
html.SetFooter("misc/footer.html")
html.SetInputDir(".:src:include")
html.SetHomepage("http://robast.github.io/")
html.SetProductName("ROBAST")
html.MakeAll()
EOF



