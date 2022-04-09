# test cases
# first: x1=2.3,x2=2.3,y1=3.5,y2=4.8  ans= inf
# second: x1=2.3,x2=3.323,y1=4,56,y2=9.2  ans= inf

.data
	array: .space 40
	space: .asciiz " "
	newline: .asciiz "\n"
	colon: .asciiz ":"
	Prompt1: .asciiz "Enter x1:"
	Prompt2: .asciiz "Enter y1:"
	Prompt3: .asciiz "Enter x2:"
	Prompt4: .asciiz "Enter y2:"	
	output: .asciiz "Slope: "

.text
.globl main
	
	#take input in appropriate registers
	main:
		li $v0,4
		la $a0,Prompt1
		syscall
		li $v0,6
		syscall
		mov.s $f1,$f0
		li $v0,4
		la $a0,Prompt2
		syscall
		li $v0,6
		syscall
		li $v0,4
		la $a0,Prompt3
		syscall
		mov.s $f2,$f0
		li $v0,6
		syscall
		mov.s $f3,$f0
		li $v0,4
		la $a0,Prompt4
		syscall
		li $v0,6
		syscall
		mov.s $f4,$f0     
		j slope

	#calculate slope
	slope:
		sub.s $f1,$f3,$f1
		sub.s $f2,$f4,$f2
		div.s $f12,$f2,$f1
		li $v0,4
		la $a0,output
		syscall
		li $v0,2
		syscall
		jr exit

	exit: jr $ra