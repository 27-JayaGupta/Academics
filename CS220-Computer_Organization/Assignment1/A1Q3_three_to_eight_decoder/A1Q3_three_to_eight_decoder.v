`include "A1Q3_two_to_four_decoder.v"

module three_to_eight_decoder(
    input [2:0] a,
    output [7:0] i    
);
    //inputs a[0],a[1] and a[2]
    // not gate is used to invert signal a[2] (wire nc)
    //Model used is described in assumptions.md file
    // a[2] and not(a[2]) are used as enable signal for two_to_four_decoders

    wire nc;
    not n1(nc,a[2]);
    two_to_four_decoder d1(i[3:0],a[0],a[1],nc);
    two_to_four_decoder d2(i[7:4],a[0],a[1],a[2]);
    
endmodule