`include "A4Q2_One_bit_adder_sub.v"

module Eight_Bit_Adder_Sub(
    input [7:0] a,
    input [7:0] b,
    input opcode,
    output [7:0] ans,
    output overflow
);

    wire [7:0] carry;
    wire cin = 0;
    
    One_Bit_Adder_Sub a1(a[0],b[0],cin,opcode,carry[0],ans[0]);
    One_Bit_Adder_Sub a2(a[1],b[1],carry[0],opcode,carry[1],ans[1]);
    One_Bit_Adder_Sub a3(a[2],b[2],carry[1],opcode,carry[2],ans[2]);
    One_Bit_Adder_Sub a4(a[3],b[3],carry[2],opcode,carry[3],ans[3]);
    One_Bit_Adder_Sub a5(a[4],b[4],carry[3],opcode,carry[4],ans[4]);
    One_Bit_Adder_Sub a6(a[5],b[5],carry[4],opcode,carry[5],ans[5]);
    One_Bit_Adder_Sub a7(a[6],b[6],carry[5],opcode,carry[6],ans[6]);
    One_Bit_Adder_Sub a8(a[7],b[7],carry[6],opcode,carry[7],ans[7]);

    assign overflow = carry[6] ^ carry[7];

endmodule