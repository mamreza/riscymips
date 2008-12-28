`timescale 1ns/1ps
module maindec(input  [5:0] op,
               output       memtoreg, memwrite,
               output       branch, alusrc,
               output       regdst, regwrite,
               output       jump,
               output [1:0] aluop);

  reg [9:0] controls;

  assign {regwrite, regdst, alusrc, branch, memwrite, memtoreg, jump,
          aluop} = controls;

  always @(*)
    case(op)
      6'b000000: controls = 9'b1100000_10; //Rtyp
    //6'b000001: // BLTZ (rt = 0) or BGEZ (rt = 1)
      6'b000010: controls = 9'b0000001_00; //J // 9'b0XXX0X1_XX
    //6'b000011: // JAL
      6'b000100: controls = 9'b0001000_01; //BEQ
    //6'b000101: // BNE
    //6'b000110: // BLEZ
    //6'b000111: // BGTZ
      6'b001000: controls = 9'b1010000_00; // ADDI
      6'b001001: controls = 9'b1010000_00; // ADDIU
    //6'b001010: // SLTI
    //6'b001011: // SLTIU
    //6'b001100: // ANDI
    //6'b001101: // ORI
    //6'b001110: // XORI
    //6'b001111: // LUI
    //6'b100000: // LB
    //6'b100001: // LH
      6'b100011: controls = 9'b1010010_00; //LW
    //6'b100100: // LBU
    //6'b100101: // LHU
    //6'b101000: // SB
    //6'b101001: // SH
      6'b101011: controls = 9'b0010100_00; //SW
      default:   controls = 9'bxxxxxxxxx; //??? unknown instruction
    endcase
endmodule
