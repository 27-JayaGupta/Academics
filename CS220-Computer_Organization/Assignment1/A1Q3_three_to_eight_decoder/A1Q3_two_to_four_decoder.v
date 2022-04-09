module two_to_four_decoder(
    output [3:0] i,
    input a,
    input b,
    input e
);

    assign i[0] = e & (~a) & (~b);
    assign i[1] = e & a & (~b);
    assign i[2] = e & (~a) & b;
    assign i[3] = e & a & b ;
    
endmodule