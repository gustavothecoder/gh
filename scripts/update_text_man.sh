rm -f ./docs/text_man
touch ./docs/text_man
man ./docs/gh.1 | col -b > ./docs/text_man
