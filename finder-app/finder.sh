#!/bin/bash

if [ $# -ne 2 ]
then
  echo "use: ./finder.sh filesdir searchstr"
  echo "filesdir: the directoy where the search to be preformed"
  echo "search: the search pattern to be found"
  exit 1
elif [[ -d $1  ]]; then
  lines=$(ls -lR $1 2>1 /dev/null | grep "^-" | wc -l)
  result=$(grep -r -i $2 $1 2>1 /dev/null | wc -l)
  echo "The number of files are $lines and the number of matching lines are $result"
else
  echo "directory $1 doesn't exist"
  exit 1
fi 

  
