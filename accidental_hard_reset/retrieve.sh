#!/bin/bash

dir=files
mkdir -p $dir
rm -rf $dir/*

# Get the sha of all dangling git blobs and commits"
blob_list=`git fsck --cache --no-reflogs --lost-found --dangling HEAD`

while IFS= read -r line; do
    blob_sha=`echo ${line##* }` 
    git cat-file blob $blob_sha > $dir/$blob_sha.txt
done <<< "$blob_list"
