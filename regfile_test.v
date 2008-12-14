`timescale 1ns/1ps

module regfile_testbench;
  wire clk;
  reg w_enable = 1'b0;
  reg [4:0] r_addr1 = 5'b00000;
  reg [4:0] r_addr2 = 5'b00000;
  reg [4:0] w_addr1 = 5'b00000;
  reg [31:0] w_data1 = 32'b00000000000000000000000000000000;
  wire [31:0] r_data1;
  wire [31:0] r_data2;

  // Connect the clock generator
  clockgenerator clock(clk);

  regfile UUT (
    .clk(clk),
    .w_enable(w_enable),
    .r_addr1(r_addr1),
    .r_addr2(r_addr2),
    .w_addr1(w_addr1),
    .w_data1(w_data1),
    .r_data1(r_data1),
    .r_data2(r_data2)
  );

  initial begin
    $list_net(UUT);
    $dumpvars(0, clk, w_enable, r_addr1, r_addr2,
              w_addr1, w_data1, r_data1, r_data2);
  end

  always @(posedge clk) begin
    $regfile_test(w_enable, r_addr1, r_addr2,
                  w_addr1, w_data1, r_data1, r_data2);
  end

  always @(negedge clk) begin
    #10; $regfile_check();
  end

  initial begin
    // Finish Time [ns]
    //#1500; $finish;
  end

endmodule

