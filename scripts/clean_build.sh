#!/bin/bash


FILES=$(ls -a ./build)

# delete all files in ./build except for the .gitignore file
for file in $FILES
do
    if [ $file != ".gitignore" ]
    then
        rm -rf ./build/$file
    fi
done
