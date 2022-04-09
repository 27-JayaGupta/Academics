`include "A3Q1_Sequence_Detector.v"

module sequence_detector_testbench;

    reg clk,reset;
    reg [7:0] inp;
    wire [7:0] out;

    detector D(clk,inp,out);

    always #1 clk=~clk;
    initial
        begin
            $dumpfile("sequence_detector.vcd");
            $dumpvars(0,sequence_detector_testbench);
                clk <= 0;

            inp = 8'b10100000;
            #16 inp = 8'b00001010;
            #16 inp = 8'b01010100;
            #16 inp = 8'b01010100;
            #16 inp = 8'b11010100;
            #16 inp = 8'b01011000;
            #16 inp = 8'b01000100;
            #16 inp = 8'b00010100;
            #16 inp = 8'b00110100;
            #16 inp = 8'b11110100;
            #16 inp = 8'b10101010;
            #16 inp = 8'b01110101;
            #16 inp = 8'b01000000;
            #16 inp = 8'b00110100;
            #16 inp = 8'b01010101;
            #16 $finish;
        end

endmodule