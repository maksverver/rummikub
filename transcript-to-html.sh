#!/bin/sh

DIR=`dirname "$0"`
FILE=`readlink -f "$1"`

if [ ! -f "$FILE" ]
then
	echo "Not a file: $FILE"
	exit 1
fi

saxon9-xq "$DIR"/transcript-to-html.xq file="$FILE" \
	'!indent=no' \
	'!doctype-public=-//W3C//DTD XHTML 1.0 Strict//EN' \
	'!doctype-system=http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd' \
	| sed 's,<html>,<html xmlns="http://www.w3.org/1999/xhtml">,'
