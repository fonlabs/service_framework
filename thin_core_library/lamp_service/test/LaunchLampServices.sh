#!/bin/bash

#Copyright AllSeen Alliance. All rights reserved.

#Permission to use, copy, modify, and/or distribute this software for any   
#purpose with or without fee is hereby granted, provided that the above		
#copyright notice and this permission notice appear in all copies.			
#																
#THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES	
#WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF		
#MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
#ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
#WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
#ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
#OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

name="LampServiceDir"
specialChar="*"

rm -r -f $name$specialChar

echo "Please enter number of lamp services (Range: 1 to 100) to run:"
read numLampServices

# Initialize Pid List
pidLst=""

for (( i=1; i <= $numLampServices; i++ ))
do
   dname=$name$i
   mkdir "$dname"
   cd "$dname"
   ../lamp_service &
   pidLst="$pidLst $!"
   echo "Launched Lamp Service $i"
   cd ..
done

# trap ctrl-c and call ctrl_c()
trap ctrl_c INT
trap ctrl_z TSTP

function ctrl_c() {
  echo "Trapped CTRL-C"
  echo "Please press Enter to exit"
}

function ctrl_z() {
  echo "Trapped CTRL-Z"
  echo "Please press Enter to exit"
}

echo "Please press Enter to exit"
read exitKey

for pid in $pidLst
do
        ps | grep $pid
        kill -9 $pid
done

rm -r -f $name$specialChar



