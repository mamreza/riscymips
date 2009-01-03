`timescale 1ns/1ps
`define DEBUG
module alu(input      [31:0] a, b,     // operands
           input      [5:0]  alucont,  // control code from decoder
           output reg [31:0] result,   // result
           output            overflow  // overflow
           );

  wire [31:0] b2, sum, slt;

`ifdef DEBUG
  initial begin
    $dumpvars(0, a, b, alucont, result, overflow, b2, sum, slt);
  end
`endif
  // change sign for substraction
  assign #1 b2 = alucont[5] ? ~b : b;
  // sum operands (if substraction - add one)
  assign #1 {overflow, sum} = a + b2 + alucont[5];
  // check sign bit (32th) - (comparsion)
  assign #1 slt = sum[31];

  // if any input changes, do:
  always @(*) begin
    case(alucont[4:0])
      5'b00000: result = a & b;  // AND, ANDI
      5'b00001: result = a | b;  // OR, ORI
      5'b00010: result = sum;    // sum: ADD, ADDI, ADDU, ADDUI, SUB, SUBI, SUBU, SUBUI
      5'b00011: result = slt;    // set if less than: SLT, SLTI
      5'b00100: result = a ^ b;  // XOR, XORI
      5'b00101: result = ~(a | b);  // NOR, NORI
      5'b00110: result = {b, 16'b0};  // LUI
      default: begin
        $display("...Error, ALU: unknown function: %b", alucont);
      end
    endcase
  end
endmodule