`include "A1Q1_eight_bit_full_adder.v"

module stimulus;

    reg[7:0] a; //input first number
    reg[7:0] b; //input second number
    reg cin;   //input cin (assigned 0)
    wire [7:0] ans;  //output ans 
    wire cout; 
    eight_bit_full_adder full_adder(ans,cout,a,b,cin);  //initialization
    
    //15 different inputs
    initial begin
        cin= 1'b0;
        a = 1;
        b = 2;

        #5
        a = 5;
        b=10;
        
        #5
        a=37;
        b=28;

        #5
        a=58;
        b=22;

        #5
        a=100;
        b=1;

        #5
        a=142;
        b=22;
        
        #5
        a=142;
        b=22;

        #5
        a=163;
        b=62;

        #5
        a=101;
        b=101;

        #5
        a=98;
        b=22;

        #5
        a=150;
        b=2;

        #5
        a=241;
        b=10;

        #5
        a=45;
        b=81;

        #5
        a=11;
        b=22;

        #5
        a=78;
        b=46;

        #5
        a=134;
        b=35;

        #5 $finish;
    end

    

    initial begin
        $monitor($time," A:%d ,B: %d,  sum: %d, cout :%d",a,b,ans,cout);
        $dumpfile("8_bit_adder.vcd");
        $dumpvars(0,stimulus);
    end

endmodule