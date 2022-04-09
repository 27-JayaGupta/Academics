`include "A1Q3_three_to_eight_decoder.v"

module stimulus;

    wire [7:0] ans; //output
    reg [2:0] a; //input
    
    // 15 different inputs for validation at 5 units time-delay
    // a[0] is the least significant bit and a[2] is the most significant bit (for input)
    initial begin
        a[0] = 0;
        a[1] = 0;
        a[2] = 0;

        #5
        a[0] = 1;
        a[1] = 0;
        a[2] = 0;

         #5
        a[0] = 0;
        a[1] = 1;
        a[2] = 0;
        
         #5
        a[0] = 1;
        a[1] = 1;
        a[2] = 0;

         #5
        a[0] = 0;
        a[1] = 0;
        a[2] = 1;

         #5
        a[0] = 1;
        a[1] = 0;
        a[2] = 1;

         #5
        a[0] = 0;
        a[1] = 1;
        a[2] = 1;

         #5
        a[0] = 1;
        a[1] = 1;
        a[2] = 1;
    end
    
    //instantiation of three_to_eight_decoder
    three_to_eight_decoder d1(a,ans);

    initial begin    
        //If ans(output) is 00000001 this means that ans[0] =1 and others are 0
        // If ans(output) is 00001000 this means that ans[3] =1 and others are 0
        // If a = 001 this means a[0] =1 , a[1] =0 and a[2] =0
        $monitor($time , " input a: %b  , output: %b",a,ans);
        $dumpfile("3_to_8_decoder.vcd");
        $dumpvars(0,stimulus);
    end   
endmodule