`include "A4Q1_Gray_counter.v"
module Top;
// add the testbench
	wire out;
	reg clk;

	GrayCodeCounter g(clk, out);

	initial begin
		clk = 0;
	end
	always begin
		#2 clk = ~clk;
	end

	initial begin
		#100 $finish;
	end

	initial begin
		$monitor($time, " input= %b, out = %b \n", clk, out);
		$dumpvars(0,Top);
		$dumpfile("gray_counter.vcd");
	end
endmodule