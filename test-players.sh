#!/bin/sh

QUERY='yourTiles=R13&table=R3.R4.R5.R6.R7.R8.R9.R10.R11.R12&opponentsTiles=14.14.14&poolTiles=51'
RESPONSE='R3.R4.R5.R6.R7.R8.R9.R10.R11.R12.R13'

for FILE in $@
do
	if [ ! -f "$FILE" ]
	then
		echo "$FILE is not a file!"
	else
		NAME=`head -1 "$FILE" | tail -1`
		URL=`head -2 "$FILE" | tail -1`
		METHOD=`head -3 "$FILE" | tail -1`

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
			echo "[$RESPONSE]"
			echo "[$res]"
		fi
	fi
done
