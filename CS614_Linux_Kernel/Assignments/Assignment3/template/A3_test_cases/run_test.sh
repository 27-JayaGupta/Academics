#!/bin/bash
export LD_LIBRARY_PATH=/home/rohit/cs614a3/all_test_cases_template/lib

#Assuming you are running this script as root user
insmod all_test_cases_template/drivers/*.ko
echo "Driver inserted"

./all_test_cases_template/create-dev



if [ ! -d $1 ];
then
	echo "Provide a valid bin path"
	exit -1
fi

find $1/* | sort --version-sort | while read line;
do

   echo "******* Testing test: $line *******"
   timeout 3m ./$line 	
   if [ $? -ne 0 ]
   then
	   echo "Test failed."
   fi
done
