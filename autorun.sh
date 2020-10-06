#/bin/sh
echo $@
make && cooja -nogui=$@
