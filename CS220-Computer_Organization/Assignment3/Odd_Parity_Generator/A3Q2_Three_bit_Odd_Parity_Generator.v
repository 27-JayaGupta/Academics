module Three_Bit_Odd_Parity_Generator(
    input [2:0] a,
    input clk,
    output out 
);

    parameter [2:0] ODD = 3'b001 , EVEN =3'b010;
    integer count;
    reg [2:0] inp;
    reg out;
    reg [2:0] state;
    reg [2:0] next_state;


    always@(a) begin
        if(out==0 || out==1) $display("                     Parity_Bit: %b  \n " , out);
        state = EVEN;
        next_state = EVEN;
        inp = a;
        count = 0;
    end

    //next state logic
    //combinational block
    always @(inp[0] or state)begin
       
        case(state)    

            ODD : begin
                
                if(inp[0] == 1) begin
                   next_state = EVEN;                   
                end
                else begin                    
                    next_state = ODD;
                end
            end

            EVEN:  begin
                if(inp[0] == 1) begin
                    next_state =ODD;
                end
                else begin
                    next_state = EVEN;
                end
            end
        endcase 
    end

    //Output logic and input change logic
    always @(posedge clk) begin
        case(state)
            ODD: begin
               if(inp[0]==1) begin
                   out <= 1;
                   inp <= inp >> 1;                   
               end
               else begin
                   out <= 0;
                   inp <=inp >> 1;                  
               end

            end

            EVEN :begin
                if(inp[0]==1) begin
                    out <= 0;
                    inp <= inp >> 1;                  
                end
                else begin
                    out <= 1;
                    inp <= inp >> 1;   
                end

            end
        endcase
    end

     // State Change Logic
    always @(posedge clk) begin
        state <= next_state;
    end

endmodule