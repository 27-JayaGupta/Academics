module johnson(clk,reset,out);
input clk,reset;
output reg [7:0] out;
integer left = -1, right = -1;

always @ (posedge clk or posedge reset)  begin
        if(reset)
        begin
                left = -1;
                right = -1;
                out<=0;
        end
        else begin
        if(left<7) begin
            left = left + 1;
            out[left]<=1'b1;
        end else begin
            if(right==7) begin
                out <= 0;
                left=-1;
                right=-1;
            end else begin
                right = right + 1;
                out[right]<=1'b0;
            end
        end
  end
end
endmodule   //johnson