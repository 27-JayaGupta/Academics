module D_FF(
    input clk,
    input reset,
    input d,
    output q
);
    reg q;
    always @(posedge clk or posedge reset) begin
        if(reset) 
            q <= 1'b0;
        else 
            q <= d;
    end
endmodule

module eight_bit_johnson_counter(
    input clk,
    input reset,
    output [7:0] q
);

    generate
        genvar i;
        for(i=0;i<8;i=i+1) begin
            if(i==0) begin
                D_FF dff1(clk,reset,~(q[7]),q[0]);
            end
            else begin
                D_FF dff2(clk,reset,q[i-1],q[i]);
            end
        end
    endgenerate

endmodule