//------------------------------------------------
//
// Testbench for MIPS processor
//------------------------------------------------
`timescale 1ns/1ps
module testbench();

  wire          clk;
  reg         reset;

  wire [31:0] writedata, dataadr;
  wire memwrite;

  // Connect the clock generator
  clockgenerator clock(clk);

  // instantiate device to be tested
  top DUT(clk, reset, writedata, dataadr, memwrite);

  initial begin
    $list_net(DUT);
    $dumpvars(0, clk, reset, writedata, dataadr, memwrite);
    #1200;
    $display("STOP: Simulation failed");
    $stop;
  end

  // initialize test
  initial
    begin
      reset <= 1; #22; reset <= 0;
    end

  // check results
  always@(negedge clk)
    begin
      if(memwrite) begin
        if(dataadr === 84 & writedata === 7) begin
          $display("Simulation succeeded");
          #40; $finish;
        //  $stop;
        end else if (dataadr !== 80) begin
          $display("Simulation failed");
          $stop;
        end
      end
    end
endmodule
