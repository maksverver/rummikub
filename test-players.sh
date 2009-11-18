#!/bin/sh

QUERY='yourTiles=R13&table=R9.R10.R11.R12&opponentsTiles=14.14.14&poolTiles=57'
RESPONSE='R9.R10.R11.R12.R13'

FAILED=0

for FILE in $@
do
	if [ ! -f "$FILE" ]
	then
		echo "$FILE is not a file!"
	else
		NAME=`head -1 "$FILE" | tail -1`
		URL=`head -2 "$FILE" | tail -1`
		METHOD=`head -3 "$FILE" | tail -1`

		if [ "$NAME" != "`basename "$FILE"`" ]
		then
			echo "WARNING: player name $NAME does not match filename $FILE!"
		fi

		if [ "$METHOD" = POST ]
		then
			res=`curl -s --data "$QUERY" "$URL"`
		elif [ "$METHOD" = GET ]
		then
			res=`curl -s "$URL"'?'"$QUERY"`
		else
			echo "Invalid request method in $FILE!"
			res=-
		fi

		# Strip trailing whitespace:
		res=`echo "$res" | sed -r 's/\s+$//'`
		echo -n "$FILE ($NAME) "
		if [ "$res" = "$RESPONSE" ]
		then
			echo "Ok."
		else
			echo "Failed!"
			echo "Expected: '$RESPONSE'"
			echo "Received: '$res'"
			FAILED=`expr $FAILED + 1`
		fi
	fi
done

exit $FAILED

exit $EXIT
