.data
	array: .space 40
	space: .asciiz " "
	newline: .asciiz "\n"
	colon: .asciiz ":"
	Prompt: .asciiz "Enter values (10):"
	output: .asciiz "Sorted Array: "

.text
.globl main
	main:
		li $v0,4
		la $a0,Prompt
		syscall
		li $t0,0
		j input

	input:
		beq $t0,40,loop_ini
		li $v0,5
		syscall
		sw $v0,array($t0)
		addi $t0,$t0,4
		j input


	loop_ini:
		li $s0,4
		j sort

	sort:
		beq $s0,40,zero_ini
		lw $s1,array($s0)
		addi $s2,$s0,-4
		j while

	while:
		slt $t0,$s2,$zero
		bne $t0,$zero,restore
		lw $s3,array($s2)
		slt $t0,$s1,$s3
		beq $t0,$zero,restore
		addi $t0,$s2,4
		sw $s3,array($t0)
		addi $s2,$s2,-4
		j while

	restore:
		addi $s2,$s2,4
		sw $s1,array($s2)
		addi $s0,$s0,4
		j sort

	zero_ini:
		li $t0,0
		j label_print

	label_print:
		li $v0,4
		la $a0,output
		syscall
		li $v0,4
		la $a0,space
		syscall
		j print

	print:
		beq $t0,40,exit
		li $v0,1
		lw $a0,array($t0)
		syscall
		li $v0,4
		la $a0,space
		syscall
		addi $t0,$t0,4
		j print

	exit:
		j $ra





