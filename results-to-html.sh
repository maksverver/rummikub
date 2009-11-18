#!/bin/sh -e

XML2HTML=`dirname "$0"`/transcript-to-html.sh

DIR=$1

if [ ! -d "$DIR" ]
then
	echo "Not a directory: $DIR"
	exit 1
fi

for xml in "$DIR"/*.xml
do
	html="$DIR"/`basename "$xml" .xml`.html
	test -e "$html" || "$XML2HTML" "$xml" >"$html"
done

