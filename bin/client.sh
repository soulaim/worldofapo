#!/bin/sh

# change to the directory where the script is located
dir="$0"
name=`basename "$dir"`
if [ "$name" = "$dir" ]; then
	OIFS=$IFS
	IFS=:
	for path in $PATH; do
		if [ -x "$path/$name" ]; then
			dir="$path/$name"
			break;
		fi
	done
	IFS=$OIFS
fi

while [ -L "$dir" ]; do
	dir=`readlink "$dir"`
done
cd "`dirname "$dir"`"

# launch the binary
./client "$*"

