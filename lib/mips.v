//------------------------------------------------
// RiscyMIPS
// Pipelined MIPS processor
//------------------------------------------------
`timescale 1ns/1ps

module mips(input         clk, reset,
            output [31:0] pcF,
            input  [31:0] instrF,
            output        memwriteM,
            output [31:0] aluoutM, writedataM,
            input  [31:0] readdataM);

  wire [5:0]  opD, functD;
  wire        regdstE, alusrcE,
              pcsrcD,
              memtoregE, memtoregM, memtoregW, regwriteE, regwriteM, regwriteW;
  wire [5:0]  alucontrolE;
  wire        flushE, equalD, branchD, jumpD;

  controller c(clk, reset, opD, functD, flushE, equalD,
               memtoregE, memtoregM, memtoregW, memwriteM, pcsrcD, branchD,
               alusrcE, regdstE, regwriteE, regwriteM, regwriteW, jumpD,
               alucontrolE);
  datapath dp(clk, reset, memtoregE, memtoregM, memtoregW, pcsrcD, branchD,
              alusrcE, regdstE, regwriteE, regwriteM, regwriteW, jumpD,
              alucontrolE,
              equalD, pcF, instrF,
              aluoutM, writedataM, readdataM,
              opD, functD, flushE);
endmodule

module controller(input        clk, reset,
                  input  [5:0] opD, functD,
                  input        flushE, equalD,
                  output       memtoregE, memtoregM, memtoregW, memwriteM,
                  output       pcsrcD, branchD, alusrcE,
                  output       regdstE, regwriteE, regwriteM, regwriteW,
                  output       jumpD,
                  output [5:0] alucontrolE);

  wire [1:0] aluopD;

  wire       memtoregD, memwriteD, alusrcD,
             regdstD, regwriteD;
  wire [5:0] alucontrolD;
  wire       memwriteE;

  // main decoder
  maindec md(opD, memtoregD, memwriteD, branchD,
             alusrcD, regdstD, regwriteD, jumpD,
             aluopD);
  // alu decoder
  aludec  ad(functD, aluopD, alucontrolD);

  // check for branch
  assign pcsrcD = branchD & equalD;

  // pipeline registers - control path
  // register between Decode and Execute stages
  floprc #(11) regE(clk, reset, flushE,
                  {memtoregD, memwriteD, alusrcD, regdstD, regwriteD, alucontrolD},
                  {memtoregE, memwriteE, alusrcE, regdstE, regwriteE,  alucontrolE});
  // register between Execute and Memory stages
  flopr #(3) regM(clk, reset,
                  {memtoregE, memwriteE, regwriteE},
                  {memtoregM, memwriteM, regwriteM});
  // register between Memory and Writeback stages
  flopr #(2) regW(clk, reset,
                  {memtoregM, regwriteM},
                  {memtoregW, regwriteW});
endmodule

module datapath(input         clk, reset,
                input         memtoregE, memtoregM, memtoregW,
                input         pcsrcD, branchD,
                input         alusrcE, regdstE,
                input         regwriteE, regwriteM, regwriteW,
                input         jumpD,
                input  [5:0]  alucontrolE,
                output        equalD,
                output [31:0] pcF,
                input  [31:0] instrF,
                output [31:0] aluoutM, writedataM,
                input  [31:0] readdataM,
                output [5:0]  opD, functD,
                output        flushE);

  wire        forwardaD, forwardbD;
  wire [1:0]  forwardaE, forwardbE;
  wire        stallF,stallD;
  wire [4:0]  rsD, rtD, rdD, rsE, rtE, rdE;
  wire [4:0]  writeregE, writeregM, writeregW;
  wire        flushD;
  wire [31:0] pcnextFD, pcnextbrFD, pcplus4F, pcbranchD;
  wire [31:0] signimmD, signimmE, signimmshD;
  wire [31:0] srcaD, srca2D, srcaE, srca2E;
  wire [31:0] srcbD, srcb2D, srcbE, srcb2E, srcb3E;
  wire [31:0] pcplus4D, instrD;
  wire [31:0] aluoutE, aluoutW;
  wire [31:0] readdataW, resultW;

  // Hazard Detection Unit
  hazard    h(rsD, rtD, rsE, rtE, writeregE, writeregM, writeregW,
              regwriteE, regwriteM, regwriteW,
              memtoregE, memtoregM, branchD,
              forwardaD, forwardbD, forwardaE, forwardbE,
              stallF, stallD, flushE);

  // next PC logic (operates in fetch and decode)
  mux2 #(32)  pcbrmux(pcplus4F, pcbranchD, pcsrcD, pcnextbrFD);
  mux2 #(32)  pcmux(pcnextbrFD,{pcplus4D[31:28], instrD[25:0], 2'b00},
                    jumpD, pcnextFD);

  // register file (operates in decode and writeback)
  regfile     rf(clk, regwriteW, rsD, rtD, writeregW,
                 resultW, srcaD, srcbD);

  //-- Fetch stage logic ---
  // F latch / PC register
  flopenr #(32) pcreg(clk, reset, ~stallF, pcnextFD, pcF);
  // PC adder/incrementer, add 4 to PC
  adder       pcadd1(pcF, 32'b100, pcplus4F);

  //-- Decode stage ---
  // D latch / decode stage register (upper part)
  flopenrc #(32) r2D(clk, reset, ~stallD, flushD, instrF, instrD);
  // D latch / decode stage register (lower part)
  flopenr #(32) r1D(clk, reset, ~stallD, pcplus4F, pcplus4D);
  // sign extend immediate value
  signext     se(instrD[15:0], signimmD);
  // shift left immediate value by 2
  sl2         immsh(signimmD, signimmshD);
  // add to to PC sign extended and shifted immediate value (branch)
  adder       pcadd2(pcplus4D, signimmshD, pcbranchD);
  // forwarding multiplexers
  mux2 #(32)  forwardadmux(srcaD, aluoutM, forwardaD, srca2D);
  mux2 #(32)  forwardbdmux(srcbD, aluoutM, forwardbD, srcb2D);
  // branch prediction comparator
  eqcmp       comp(srca2D, srcb2D, equalD);

  // get operation code (6 bit) from instruction
  assign opD = instrD[31:26];
  // get function code (6 bit) from instruction
  assign functD = instrD[5:0];
  // get source register (5 bit) from instruction
  assign rsD = instrD[25:21];
  // get target register (5 bit) from instruction
  assign rtD = instrD[20:16];
  // get destination register (5 bit) from instruction
  assign rdD = instrD[15:11];

  // flush D latch if branch or jump
  assign flushD = pcsrcD | jumpD;
  //---

  //-- Execute stage ---
  // latch E
  floprc #(32) r1E(clk, reset, flushE, srcaD, srcaE);
  floprc #(32) r2E(clk, reset, flushE, srcbD, srcbE);
  floprc #(32) r3E(clk, reset, flushE, signimmD, signimmE);
  floprc #(5)  r4E(clk, reset, flushE, rsD, rsE);
  floprc #(5)  r5E(clk, reset, flushE, rtD, rtE);
  floprc #(5)  r6E(clk, reset, flushE, rdD, rdE);
  // forwarding multiplexers
  mux3 #(32)  forwardaemux(srcaE, resultW, aluoutM, forwardaE, srca2E);
  mux3 #(32)  forwardbemux(srcbE, resultW, aluoutM, forwardbE, srcb2E);
  // ALU B operand source selector
  mux2 #(32)  srcbmux(srcb2E, signimmE, alusrcE, srcb3E);
  // ALU Unit
  alu         alu(srca2E, srcb3E, alucontrolE, aluoutE);
  // Write register selector (rt or td)
  mux2 #(5)   wrmux(rtE, rdE, regdstE, writeregE);
  // ---

  //-- Memory stage ---
  // latch M
  flopr #(32) r1M(clk, reset, srcb2E, writedataM);
  flopr #(32) r2M(clk, reset, aluoutE, aluoutM);
  flopr #(5)  r3M(clk, reset, writeregE, writeregM);

  //-- Writeback stage ---
  // latch W
  flopr #(32) r1W(clk, reset, aluoutM, aluoutW);
  flopr #(32) r2W(clk, reset, readdataM, readdataW);
  flopr #(5)  r3W(clk, reset, writeregM, writeregW);
  // result selector (from ALU or Memory)
  mux2 #(32)  resmux(aluoutW, readdataW, memtoregW, resultW);

endmodule

// Hazard Unit
module hazard(input  [4:0] rsD, rtD, rsE, rtE,
              input  [4:0] writeregE, writeregM, writeregW,
              input        regwriteE, regwriteM, regwriteW,
              input        memtoregE, memtoregM, branchD,
              output           forwardaD, forwardbD,
              output reg [1:0] forwardaE, forwardbE,
              output       stallF, stallD, flushE);

  wire lwstallD, branchstallD;

  // forwarding sources to D stage (branch equality)
  assign forwardaD = (rsD != 0 & rsD == writeregM & regwriteM);
  assign forwardbD = (rtD != 0 & rtD == writeregM & regwriteM);

  // forwarding sources to E stage (ALU)
  always @(*)
    begin
      forwardaE = 2'b00; forwardbE = 2'b00;
      if (rsE != 0)
        if (rsE == writeregM & regwriteM) forwardaE = 2'b10;
        else if (rsE == writeregW & regwriteW) forwardaE = 2'b01;
      if (rtE != 0)
        if (rtE == writeregM & regwriteM) forwardbE = 2'b10;
        else if (rtE == writeregW & regwriteW) forwardbE = 2'b01;
    end

  // stalls
  assign #1 lwstallD = memtoregE & (rtE == rsD | rtE == rtD);
  assign #1 branchstallD = branchD &
             (regwriteE & (writeregE == rsD | writeregE == rtD) |
              memtoregM & (writeregM == rsD | writeregM == rtD));

  assign #1 stallD = lwstallD | branchstallD;
  assign #1 stallF = stallD; // stalling D stalls all previous stages
  assign #1 flushE = stallD; // stalling D flushes next stage

  // *** not necessary to stall D stage on store if source comes from load;
  // *** instead, another bypass network could be added from W to M
endmodule

// simple adder
module adder(input  [31:0] a, b,
             output [31:0] y);

  assign #1 y = a + b;
endmodule

// simple comparator
module eqcmp(input [31:0] a, b,
             output        eq);

  assign #1 eq = (a == b);
endmodule

// shift left by 2
module sl2(input  [31:0] a,
           output [31:0] y);

  // shift left by 2
  assign #1 y = {a[29:0], 2'b00};
endmodule

// extend sign of immediate value to 32 bits
module signext(input  [15:0] a,
               output [31:0] y);

  assign #1 y = {{16{a[15]}}, a};
endmodule

module flopr #(parameter WIDTH = 8)
              (input                  clk, reset,
               input      [WIDTH-1:0] d,
               output reg [WIDTH-1:0] q);

  always @(posedge clk, posedge reset)
    if (reset) q <= #1 0;
    else       q <= #1 d;
endmodule

// latch with clear
module floprc #(parameter WIDTH = 8)
              (input                  clk, reset, clear,
               input      [WIDTH-1:0] d,
               output reg [WIDTH-1:0] q);

  always @(posedge clk, posedge reset)
    if (reset)      q <= #1 0;
    else if (clear) q <= #1 0;
    else            q <= #1 d;
endmodule

// latch with enable
module flopenr #(parameter WIDTH = 8)
                (input                  clk, reset,
                 input                  en,
                 input      [WIDTH-1:0] d,
                 output reg [WIDTH-1:0] q);

  always @(posedge clk, posedge reset)
    if      (reset) q <= #1 0;
    else if (en)    q <= #1 d;
endmodule

// latch with enable and clear
module flopenrc #(parameter WIDTH = 8)
                 (input                  clk, reset,
                  input                  en, clear,
                  input      [WIDTH-1:0] d,
                  output reg [WIDTH-1:0] q);

  always @(posedge clk, posedge reset)
    if      (reset) q <= #1 0;
    else if (clear) q <= #1 0;
    else if (en)    q <= #1 d;
endmodule

// 2 input multiplexer
module mux2 #(parameter WIDTH = 8)
             (input  [WIDTH-1:0] d0, d1,
              input              s,
              output [WIDTH-1:0] y);

  assign #1 y = s ? d1 : d0;
endmodule

// 3 input multiplexer
module mux3 #(parameter WIDTH = 8)
             (input  [WIDTH-1:0] d0, d1, d2,
              input  [1:0]       s,
              output [WIDTH-1:0] y);

  assign #1 y = s[1] ? d2 : (s[0] ? d1 : d0);
endmodule

// 4 input multiplexer
module mux4 #(parameter WIDTH = 8)
             (input  [WIDTH-1:0] d0, d1, d2, d3,
              input  [2:0]       s,
              output [WIDTH-1:0] y);

  assign #1 y = s[2] ? d3 : (s[1] ? d2 : (s[0] ? d1 : d0));
endmodule
