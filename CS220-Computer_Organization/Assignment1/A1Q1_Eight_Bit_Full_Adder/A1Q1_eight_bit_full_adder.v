`include "A1Q1_one_bit_adder.v"

module eight_bit_full_adder(
        output [7:0] ans,
        output cout,
        input [7:0] a,
        input [7:0] b,
        input cin
    );
    //input cin is 0 
    wire [7:0] carry_in; //internal wire of Eight_Bit_Adder
    assign carry_in[0]=cin;

    generate
        genvar i;
        for(i=0;i<7;i=i+1) begin : generate_block_identifier
            one_bit_adder o1(carry_in[i+1],ans[i],a[i],b[i],carry_in[i]);
        end

        one_bit_adder o2(cout,ans[7],a[7],b[7],carry_in[7]);

    endgenerate
   
endmodule