#!/bin/bash
echo "[+]Compiling the SatSolver.cpp..."
g++-11 -o sat_final.out SatSolver.cpp
cd testcases
echo "[+]Starting Test..."
for f in *
do
	echo "[+]Running over $f..."
	
	command ../sat_final.out  $(echo "$f")

	echo ""

done

