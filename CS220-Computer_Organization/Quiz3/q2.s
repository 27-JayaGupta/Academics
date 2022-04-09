#first: 56 (8 ,0)
#second : 37 (5,2)
#
.data
	array: .space 40
	space: .asciiz " "
	newline: .asciiz "\n"
	colon: .asciiz ":"
	Prompt: .asciiz "Enter N:"
	output: .asciiz "Ans(x and y): "

.text
.globl main
	
	main:
		li $v0,4
		la $a0,Prompt
		syscall
		li $v0,5
		syscall
		move $s0,$v0
		li $s1,0
		li $s2,0
		j loop

	loop:
		slti $t0,$s0,7
		bne $t0,$zero,exit
		addi $s1,$s1,1
		addi $s0,$s0,-7
		j loop

	exit:
		add $s2,$s2,$s0
		li $v0,4
		la $a0,output
		syscall
		li $v0,1
		move $a0,$s1
		syscall
		li $v0,4
		la $a0,space
		syscall
		li $v0,1
		move $a0,$s2
		syscall
		jr $ra
