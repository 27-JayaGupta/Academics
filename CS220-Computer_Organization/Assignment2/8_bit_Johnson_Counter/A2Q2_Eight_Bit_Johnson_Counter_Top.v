`include "A2Q2_Eight_Bit_Johnson_Counter.v"

module stimulus;

    reg clk;
    reg reset;

    wire [7:0] q;

    eight_bit_johnson_counter counter1(clk,reset,q);

    initial begin
        clk=1'b0;
    end

    always  begin
        #5 clk=~clk;
    end

    initial begin
        
        reset=1'b1;
        #15 reset=1'b0;
        #180 reset=1'b1;
        #10 reset=1'b0;
        #10 $finish;
    end


    initial begin
        $dumpfile("8_bit_johnson_counter.vcd");
        $dumpvars(0,stimulus);
    end

    initial begin
        $display("\t \t \t \t \t q7 q6 q5 q4 q3 q2 q1 q0  ");
        $monitor($time," reset = %b,Output q = %b  %b  %b  %b  %b  %b  %b  %b",reset,q[7],q[6],q[5],q[4],q[3],q[2],q[1],q[0]); 
    end

endmodule