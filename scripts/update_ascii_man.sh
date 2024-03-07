rm -f ./docs/ascii_man
touch ./docs/ascii_man
cat ./docs/gh.1 | groff -mandoc -T ascii > ./docs/ascii_man

