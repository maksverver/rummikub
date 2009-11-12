#!/bin/sh

if [ ! -f "$1" ]
then
	echo "Not a file: $1"
	exit 1
fi

saxon9-xq transcript-to-html.xq file="$1" \
	'!indent=no' \
	'!doctype-public=-//W3C//DTD XHTML 1.0 Strict//EN' \
	'!doctype-system=http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd' \
	| sed 's,<html>,<html xmlns="http://www.w3.org/1999/xhtml">,'
