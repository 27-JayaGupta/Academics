## Model used

<br/>
<img src=decoder.png>
<hr/>
<br/>

## Assumptions

1. Three_to_8_decoder is a decoder that takes three inputs and produces eight outputs.
2. We have used two_to_four_decoder as a submodule for this decoder.
3. Three inputs are:a[0],a[1],a[2]
4. a[2] and not(a[2]) are fed as enable signals to two_to_four_decoder (d2 and d1 respectively. Also we have used not gate for signal inversion).
5. a[0] and a[1] are given as input to both two_to_four_decoder.
