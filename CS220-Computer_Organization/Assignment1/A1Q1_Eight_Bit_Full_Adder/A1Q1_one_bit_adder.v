module one_bit_adder(
        output cout,
        output ans,
        input a ,
        input b,
        input cin 
);
    //sum and cout logic in behavioural form
    assign ans  = a^b^cin;
    assign cout = (a&cin) | (b&cin) |(a&b) ;
    
endmodule