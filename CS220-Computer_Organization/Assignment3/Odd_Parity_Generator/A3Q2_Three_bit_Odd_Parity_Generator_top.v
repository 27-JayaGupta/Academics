`include "A3Q2_Three_bit_Odd_Parity_Generator.v"
module stimulus;

    reg clk;
    reg [2:0] a;
    wire out;

    initial begin
        clk = 1'b0;
        a = 3'b000;
        #6 a = 3'b001;
        #6 a = 3'b010;
        #6 a = 3'b100;
        #6 a = 3'b011;
        #6 a = 3'b110;
        #6 a = 3'b101;
        #6 a = 3'b111;
      
        #6 $display("                     Parity_Bit : %b",out);
        #6 $finish;
    end

    Three_Bit_Odd_Parity_Generator P(a,clk,out);

    always begin
        #1 clk = ~clk;
    end

    initial begin
        $monitor($time," A:%b ",a);
        $dumpfile("Three_Bit_Odd_Parity_Generator.vcd");
        $dumpvars(0,stimulus);
    end

endmodule