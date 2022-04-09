module detector(input clk,input [7:0]seq,output [7:0]out);

    parameter SIZE = 2;
    parameter [1:0]ROOT = 2'b00, ONE = 2'b01, TWO = 2'b10, THREE = 2'b11;
    reg [7:0] out;

    reg [1:0] state,next_state;
    integer j ;

    initial begin
        j=7;
    end

    // state initialization

    always@(seq) begin
        if(seq[j]==0) begin
            state = ROOT;
            next_state =ROOT;
        end
        else begin
            state = ONE;
            next_state = ONE;
        end  
    end

    //state update
    always @ (posedge clk ) begin
       
        state <= next_state;
   
    end

    // Output state logic
    always @(posedge clk) begin
        case(state)
            ROOT:begin
                out[j] = 0;
                j = j-1;
            end

            ONE:begin
                out[j] = 0;
                j = j-1;
            end

            TWO:begin
                out[j] = 0;
                j = j-1;
            end

            THREE:begin
                if(seq[j] == 1'b1) begin
                    out[j] = 0;
                    j = j-1;
                end else begin
                    out[j] = 1;
                    j = j-1;
                end
            end

            default : next_state = ROOT;
        endcase

         if(j <0) begin
           j = 7;
           $display("input: %b , output: %b",seq,out);
        end
    end

    // Next State Logic(Combinational Logic)
    always @ (state or seq[j]) begin
    case(state)
            ROOT:begin
                if(seq[j] == 1'b1) begin
                    next_state = ONE;
                end 
                else begin
                    next_state = ROOT;
                end
            end

            ONE:begin
                if(seq[j] == 1'b1) begin
                    next_state = ONE;
                end else begin
                    next_state = TWO;
                end
            end

            TWO:
            begin
                if(seq[j] == 1'b1) begin
                next_state = THREE;
            end else begin
                next_state = ROOT;
            end
            end

            THREE:begin
                if(seq[j] == 1'b1) begin
                next_state = ONE;
            end else begin
                next_state = TWO;
            end
            end

            default : next_state = ROOT;
        endcase
    end
endmodule