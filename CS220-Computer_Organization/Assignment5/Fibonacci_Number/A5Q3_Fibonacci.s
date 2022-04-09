.data
    output: .asciiz "Fibonacci numbers upto 500: :"
    newline: .asciiz "\n"
    space: .asciiz " "

.text
.globl main

    main: 
        li $v0,4
        la $a0,output
        syscall
        li $v0,4
        la $a0,space
        syscall
        li $v0,1
        li $a0,0
        li $t0,0
        syscall
        li $v0,4
        la $a0,space
        syscall
        li $v0,1
        li $a0,1
        li $t1,1
        syscall
        li $v0,4
        la $a0,space
        syscall
        add $a0,$t0,$t1
        j fibonacci

    fibonacci: 
        slti $t2,$a0,500
        beq $t2,$zero,exit
        li $v0,1
        syscall
        li $v0,4
        la $a0,space
        syscall
        add $a0,$t0,$t1
        move $t0,$t1
        move $t1,$a0
        add $a0,$t0,$t1
        j fibonacci

    exit:   
        jr $ra