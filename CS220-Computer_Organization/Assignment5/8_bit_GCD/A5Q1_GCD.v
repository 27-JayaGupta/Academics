module GCD(input[7:0] u,
		   input[7:0] v,
		   input clk,
		   output[7:0] gcd);
reg[7:0] gcd;
integer count;
integer flag;
reg [7:0] x, y;
reg[2:0] state, next_state;
initial begin
	state = 0;
	next_state = 0;
	count = 0;
	flag =0;
end
always @(u or v) begin
	state = 0;
	next_state = 0;
	count = 0;
	flag = 0;
end
always @(posedge clk)
begin

	state = next_state;
	case(state)
	0: begin
		x = u;
		y = v;
	end
	1: begin
		count = count+1;
		x = x >> 1;
		y = y >> 1;
	end
	2: begin
		x = x >> 1;
	end
	3: begin
		y = y >> 1;
	end
	4: begin
		if (x < y) begin
			y = (y-x) >> 1;
		end
		else begin
			x = (x-y) >> 1;
		end
	end
	5: begin
		if(count == 0)begin

			gcd =x;

			if(flag ==0 )  begin
				$display($time," input: u: %d , v: %d, gcd: %d",u,v,gcd);
				flag = 1;
			end
		end
		else if(count > 0) begin
			count  = count - 1;
			x = x << 1;
			y = y << 1;
		end
	end	
	endcase
end


always @(x, y) begin
	if(x == y) begin
		next_state = 5;
	end 
	else if(x[0] == 0 && y[0] == 0) begin
		next_state = 1;
	end
	else if(x[0] == 0 && y[0] == 1) begin
		next_state = 2;
	end
	else if(x[0] == 1 && y[0] == 0) begin
		next_state = 3;
	end
	else if(x[0] == 1 && y[0] == 1) begin
		next_state = 4;
	end
	
end
endmodule