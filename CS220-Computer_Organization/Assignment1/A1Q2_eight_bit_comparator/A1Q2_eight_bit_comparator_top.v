`include "A1Q2_eight_bit_comparator.v"
module stimulus;

    reg [7:0] a;
    reg [7:0] b;
    reg l_initial;
    reg e_initial;
    reg g_initial;
    
    wire l,e,g;

    initial begin
        l_initial = 0;
        e_initial =1;
        g_initial =0; 
        a = 8'b00000001;
        b = 8'b00000001;

        #5
        a = 8'b00000001;
        b = 8'b00000010;

         #5
        a = 8'b00010000;
        b = 8'b00000011;

         #5
        a = 8'b00011000;
        b = 8'b00000010;

         #5
        a = 8'b10000001;
        b = 8'b01000010;

         #5
        a = 8'b00000100;
        b = 8'b00000100;

         #5
        a = 8'b00000001;
        b = 8'b00000010;

         #5
        a = 8'b00100001;
        b = 8'b00100010;

         #5
        a = 8'b00000001;
        b = 8'b00010010;

         #5
        a = 8'b00100001;
        b = 8'b00010010;

         #5
        a = 8'b01000001;
        b = 8'b00010010;

         #5
        a = 8'b00000001;
        b = 8'b10000010;

         #5
        a = 8'b00100001;
        b = 8'b00100010;

         #5
        a = 8'b00000001;
        b = 8'b01111110;

         #5
        a = 8'b00000000;
        b = 8'b00000000;

    end

    eight_bit_comparator c1(a,b,l_initial,e_initial,g_initial,l,e,g);

    initial begin
        $monitor($time," a : %d ,b : %d , a<b : %d , a=b : %d , a>b : %d",a,b,l,e,g);
        $dumpfile("8_bit_comparator.vcd");
        $dumpvars(0,stimulus);
    end
endmodule