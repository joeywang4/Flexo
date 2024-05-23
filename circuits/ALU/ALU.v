module ALU (
    input [3:0] x, // 4-bit inputs
    input [3:0] y,
    input [5:0] control,
    output [5:0] out,  // 4-bit output + zr + ng
);
  wire zx;  // zero the x input?
  wire nx;  // negate the x input?
  wire zy;  // zero the y input?
  wire ny;  // negate the y input?
  wire f;   // compute  out = x + y (if 1) or out = x & y (if 0)
  wire no;  // negate the out output?
  wire [3:0] zxOut;
  wire [3:0] notX;
  wire [3:0] nxOut;
  wire [3:0] zyOut;
  wire [3:0] notY;
  wire [3:0] nyOut;
  wire [3:0] andOut;
  wire [3:0] addOut;
  wire [3:0] fOut;
  wire [3:0] noOut;
  wire [3:0] result;
  wire zrOut;

  assign zx = control[0];
  assign nx = control[1];
  assign zy = control[2];
  assign ny = control[3];
  assign f  = control[4];
  assign no = control[5];

  // ----- Zero x (zx bit)
  // If the zx bit is set, the muxer will select 16 bits of 0,
  // otherwise it will select x
  assign zxOut = zx ? 4'b0 : x;

  // ----- Negate x (nx bit)
  // Negate x with bitwise negation. Then use Mux to select between negated x
  // or the previous output (zxOut).
  assign notX = ~zxOut;
  assign nxOut = nx ? notX : zxOut;

  // ----- Zero y (zy bit)
  // Exactly the same as zero x.
  assign zyOut = zy ? 4'b0 : y;

  // ----- Negate y (ny bit)
  // Behaves exactly the same as Negate x.
  assign notY = ~zyOut;
  assign nyOut = ny ? notY : zyOut;

  // ----- Compute out (f bit)
  // x & y
  assign andOut = nxOut & nyOut;

  // x + y
  assign addOut = nxOut + nyOut;

  // Return the output that the f bit expects
  assign fOut = f ? addOut : andOut;

  // ----- Negate output (no bit)
  // Simply negate the output if this bit is set. This operation returns
  // the result output.
  assign result = no ? ~fOut : fOut;

  // Need out result mux so that we can save the result output and run a few
  // tests on it below.
  assign out[3:0] = result;

  // ----- Return true if the output is zero (zr)
  // Or over all 16 bits. This will return false if the number is zero,
  // so flip the bit so that zr is true
  assign zrOut = |result;
  assign out[4] = ~zrOut;

  // ----- Return true if the output is negative (ng)
  // The most significant bit will be 1 if the number is negative thanks to
  // 2's complement. But I abstracted all of that out into another chip.
  assign out[5] = result[3];
endmodule
