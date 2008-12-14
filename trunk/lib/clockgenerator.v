`timescale 1ns/1ps
//         _
// 25 MHz | |_|
// 1/T = f   ->  1/[s] = [Hz]
// (1/4*10e-8)*10e+6 = 25 [MHz]
//

module clockgenerator(clk);
  output clk;
  reg clk = 1'b0;

  parameter PERIOD = 40;
  parameter real DUTY_CYCLE = 0.5;
  parameter OFFSET = 0;

  initial begin
    #OFFSET;
    forever begin
      clk = 1'b1;
      #(PERIOD-(PERIOD*DUTY_CYCLE)) clk = 1'b0;
      #(PERIOD*DUTY_CYCLE);
    end
  end
endmodule
