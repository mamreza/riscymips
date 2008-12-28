`timescale 1ns/1ps

`define DEBUG

module alu_testbench;
  wire clk;
  wire [31:0] result;

  reg [31:0] a = 32'd0, b = 32'd0;
  reg [5:0] alucont = 6'd0;

  // Connect the clock generator
  clockgenerator clock(clk);

  alu UUT(a, b, alucont, result);

  initial begin
    $list_net(UUT);
  end

  always @(posedge clk) begin
    $alu_test(a, b, alucont, result);
  end

  always @(negedge clk) begin
    #10; $alu_check();
  end
endmodule

