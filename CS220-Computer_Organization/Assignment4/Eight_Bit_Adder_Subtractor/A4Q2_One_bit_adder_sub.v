module One_Bit_Adder_Sub(
    input a,
    inout b,
    input bcin,
    input opcode,
    output bcout,
    output ans
);

    assign ans = a ^ b ^ bcin ;
    assign bcout = opcode ?  (((~a) & b ) | (bcin & ((~a) | b))) :((a & b) | (bcin & (a | b)));
       
endmodule