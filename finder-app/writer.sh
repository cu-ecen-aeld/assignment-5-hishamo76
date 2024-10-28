#!/bin/bash

if [ $# -ne 2 ]
then
  echo "use: ./writer.sh writefile writestr"
  echo "writefile: the path including the filename"
  echo "writestr the string to be written"
  exit 1
else
  mypath=$1
  directory=$(dirname "$mypath")
  wilename=$(basename "$mypath")
  mkdir -m 777 -p $directory
  touch $mypath
  chmod 666 $mypath
  echo "$2" > "$mypath"
fi 

