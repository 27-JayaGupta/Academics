.data
prompt: .asciiz "Enter the number of elements in the array: \n"
prompt2: .asciiz "Enter a number: \n"
prompt3: .asciiz "Answer: \n"
.globl main
.text
main:

li $v0, 4
la $a0, prompt
syscall
li $v0, 5             
syscall               
move $s0, $v0           

li.s $f6, 0.0
li.s $f2, 1.0
li.s $f4, -1.0
li $t0, 0

loop:
li $v0, 4
la $a0, prompt2
syscall
li $v0, 6              
syscall  
addi $t0, $t0, 1      
mul.s $f0, $f0, $f2
add.s $f6, $f0, $f6
mul.s $f2, $f2, $f4
beq $t0, $s0, end
j loop
end: 
li $v0, 4
la $a0, prompt3
syscall 
li $v0, 2
mov.s $f12, $f6  
syscall
li $v0, 10
syscall