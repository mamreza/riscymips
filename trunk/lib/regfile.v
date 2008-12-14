`timescale 1ns/1ps

// Register file:
// three ported register file
// read two ports combinationally
// write third port on falling edge of clock
// register 0 hardwired to 0

module regfile (clk, w_enable, r_addr1, r_addr2, w_addr1, w_data1, r_data1, r_data2);
    input        clk;           // clock
    input        w_enable;      // write enable
    input [4:0]  r_addr1,
                 r_addr2,
                 w_addr1;       // addres lines
    input [31:0] w_data1;       // data input
    output [31:0] r_data1,
                  r_data2;      // data output(s)

    reg [31:0] rfile[31:0];     // actual register file

`ifdef DEBUG
    initial begin
      $dumpvars(0, clk, w_enable, r_addr1, r_addr2,
                w_addr1, w_data1, r_data1, r_data2);
    end
`endif
    // write on rising clock edge
    always @(negedge clk)
      if(w_enable) rfile[w_addr1] <= w_data1;

    assign r_data1 = (r_addr1 != 0) ? rfile[r_addr1] : 0;
    assign r_data2 = (r_addr2 != 0) ? rfile[r_addr2] : 0;
endmodule