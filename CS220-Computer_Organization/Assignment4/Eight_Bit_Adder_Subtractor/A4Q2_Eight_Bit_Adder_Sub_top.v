`include "A4Q2_Eight_bit_Adder_Sub.v"

module stimulus;

    reg[7:0] a;
    reg[7:0] b;
    reg opcode;
    wire [7:0] ans;
    wire overflow;
    Eight_Bit_Adder_Sub full_adder_sub(a,b,opcode,ans,overflow);
    
    initial begin
        
        a = 115; b = 30; opcode = 0;

        #5
        a = -115; b = 30; opcode = 1;
        
        #5
        a = 68; b = -93; opcode = 0;
        
        #5
        a = 68; b = -93; opcode = 1;
       
        #5
        a = -57; b = 79; opcode = 0;
        
        #5
        a = -127; b = 127; opcode = 0;
        
        #5
        a = -57; b = 79; opcode = 1;
        
        #5
        a = -88; b = -98; opcode = 0;
    
        #5
        a = -88; b = -98; opcode = 1;

        #5   
        a = -128; b = -128; opcode = 1;

        #5   
        a = 20; b = 50; opcode = 1;

        #5   
        a = -16; b = -18; opcode = 0;

        #5   
        a = 17; b = 18; opcode = 0;

        #5   
        a = -127; b = -1; opcode = 1;

        #5   
        a = 12; b = -144; opcode = 1;

        #5 $finish;
    end

    

    initial begin
        $monitor($time," A:%b ,B: %b, opcode = %b, sum: %b, overflow :%d",a,b,opcode,ans,overflow);
        $dumpfile("8_bit_adder.vcd");
        $dumpvars(0,stimulus);
    end

endmodule