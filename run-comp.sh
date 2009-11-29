#!/bin/sh -e

BASEPATH=`readlink -f "$0"`
BASEDIR=`dirname "$BASEPATH"`
ARBITER="$BASEDIR"/arbiter/arbiter
GENTILES="$BASEDIR"/gen-tiles.py
TESTPLAYERS="$BASEDIR"/test-players.sh

DIR=$1

if [ -z "$DIR" ]
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

#if ! "$TESTPLAYERS" players/*
#then
#	echo "$? players failed!"
#	exit 1
#fi

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
				"$GENTILES" > "$tiles"
			fi

			# Determine players in this match:
			P1=players/`cut -f1 "$match"`
			P2=players/`cut -f2 "$match"`
			P3=players/`cut -f3 "$match"`
			P4=players/`cut -f4 "$match"`
			echo "Running match $name with $P1, $P2, $P3 and $P4..."
			tmp=`mktemp`
			chmod 0644 "$tmp"
			if ! "$ARBITER" "$P1" "$P2" "$P3" "$P4" "$tiles" > "$tmp"
			then
				cat "$tmp"    # show arbiter error message
				rm -f "$tmp"
			else
				mv "$tmp" "$xml"
			fi
		fi

	) 100>>"$match"
done
