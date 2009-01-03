`timescale 1ns/1ps
`define DEBUG
module aludec(input      [5:0] funct,
              input      [3:0] aluop,
              output reg [5:0] alucontrol);

`ifdef DEBUG
  initial begin
    $dumpvars(0, funct, aluop, alucontrol);
  end
`endif
  always @(*)
    case(aluop)
      4'b0000: alucontrol = 6'b0_00010;  // ADDI
      4'b0001: alucontrol = 6'b1_00010;  // SUBI
      4'b0010: alucontrol = 6'b1_00011;  // SLTI
    //4'b0011: // SLTU
      4'b0100: alucontrol = 6'b0_00000;  // ANDI
      4'b0101: alucontrol = 6'b0_00001;  // ORI
      4'b0110: alucontrol = 6'b0_00100;  // XORI
      4'b0111: alucontrol = 6'b0_00110;  // LUI
      4'b1111: case(funct)         // RTYPE
          //6'b000000: // sll - shift left logical
          //6'b000010: // srl - shift right logical
          //6'b000011: // sra - shift right arithmetic
          //6'b000100: // sllv
          //6'b000110: // srlv
          //6'b000111: // srav
          //6'b001000: // jr
          //6'b001001: // jalr
          //6'b001100: // syscall
          //6'b001101: // break
          6'b100000: alucontrol = 6'b0_00010;  // ADD
          6'b100001: alucontrol = 6'b0_00010;  // ADDU
          6'b100010: alucontrol = 6'b1_00010;  // SUB
          6'b100011: alucontrol = 6'b1_00010;  // SUBU
          6'b100100: alucontrol = 6'b0_00000;  // AND
          6'b100101: alucontrol = 6'b0_00001;  // OR
          6'b100110: alucontrol = 6'b0_00100;  // XOR
          6'b100111: alucontrol = 6'b0_00101;  // NOR
          6'b101010: alucontrol = 6'b1_00011;  // SLT
          //6'b101011: // SLTU
          default: begin
            $display("...Error AluDec: unknown instruction, func: %b", funct);
            alucontrol = 6'bxxxxxx;  // ??? unknown function
          end
        endcase
        default: begin
          $display("...Error AluDec: unknown instruction, aluop: %b", aluop);
        end
    endcase
endmodule