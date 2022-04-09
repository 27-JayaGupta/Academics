`include "A1Q4_priority_encoder.v"
module tester;

    reg [7:0] in;
    wire [2:0] out;
    priority_encoder E(in, out);
    initial begin
        $monitor($time," in=%b out=%b", in,out);
    end
    initial begin
        $dumpfile("test.vcd");
        $dumpvars(0,tester);
        in = 8'b01010101;
        #5
        $display("\n");
        in = 8'b01010100;
        #5
        $display("\n");
        in = 8'b01110000;
        #5
        $display("\n");
        in = 8'b11000000;
        #5
        $display("\n");
        in = 8'b11010000;
        #5
        $display("\n");
        in = 8'b00000000;
        #5
        $display("\n");
        in = 8'b01011100;
        #5
        $display("\n");
        in = 8'b01010110;
        #5
        $display("\n");
        in = 8'b01111000;
        #5
        $display("\n");
        in = 8'b01000000;
    end
endmodule