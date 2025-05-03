#!/bin/sh
cat $1.log | grep -oE 'implicit declaration of function ‘[^’]+’' >/dev/null 2>&1
if [ "$?" = "0" ]; then
	F=`cat $1.log | grep -oE 'implicit declaration of function ‘[^’]+’' | sed -E 's/implicit declaration of function .(.+).$/\1/g'`
	for i in $F; do
		echo "Finding $i"
		new=true
		for sym in `cat implicit_list`; do
			if [ "$sym" = "$i" ]; then
				new=false
				break
			fi
		done
		if $new; then
			echo "$i" >> implicit_list
			SYM="`grep -oE "^[ \\t]*(struct )?[^ ]+ $i.+[ \\t]*\$" *.c | cut -d":" -f2`"
			if [ ! "x$SYM" = "x" ]; then
				echo "$SYM;" >> include/glquake_implicit.h
			fi
		fi
	done
fi
