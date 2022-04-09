`include "A5Q1_GCD.v"

module Stimulus;
	reg clk;
	reg [7:0] u;
	reg [7:0] v;
	wire[7:0] gcd;
	GCD g(u, v, clk, gcd);
	initial begin
		u = 10;
		v = 5;
		clk = 0;

		#30
		u = 10;
		v = 20;

		#30
		u = 1;
		v = 20;

		#30
		u = 7;
		v = 12;

		#30
		u = 100;
		v = 2;

		#30
		u = 13;
		v = 91;

		#30
		u = 90;
		v = 110;

		#30
		u = 12;
		v = 43;

		#30
		u = 78;
		v = 3;

		#30
		u = 10;
		v = 25;

		#30
		u = 29;
		v = 20;

		#30
		u = 34;
		v = 5;

		#30
		u = 12;
		v = 6;

		#30
		u = 12;
		v = 16;

		#30
		u = 11;
		v = 22;

		#30 $finish;
	end
	
	always begin
		#1 clk = !clk;
	end

endmodule