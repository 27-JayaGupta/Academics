.data
prompt: .asciiz "Enter the number of elements in the array: \n"
prompt2: .asciiz "Enter a number: \n"
prompt3: .asciiz "\nNext number: \n"
.globl main
.text
main:

li $t0, 1
li $s0, 500
li $t1, 1
loop:
li $v0, 4
la $a0, prompt3
syscall
li $v0, 1
move $a0, $t0
syscall

move $t3, $t0
add $t0, $t0, $t1
move $t1, $t3
bgt $t0, $s0, end
j loop
end:
li $v0, 10
syscall
