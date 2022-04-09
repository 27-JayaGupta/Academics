module GrayCodeCounter(input clk,
					output out);

reg out;
integer counter;
reg [2:0] state, nextstate;
// gray encoding of states
parameter [2:0] S0=3'b000,S1=3'b001,S2=3'b011,S3=3'b010,S4=3'b110,S5=3'b111,S6=3'b101,S7=3'b100;

initial begin
	counter = 0;
	state = S0;
	nextstate = S1;
end

always@ (posedge clk) begin
	$display("\t \t count=%d", counter);
end
always @(posedge clk) begin
	//$display("current state: %d", state);
	case(state)
		S0: begin
			out <= 0;
			counter <= counter + 1;
		end
		S1: begin
			out <= 0;
			counter <= counter + 1;
			end
		S2: begin 
			out <= 0;
			counter <= counter + 1;
		end
		S3: begin
			out <= 0;
			counter <= counter + 1;
		end

		S4: begin
			out <= 0;
			counter <= counter + 1;
		   end
		S5: begin
			out <= 0;
			counter <= counter + 1;
		end
		S6: begin
			out <= 0;
			counter <= counter + 1;
		end
		S7: begin
			counter <= 0;
			out <= 1;
		end

	endcase
end


always @(posedge clk) begin
	state <= nextstate;
end

always @(counter) begin
	case(state) 
	S0: begin
		nextstate = S1;
	end
	S1: begin
		nextstate = S2;
	end
	S2: begin
		nextstate = S3;
		
	end
	S3: begin
		nextstate = S4;
		
	end
	S4: begin
		nextstate = S5;
		
	end
	S5: begin
		nextstate = S6;
		
	end
	S6: begin
		nextstate = S7;
		
	end
	S7: begin
		nextstate = S0;
	end
	
	endcase
end
endmodule

