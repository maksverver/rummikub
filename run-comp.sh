#!/bin/sh -e

BASEPATH=`readlink -f "$0"`
BASEDIR=`dirname "$BASEPATH"`
ARBITER="$BASEDIR"/arbiter/arbiter
GENTILES="$BASEDIR"/gen-tiles.py
GENHTML="$BASEDIR"/transcript-to-html.sh

DIR=$1

if [ -z "DIR" ]
then
	echo "Usage: $0 <dir>"
	exit 0
fi

if ! cd "$DIR"
then
	echo "Couldn't cd to $DIR"
	exit 1
fi

mkdir -p tiles results

for match in matches/*
do
	name=`basename "$match"`
	tiles=tiles/"$name"
	xml=results/"$name".xml
	html=results/"$name".html

	[ -f "$match" ] && (
		
		if ! flock -n 100
		then
			echo "Couldn't lock $match; skipping."
		elif [ ! -s "$xml" ]  # NB. must check it AFTER locking!
		then

			# Generate new tiles if necessary:
			if [ ! -s "$tiles" ]
			then
				echo "Generating $tiles..."
				$GENTILES > "$tiles"
			fi

			# Determine players in this match:
			P1=`cut -f1 "$match"`
			P2=`cut -f2 "$match"`
			P3=`cut -f3 "$match"`
			P4=`cut -f4 "$match"`
			if [ -z "$P1" -o -z "$P2" -o -z "$P3" -o -z "$P4" ]
			then
				echo "Invalid match: $match!"
			else
				echo "Running match $name with $P1, $P2, $P3 and $P4..."
				P1=players/"$P1"
				P2=players/"$P2"
				P3=players/"$P3"
				P4=players/"$P4"
				tmp=`tempfile -m 0644`
				if $ARBITER "$P1" "$P2" "$P3" "$P4" "$tiles" > "$tmp"
				then
					mv "$tmp" "$xml"
					echo "Generating HTML..."
					$GENHTML "$xml" > "$html"
				fi
			fi
		fi

	) 100>>"$match"
done
