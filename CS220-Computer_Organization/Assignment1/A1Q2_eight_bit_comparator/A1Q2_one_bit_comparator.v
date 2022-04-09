module one_bit_comparator (
    input a, 
    input b, 
    input l_prev, 
    input e_prev, 
    input g_prev, 
    output less, 
    output equal, 
    output greater);

   // Combinational logic
   assign less = l_prev|e_prev&~a&b;
   assign equal = e_prev&~(a^b);
   assign greater = g_prev|e_prev&a&~b; 

endmodule