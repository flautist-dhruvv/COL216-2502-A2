.data : 5 10
addi x1 x0 1
addi x2 x0 2
addi x3 x0 0

# Predictor starts at State 0 (Predict Taken). 
# It will guess "Taken", but 1 != 2, so it actually shouldn't jump!
beq x1 x2 bad_path

# --- CORRECT PATH ---
lw x4 0(x0)      
sw x4 1(x0)      
j end

# --- WRONG PATH (Executes Speculatively) ---
bad_path:
sw x1 0(x0)      # Speculative Store: Writes 1 to addr 0
lw x5 0(x0)      # Speculative Forward: Gets 1 from the LSQ
div x6 x5 x3     # Speculative Exception: Divide by 0!

end:
lw x7 1(x0)