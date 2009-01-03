`timescale 1ns/1ps
`define DEBUG
module maindec(input  [5:0] op,
               output       memtoreg, memwrite,
               output       branch, alusrc,
               output       regdst, regwrite,
               output       jump,
               output       zeroextend,
               output [3:0] aluop);

  reg [11:0] controls;

`ifdef DEBUG
  initial begin
    $dumpvars(0, op,
              regwrite, regdst, alusrc, branch, memwrite, memtoreg, jump,
              zeroextend, aluop, controls);
  end
`endif

  assign {regwrite, regdst, alusrc, branch, memwrite, memtoreg, jump, zeroextend,
          aluop} = controls;

  always @(*)
    case(op)
      6'b000000: controls = 12'b11000000_1111; //Rtyp
    //6'b000001: // BLTZ (rt = 0) or BGEZ (rt = 1)
      6'b000010: controls = 12'b00000010_0000; //J // 9'b0XXX0X1_XX
    //6'b000011: // JAL
      6'b000100: controls = 12'b00010000_0001; //BEQ
    //6'b000101: // BNE
    //6'b000110: // BLEZ
    //6'b000111: // BGTZ
      6'b001000: controls = 12'b10100000_0000; // ADDI
      6'b001001: controls = 12'b10100000_0000; // ADDIU
      6'b001010: controls = 12'b10100000_0010; // SLTI
    //6'b001011: controls = 12'b10100000_0011; // SLTIU
      6'b001100: controls = 12'b10100001_0100; // ANDI
      6'b001101: controls = 12'b10100001_0101; // ORI
      6'b001110: controls = 12'b10100001_0110; // XORI
      6'b001111: controls = 12'b10100001_0111; // LUI
    //6'b100000: // LB
    //6'b100001: // LH
      6'b100011: controls = 12'b10100100_0000; //LW
    //6'b100100: // LBU
    //6'b100101: // LHU
    //6'b101000: // SB
    //6'b101001: // SH
      6'b101011: controls = 12'b00101000_0000; //SW
      default: begin
        $display("...Error MainDec: unknown operation, op %x", op);
        controls = 12'bxxxxxxxxx; //??? unknown instruction
        end
    endcase
endmodule
