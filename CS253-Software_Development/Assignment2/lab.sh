#!/bin/bash

#usage instructions for the script
usage(){
	echo -e "Instructions to run the program "
	echo -e "./lab.sh <file1> <file2>"
}


# check if there are 2 file name arguments or not
if [ $# -lt 2 ]; then
	usage
	exit -1
fi


#check if input file exists or not
if [ ! -f "$1" ]; then
	echo -e "Input file does not exist"
	exit -1
fi


#storing the file name in variables
INPUT_FILE=$1
OUTPUT_FILE=$2


#print the desired columns to the output file separated by space
awk -F',' '{print $1" "$2" "$3" "$5" "$6" "$7" "$10" "$11}' $INPUT_FILE > $OUTPUT_FILE


#append the name of colleges whose HIGHEST DEGREE is Bachelor's to the output file
awk -F',' '{
				if($3="Bachelor" && $1!="Name")
					print $1
			}' $INPUT_FILE >> $OUTPUT_FILE


#append average admission rate grouped by geography to the output file
echo -e "Geography: Average Admission Rate" >> $OUTPUT_FILE

awk -F',' '{
	a[$6]++;
	b[$6]+=$7
}
END{
	for (i in a) {
		if(i!="Geography"){
			printf "%s: %f \n",i,b[i]/a[i];			
		}
	}
}' $INPUT_FILE >> $OUTPUT_FILE

tail -n +2 $INPUT_FILE
#Sort the entries of the input file by meadian earnings in descending order and take the college name of the first five
{ head -n 1 $INPUT_FILE && tail -n +2 $INPUT_FILE | sort -t"," -r -k16 | head -n 5;} > max.txt

awk -F',' '{
		if($1!="Name"){
			print $1
		}
	}' max.txt >> $OUTPUT_FILE

rm max.txt