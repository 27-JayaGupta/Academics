#test
#first: 1,2,3,4,5,6,7,8,9  k = 10 ans = not found
#second : 12,23,24,12,67,32,13 k=12   ans = 3 (finds one index where the element is present out of many)
.data
        array: .space 40
        space: .asciiz " "
        Prompt1: .asciiz "Enter n: "
        Prompt2: .asciiz "Enter array elements line by line: "
        Prompt3: .asciiz "Enter integer to search for: "
        Prompt4: .asciiz "Found element at index: "
        Prompt5: .asciiz "Element was not found"
        Prompt6: .asciiz "No array to search in"
        

.text
.globl main
        main:
                li $v0,4
                la $a0,Prompt1
                syscall
                li $v0,5
                syscall
                move $s0,$v0
                move $s1,$s0
                li $t0,0
                beq $s0,$zero,empty
                j inputArr_label

        inputArr_label:
                li $v0,4
                la $a0,Prompt2
                syscall
                j inputArr

        #input array in array variable
        inputArr:
                beq $s1,$zero,input_k
                addi $s1,$s1,-1
                li $v0,5
                syscall
                sw $v0,array($t0)
                addi $t0,$t0,4
                j inputArr
        
        #input k 
        input_k:
                li $v0,4
                la $a0,Prompt3
                syscall
                li $v0,5
                syscall
                move $s2,$v0
                li $t4,2
                li $s4,0
                move $s5,$s0
                j cal_index

        # calculate index for binary search(i.e middle of first and last index)
        cal_index:
                add $t1,$s4,$s5
                div $t1,$t4
                mflo $s7
                move $s1,$s7
                li $t7,4
                mul $s1,$s1,$t7
                j binary_search

        # do a recursive binary search until you find a element
        binary_search:
                slt $t6,$s5,$s4
                bne $t6,$zero,not_found
                lw $t2,array($s1)
                beq $t2,$s2,output
                slt $t0,$t2,$s2
                beq $t0,$zero,less
                j more

        # index changes when the middle value is less than the value of k
        less:
                addi $s7,$s7,-1
                move $s5,$s7
                j cal_index

        # index changes when the middle value is more than the value of k
        more:
                addi $s7,$s7,1
                move $s4,$s7
                j cal_index

        #give final output i.e the index of the position where the element is found
        output:
                li $v0,4
                la $a0,Prompt4
                syscall
                li $v0,1
                move $a0,$s7
                syscall
                j exit

        # output when element is not found
        not_found:
                li $v0,4
                la $a0,Prompt5
                syscall
                j exit

        # output when array is empty
        empty:
                li $v0,4
                la $a0,Prompt6
                syscall
                j exit

        #exit the program
        exit:
                jr $ra