module alu(input      [31:0] a, b,
           input      [2:0]  alucont,
           output reg [31:0] result
           );

  wire [31:0] b2, sum, slt;

  initial begin
    $dumpvars(0,a,b,alucont,result);
  end

  assign #1 b2 = alucont[2] ? ~b:b;
  assign #1 sum = a + b2 + alucont[2];
  assign #1 slt = sum[31];

  always@(*)
    case(alucont[1:0])
      2'b00: result = #1 a & b;
      2'b01: result = #1 a | b;
      2'b10: result = #1 sum;
      2'b11: result = #1 slt;
    endcase
endmodule